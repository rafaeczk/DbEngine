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
	if (argc < 6 || _stricmp(argv[4], "from") != 0)
	{
		fprintf(stderr, "Blad: Niepoprawne argumenty!\n");
		print_help(argv[2]);
		return 1;
	}

	const char* db_filename = argv[1];
	const char* requested_fields_str = argv[3];
	const char* table_name = argv[5];


	// ============= open file and read header =============

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

	// ============= store requested field offsets =============

	uint32_t column_offsets[MAX_COLUMNS];
	uint32_t item_payload_size = 0;
	for (uint16_t i = 0; i < target_table->column_count; i++)
	{
		column_offsets[i] = item_payload_size;
		item_payload_size += target_table->columns[i].size;
	}

	Column** requested_fields = (Column**)malloc(sizeof(Column*) * MAX_COLUMNS);
	uint32_t* requested_field_offsets = (uint32_t*)malloc(sizeof(uint32_t) * MAX_COLUMNS);
	if (!requested_fields || !requested_field_offsets)
	{
		fprintf(stderr, "Blad alokacji pamieci dla requested_fields!\n");
		free(requested_fields);
		free(requested_field_offsets);
		fclose(db_file);
		return 1;
	}

	// ============= prarse requested fields =============

	int requested_fields_count = 0;

	char requested_fields_copy[512];
	strncpy_s(requested_fields_copy, sizeof(requested_fields_copy), requested_fields_str, _TRUNCATE);

	char* requested_field_context = NULL;
	char requested_field_separator[] = ",";
	char* requested_field_token = strtok_s(requested_fields_copy, requested_field_separator, &requested_field_context);

	while (requested_field_token != NULL)
	{
		if (requested_fields_count >= MAX_COLUMNS)
		{
			fprintf(stderr, "Blad: Przekroczono maksymalna liczbe kolumn w zapytaniu!\n");
			free(requested_fields);
			free(requested_field_offsets);
			fclose(db_file);
			return 1;
		}

		Column* col = NULL;
		uint32_t offset = 0;
		for (int i = 0; i < target_table->column_count; i++)
		{
			if (_stricmp(requested_field_token, target_table->columns[i].name) == 0)
			{
				col = &target_table->columns[i];
				offset = column_offsets[i];
				break;
			}
		}

		if (col == NULL)
		{
			fprintf(stderr, "Blad: Nie znaleziono pola '%s' w tabeli %s!\n", requested_field_token, table_name);
			free(requested_fields);
			free(requested_field_offsets);
			fclose(db_file);
			return 1;
		}

		requested_fields[requested_fields_count] = col;
		requested_field_offsets[requested_fields_count] = offset;
		requested_fields_count++;
		requested_field_token = strtok_s(NULL, requested_field_separator, &requested_field_context);
	}

	// ============= print header =============

	printf("\n--- Tabela: %s (Rekordow: %u) ---\n", target_table->name, target_table->record_count);

	printf("     ");
	for (int j = 0; j < requested_fields_count; j++)
	{
		printf("%-15s", requested_fields[j]->name);
	}
	printf("\n");

	printf("-----");
	for (int j = 0; j < requested_fields_count; j++)
	{
		printf("---------------");
	}
	printf("\n");

	// ============= print items =============

	uint32_t current_offset = target_table->first_record_offset;
	int index = 1;

	char* item_buffer = (char*)calloc(1, item_payload_size);
	if (!item_buffer)
	{
		fprintf(stderr, "Blad alokacji pamieci bufora!\n");
		fclose(db_file);
		return 1;
	}

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
			printf("%-5d", index);

			for (int j = 0; j < requested_fields_count; j++)
			{
				Column* col = requested_fields[j];
				char* field_ptr = item_buffer + requested_field_offsets[j];

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
					printf("%-15.*s", (int)col->size, field_ptr);
				}
			}
			printf("\n");
			index++;
		}

		current_offset = rec_header.next_record_offset;
	}

	printf("\n");

	free(requested_fields);
	free(requested_field_offsets);
	free(item_buffer);
	fclose(db_file);
	return 0;
}
