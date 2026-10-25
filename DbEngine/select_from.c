#include "select_from.h"

#include "config.h"
#include "structures.h"
#include "print_help.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

int select_from(int argc, char* argv[])
{
	if (argc != 4)
	{
		fprintf(stderr, "Blad: Niepoprawna liczba argumentow!\n");
		print_help(argv[2]);
		return 1;
	}

	const char* db_filename = argv[1];
	const char* table_name = argv[3];

	FILE* db_file = NULL;
	fopen_s(&db_file, db_filename, "rb+");
	if (!db_file)
	{
		fprintf(stderr, "Blad: Nie mozna otworzyc pliku bazy '%s'!\n", db_filename);
		return 1;
	}

	DBHeader header;
	if (fread(&header, sizeof(DBHeader), 1, db_file) != 1 || header.magic != DB_MAGIC)
	{
		fprintf(stderr, "Blad: Niepoprawny plik bazy danych!\n");
		fclose(db_file);
		return 1;
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
		fclose(db_file);
		return 1;
	}

	TableEntry* target_table = &header.tables[table_index];

	uint32_t item_payload_size = 0;
	for (uint16_t i = 0; i < target_table->column_count; i++)
	{
		item_payload_size += target_table->columns[i].size;
	}

	char* item_buffer = (char*)calloc(1, item_payload_size);
	if (!item_buffer)
	{
		fprintf(stderr, "Blad alokacji pamieci bufora!\n");
		fclose(db_file);
		return 1;
	}

	printf("\n--- Tabela: %s (Rekordow: %u) ---\n", target_table->name, target_table->record_count);

	printf("     ");
	for (uint16_t i = 0; i < target_table->column_count; i++)
	{
		printf("%-15s", target_table->columns[i].name);
	}
	printf("\n");

	for (uint16_t i = 0; i < target_table->column_count; i++)
	{
		printf("---------------");
	}
	printf("\n");

	uint32_t current_offset = target_table->first_record_offset;
	int index = 1;

	while (current_offset != 0)
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

		if (!rec_header.is_deleted)
		{
			uint32_t byte_offset = 0;

			printf("%-5d", index);

			for (uint16_t i = 0; i < target_table->column_count; i++)
			{
				Column* col = &target_table->columns[i];
				char* field_ptr = item_buffer + byte_offset;

				if (col->type == TYPE_INT)
				{
					int32_t val = *(int32_t*)field_ptr;
					printf("%-15d", val);
				}
				else if (col->type == TYPE_FLOAT)
				{
					float val = *(float*)field_ptr;
					printf("%-15.2f", val);
				}
				else if (col->type == TYPE_STRING)
				{
					printf("%-15s", field_ptr);
				}

				byte_offset += col->size;
			}
			printf("\n");
			index++;
		}

		current_offset = rec_header.next_record_offset;
	}

	printf("\n");

	free(item_buffer);
	fclose(db_file);
	return 0;
}
