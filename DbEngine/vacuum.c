#include "vacuum.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "structures.h"
#include "config.h"
#include "print_help.h"

int vacuum(int argc, char* argv[])
{
	int status = 1;
	FILE* old_file = NULL;
	FILE* new_file = NULL;
	char temp_filename[512] = { 0 };
	void* record_payload_buffer = NULL;
	int old_file_closed = 0;
	int new_file_closed = 0;

	if (argc != 3)
	{
		fprintf(stderr, "Blad: Niepoprawna liczba argumentow!\n");
		print_help(argv[2]);
		goto cleanup;
	}

	const char* db_filename = argv[1];

	fopen_s(&old_file, db_filename, "rb");
	if (!old_file)
	{
		fprintf(stderr, "Blad: Nie mozna otworzyc pliku bazy '%s'!\n", db_filename);
		goto cleanup;
	}

	DBHeader old_header;
	if (fread(&old_header, sizeof(DBHeader), 1, old_file) != 1 || old_header.magic != DB_MAGIC)
	{
		fprintf(stderr, "Blad: Niepoprawny plik bazy danych!\n");
		goto cleanup;
	}

	sprintf_s(temp_filename, sizeof(temp_filename), "%s.tmp", db_filename);

	fopen_s(&new_file, temp_filename, "wb+");
	if (!new_file)
	{
		fprintf(stderr, "Blad: Nie mozna utworzyc pliku tymczasowego '%s'!\n", temp_filename);
		goto cleanup;
	}

	DBHeader new_header = old_header;

	if (fwrite(&new_header, sizeof(DBHeader), 1, new_file) != 1)
	{
		fprintf(stderr, "Blad: Nie udalo sie zapisac naglowka do nowego pliku!\n");
		goto cleanup;
	}

	uint32_t total_removed_records = 0;
	uint32_t total_retained_records = 0;

	for (uint32_t t = 0; t < old_header.table_count; t++)
	{
		TableEntry* old_table = &old_header.tables[t];
		TableEntry* new_table = &new_header.tables[t];

		uint32_t first_offset = 0;
		uint32_t prev_rec_offset = 0;
		uint32_t active_count = 0;

		uint32_t current_offset = old_table->first_record_offset;

		while (current_offset != 0)
		{
			if (fseek(old_file, current_offset, SEEK_SET) != 0)
			{
				break;
			}

			RecordHeader rec_header;
			if (fread(&rec_header, sizeof(RecordHeader), 1, old_file) != 1)
			{
				break;
			}

			uint32_t next_offset = rec_header.next_record_offset;

			if (rec_header.is_deleted)
			{
				total_removed_records++;
			}
			else
			{
				record_payload_buffer = realloc(record_payload_buffer, rec_header.payload_size);
				if (rec_header.payload_size > 0 && !record_payload_buffer)
				{
					fprintf(stderr, "Blad alokacji pamieci dla rekordu!\n");
					goto cleanup;
				}

				if (rec_header.payload_size > 0)
				{
					if (fread(record_payload_buffer, rec_header.payload_size, 1, old_file) != 1)
					{
						fprintf(stderr, "Blad odczytu danych rekordu z pliku '%s'!\n", db_filename);
						goto cleanup;
					}
				}

				fseek(new_file, 0, SEEK_END);
				uint32_t new_rec_offset = (uint32_t)ftell(new_file);

				RecordHeader new_rec_header;
				new_rec_header.is_deleted = 0;
				new_rec_header.next_record_offset = 0;
				new_rec_header.payload_size = rec_header.payload_size;

				if (fwrite(&new_rec_header, sizeof(RecordHeader), 1, new_file) != 1)
				{
					fprintf(stderr, "Blad zapisu naglowka rekordu do nowego pliku!\n");
					goto cleanup;
				}

				if (rec_header.payload_size > 0)
				{
					if (fwrite(record_payload_buffer, rec_header.payload_size, 1, new_file) != 1)
					{
						fprintf(stderr, "Blad zapisu danych rekordu do nowego pliku!\n");
						goto cleanup;
					}
				}

				if (active_count == 0)
				{
					first_offset = new_rec_offset;
				}
				else
				{
					long prev_next_ptr_pos = (long)prev_rec_offset + (long)offsetof(RecordHeader, next_record_offset);
					fseek(new_file, prev_next_ptr_pos, SEEK_SET);
					fwrite(&new_rec_offset, sizeof(uint32_t), 1, new_file);
				}

				prev_rec_offset = new_rec_offset;
				active_count++;
				total_retained_records++;
			}

			current_offset = next_offset;
		}

		new_table->first_record_offset = first_offset;
		new_table->last_record_offset = prev_rec_offset;
		new_table->record_count = active_count;
	}

	fseek(new_file, 0, SEEK_SET);
	if (fwrite(&new_header, sizeof(DBHeader), 1, new_file) != 1)
	{
		fprintf(stderr, "Blad aktualizacji naglowka bazy w nowym pliku!\n");
		goto cleanup;
	}

	fflush(new_file);

	long old_size = 0;
	long new_size = 0;

	fseek(old_file, 0, SEEK_END);
	old_size = ftell(old_file);

	fseek(new_file, 0, SEEK_END);
	new_size = ftell(new_file);

	fclose(old_file);
	old_file = NULL;
	old_file_closed = 1;

	fclose(new_file);
	new_file = NULL;
	new_file_closed = 1;

	if (remove(db_filename) != 0)
	{
		fprintf(stderr, "Blad: Nie udalo sie usunac starego pliku bazy '%s'!\n", db_filename);
		goto cleanup;
	}

	if (rename(temp_filename, db_filename) != 0)
	{
		fprintf(stderr, "Blad: Nie udalo sie zmienic nazwy pliku '%s' na '%s'!\n", temp_filename, db_filename);
		goto cleanup;
	}

	printf("\nOperacja VACUUM zakonczona sukcesem dla bazy '%s'.\n", db_filename);
	printf(" - Usuniete / pominiete rekordy: %u\n", total_removed_records);
	printf(" - Zachowane aktywne rekordy  : %u\n", total_retained_records);
	printf(" - Rozmiar przed: %ld B -> po: %ld B (odzyskane: %ld B)\n\n", old_size, new_size, (old_size > new_size ? old_size - new_size : 0));

	status = 0;

cleanup:
	if (record_payload_buffer)
	{
		free(record_payload_buffer);
	}
	if (old_file)
	{
		fclose(old_file);
	}
	if (new_file)
	{
		fclose(new_file);
	}
	if (status != 0 && temp_filename[0] != '\0')
	{
		remove(temp_filename);
	}

	return status;
}
