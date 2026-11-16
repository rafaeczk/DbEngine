#include "update.h"
#include "config.h"
#include "structures.h"
#include "print_help.h"
#include "parse_conditions.h"
#include "check_conditions.h"
#include "str_to_function_type.h"
#include "function_type_to_str.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <math.h>

int update(int argc, char* argv[])
{
	int status = 1;

	FILE* db_file = NULL;
	SelectField** requested_fields = NULL;
	uint32_t* requested_field_offsets = NULL;
	Condition* conditions = NULL;
	char* item_buffer = NULL;

	if (argc != 7 || _stricmp(argv[4], "in") != 0)
	{
		fprintf(stderr, "Blad: Niepoprawne argumenty!\n");
		print_help(argv[2]);
		goto cleanup;
	}

	const char* db_filename = argv[1];
	const char* raw_values = argv[3];
	const char* table_name = argv[5];
	const char* conditions_raw = argv[6];

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

	// ============= allocate item buffer once =============
	item_buffer = (char*)calloc(1, item_payload_size);
	if (!item_buffer)
	{
		fprintf(stderr, "Blad: Nie udalo sie zaalokowac pamieci bufora!\n");
		goto cleanup;
	}

	// ============= parse conditions =============

	int conditions_count = 0;

	if (parse_conditions(conditions_raw, &conditions, &conditions_count, target_table, column_offsets) != 0)
	{
		goto cleanup;
	}

	// ============= apply changes =============

	uint32_t current_offset = target_table->first_record_offset;
	int records_modified = 0;

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

		if (!rec_header.is_deleted && check_conditions(conditions_count, conditions, item_buffer))
		{
			char values_copy[512];
			strncpy_s(values_copy, sizeof(values_copy), raw_values, _TRUNCATE);

			char* val_context = NULL;
			char val_separator[] = ",";
			char* val_token = strtok_s(values_copy, val_separator, &val_context);

			uint32_t byte_offset = 0;
			uint16_t col_idx = 0;

			while (val_token != NULL)
			{
				if (col_idx >= target_table->column_count)
				{
					fprintf(stderr, "Blad: Za duzo kolumn (max %u)!\n", target_table->column_count);
					goto cleanup;
				}

				Column* col = &target_table->columns[col_idx];
				char* field_ptr = item_buffer + byte_offset;
				const char* col_type_str = data_type_to_string(col->type);

				if (_stricmp(val_token, "_") != 0)
				{
					if (col->type == TYPE_INT_AUTO_INC)
					{
						printf("Blad: Proba recznej zmiany identyfikatora.");
						goto cleanup;
					}
					else if (col->type == TYPE_INT)
					{
						int32_t val;
						if (!is_valid_int(val_token, &val))
						{
							fprintf(stderr, "Blad: Niepoprawny typ! Podano '%s' do kolumny typu %s.\n", val_token, col_type_str);
							goto cleanup;
						}
						memcpy(field_ptr, &val, sizeof(int32_t));
					}
					else if (col->type == TYPE_FLOAT)
					{
						float val;
						if (!is_valid_float(val_token, &val))
						{
							fprintf(stderr, "Blad: Niepoprawny typ! Podano '%s' do kolumny typu %s.\n", val_token, col_type_str);
							goto cleanup;
						}
						memcpy(field_ptr, &val, sizeof(float));
					}
					else if (col->type == TYPE_STRING)
					{
						strncpy_s(field_ptr, col->size, val_token, _TRUNCATE);
					}
				}

				col_idx++;
				byte_offset += col->size;
				val_token = strtok_s(NULL, val_separator, &val_context);
			}

			if (col_idx < target_table->column_count)
			{
				fprintf(stderr, "Blad: Za malo wartosci (%u z %u kolumn)!\n", col_idx, target_table->column_count);
				goto cleanup;
			}

			fseek(db_file, current_offset, SEEK_SET);
			fwrite(&rec_header, sizeof(RecordHeader), 1, db_file);
			fwrite(item_buffer, item_payload_size, 1, db_file);
			records_modified++;
		}

		current_offset = rec_header.next_record_offset;
	}

	printf("\nSukces! Zmodyfikowano %d rekordow.", records_modified);
	status = 0;

cleanup:
	free(conditions);
	free(requested_fields);
	free(requested_field_offsets);
	free(item_buffer);
	if (db_file)
	{
		fclose(db_file);
	}
	return status;
}