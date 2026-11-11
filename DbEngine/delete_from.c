#include "delete_from.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stddef.h>

#include "structures.h"
#include "config.h"
#include "print_help.h"
#include "parse_conditions.h"
#include "check_conditions.h"

int delete_from(int argc, char* argv[])
{
	int status = 1;
	FILE* db_file = NULL;
	uint32_t* offsets_to_delete = NULL;
	Condition* conditions = NULL;
	char* item_buffer = NULL;

	if (argc != 6)
	{
		fprintf(stderr, "Blad: Niepoprawna liczba argumentow!\n");
		print_help(argv[2]);
		goto cleanup;
	}

	const char* db_filename = argv[1];
	const char* table_name = argv[3];
	const char* record_to_delete_count_str = argv[4];
	const char* conditions_str = argv[5];

	
	// ============= open file and read header =============

	fopen_s(&db_file, db_filename, "rb+");
	if (!db_file)
	{
		fprintf(stderr, "Blad: Nie mozna otworzyc pliku bazy '%s'!\n", db_filename);
		goto cleanup;
	}

	DBHeader header;
	if (fread(&header, sizeof(DBHeader), 1, db_file) != 1 || header.magic != DB_MAGIC)
	{
		fprintf(stderr, "Blad: Niepoprawny plik bazy danych!\n");
		goto cleanup;
	}

	int table_index = -1;
	for (uint32_t i = 0; i < header.table_count; i++)
	{
		if (_stricmp(header.tables[i].name, table_name) == 0)
		{
			table_index = (int)i;
			break;
		}
	}

	if (table_index == -1)
	{
		fprintf(stderr, "Blad: Tabela '%s' nie istnieje w bazie!\n", table_name);
		goto cleanup;
	}

	TableEntry* target_table = &header.tables[table_index];

	
	// ============= store field offsets =============

	uint32_t column_offsets[MAX_COLUMNS];
	uint32_t item_payload_size = 0;
	for (uint16_t i = 0; i < target_table->column_count; i++)
	{
		column_offsets[i] = item_payload_size;
		item_payload_size += target_table->columns[i].size;
	}


	// ============= parse conditions =============


	int conditions_count = 0;

	if (parse_conditions(conditions_str, &conditions, &conditions_count, target_table, column_offsets) != 0)
	{
		goto cleanup;
	}


	// ============= seek rows meeting conditions =============

	int record_to_delete_count = atoi(record_to_delete_count_str);

	if (record_to_delete_count <= 0)
	{
		fprintf(stderr, "Blad: Liczba rekordow do usuniecia musi byc wieksza od zera!\n");
		goto cleanup;
	}

	offsets_to_delete = (uint32_t*)calloc(record_to_delete_count, sizeof(uint32_t));
	if (!offsets_to_delete)
	{
		fprintf(stderr, "Blad: Nie udalo sie zaalokowac pamieci!\n");
		goto cleanup;
	}

	int deleted_records_count = 0;

	uint32_t current_offset = target_table->first_record_offset;

	item_buffer = (char*)calloc(1, item_payload_size);
	if (!item_buffer)
	{
		fprintf(stderr, "Blad: Nie udalo sie zaalokowac pamieci bufora!\n");
		goto cleanup;
	}

	while (current_offset != 0 && deleted_records_count < record_to_delete_count)
	{
		fseek(db_file, current_offset, SEEK_SET);

		RecordHeader rec_header;
		if (fread(&rec_header, sizeof(RecordHeader), 1, db_file) != 1)
		{
			break;
		}

		if (fread(item_buffer, item_payload_size, 1, db_file) != 1)
		{
			break;
		}

		if (!rec_header.is_deleted && check_conditions(conditions_count, &conditions, item_buffer))
		{
			offsets_to_delete[deleted_records_count] = current_offset;

			deleted_records_count++;
		}

		current_offset = rec_header.next_record_offset;
	}


	// ============= soft delete records =============

	for (int i = 0; i < deleted_records_count; i++)
	{
		uint8_t tombstone = 1;
		fseek(db_file, offsets_to_delete[i] + offsetof(RecordHeader, is_deleted), SEEK_SET);
		fwrite(&tombstone, sizeof(uint8_t), 1, db_file);
	}

	target_table->record_count -= deleted_records_count;
	fseek(db_file, 0, SEEK_SET);
	if (fwrite(&header, sizeof(DBHeader), 1, db_file) != 1)
	{
		fprintf(stderr, "Blad: Zapis naglowka bazy danych nie powiodl sie!\n");
		goto cleanup;
	}

	fflush(db_file);
	printf("\nPomyslnie usunieto %d z %d rekord(ow) z tabeli '%s'.\n\n", deleted_records_count, record_to_delete_count, table_name);
	status = 0;

cleanup:
	free(offsets_to_delete);
	free(conditions);
	free(item_buffer);
	if (db_file)
	{
		fclose(db_file);
	}
	return status;
}
