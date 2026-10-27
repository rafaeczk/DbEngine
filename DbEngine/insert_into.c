#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stddef.h>

#include "insert_into.h"
#include "structures.h"
#include "print_help.h"
#include "type_validator.h"
#include "data_type_to_string.h"

int insert_into(int argc, char* argv[])
{
	if (argc < 5)
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

	uint32_t total_payload_size = 0;
	for (uint16_t i = 0; i < target_table->column_count; i++)
	{
		total_payload_size += target_table->columns[i].size;
	}

	int records_count_to_insert = argc - 4;

	char** payload_buffers = (char**)malloc(sizeof(char*) * records_count_to_insert);
	if (!payload_buffers)
	{
		fprintf(stderr, "Blad alokacji pamieci!\n");
		fclose(db_file);
		return 1;
	}

	for (int i = 0; i < records_count_to_insert; i++)
	{
		payload_buffers[i] = (char*)calloc(1, total_payload_size);
		const char* raw_values = argv[i + 4];

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
				fprintf(stderr, "Blad: Wiersz [%d] ma za duzo kolumn (max %u)!\n", i + 1, target_table->column_count);
				for (int j = 0; j <= i; j++) free(payload_buffers[j]);
				free(payload_buffers);
				fclose(db_file);
				return 1;
			}

			Column* col = &target_table->columns[col_idx];
			char* field_ptr = payload_buffers[i] + byte_offset;

			char* col_type_str = data_type_to_string(col->type);

			if (col->type == TYPE_INT)
			{
				int32_t val;
				if (!is_valid_int(val_token, &val))
				{
					fprintf(stderr, "Blad: Niepoprawny typ! Podano \"%s\" do kolumny typu %s.\n", val_token, col_type_str);
					return 1;
				}
				memcpy(field_ptr, &val, sizeof(int32_t));
			}
			else if (col->type == TYPE_FLOAT)
			{
				float val;
				if (!is_valid_float(val_token, &val))
				{
					fprintf(stderr, "Blad: Niepoprawny typ! Podano \"%s\" do kolumny typu %s.\n", val_token, col_type_str);
					return 1;
				}
				memcpy(field_ptr, &val, sizeof(float));
			}
			else if (col->type == TYPE_STRING)
			{
				strncpy_s(field_ptr, col->size, val_token, _TRUNCATE);
			}

			byte_offset += col->size;
			col_idx++;
			val_token = strtok_s(NULL, val_separator, &val_context);
		}

		if (col_idx < target_table->column_count)
		{
			fprintf(stderr, "Blad: Wiersz [%d] ma za malo wartosci (%u z %u kolumn)!\n", i + 1, col_idx, target_table->column_count);
			for (int j = 0; j <= i; j++) free(payload_buffers[j]);
			free(payload_buffers);
			fclose(db_file);
			return 1;
		}
	}

	for (int i = 0; i < records_count_to_insert; i++)
	{
		fseek(db_file, 0, SEEK_END);
		uint32_t new_record_offset = (uint32_t)ftell(db_file);

		RecordHeader rec_header = {
			.is_deleted = 0,
			.next_record_offset = 0,
			.payload_size = total_payload_size
		};

		fwrite(&rec_header, sizeof(RecordHeader), 1, db_file);
		fwrite(payload_buffers[i], total_payload_size, 1, db_file);

		if (target_table->record_count == 0)
		{
			target_table->first_record_offset = new_record_offset;
		}
		else
		{
			long next_ptr_pos = (long)target_table->last_record_offset + offsetof(RecordHeader, next_record_offset);
			fseek(db_file, next_ptr_pos, SEEK_SET);
			fwrite(&new_record_offset, sizeof(uint32_t), 1, db_file);
		}

		target_table->last_record_offset = new_record_offset;
		target_table->record_count++;

		free(payload_buffers[i]);
	}

	free(payload_buffers);

	fseek(db_file, 0, SEEK_SET);
	fwrite(&header, sizeof(DBHeader), 1, db_file);
	fflush(db_file);
	fclose(db_file);

	printf("Pomyslnie dodano %d rekord(ow) do tabeli '%s'.\n", records_count_to_insert, table_name);
	return 0;
}
