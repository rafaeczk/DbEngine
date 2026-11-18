#include "select_from.h"

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

int compare_records(void* context, const void* a, const void* b);

int select_from(int argc, char* argv[])
{
	int status = 1;

	FILE* db_file = NULL;
	SelectField** requested_fields = NULL;
	uint32_t* requested_field_offsets = NULL;
	Condition* conditions = NULL;
	char* item_buffer = NULL;
	char* order_by = NULL;

	if (argc < 6 || _stricmp(argv[4], "from") != 0)
	{
		fprintf(stderr, "Blad: Niepoprawne argumenty!\n");
		print_help(argv[2]);
		goto cleanup;
	}

	const char* db_filename = argv[1];
	const char* requested_fields_str = argv[3];
	const char* table_name = argv[5];

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

	// ============= parse requested fields =============

	requested_fields = (SelectField**)malloc(sizeof(SelectField*) * MAX_COLUMNS);
	requested_field_offsets = (uint32_t*)malloc(sizeof(uint32_t) * MAX_COLUMNS);
	if (!requested_fields || !requested_field_offsets)
	{
		fprintf(stderr, "Blad: Nie udalo sie zaalokowac pamieci dla requested_fields!\n");
		goto cleanup;
	}

	int requested_fields_count = 0;

	if (_stricmp(requested_fields_str, "*") == 0)
	{
		for (int i = 0; i < target_table->column_count; i++)
		{
			requested_fields[i] = malloc(sizeof(SelectField));
			if (requested_fields[i] == NULL)
			{
				fprintf(stderr, "Blad: Nie udalo sie zaalokowac pamieci!\n");
				goto cleanup;
			}
			
			strncpy_s(requested_fields[i]->name, MAX_COL_NAME, target_table->columns[i].name, _TRUNCATE);
			requested_fields[i]->size = target_table->columns[i].size;
			requested_fields[i]->type = target_table->columns[i].type;
			requested_fields[i]->function_type = FunctionType_None;

			requested_field_offsets[requested_fields_count] = column_offsets[i];
			requested_fields_count++;
		}
	}
	else
	{
		char requested_fields_str_copy[512];
		strncpy_s(requested_fields_str_copy, sizeof(requested_fields_str_copy), requested_fields_str, _TRUNCATE);

		char* requested_field_context = NULL;
		char requested_field_separator[] = ",";
		char* requested_field_token = strtok_s(requested_fields_str_copy, requested_field_separator, &requested_field_context);

		while (requested_field_token != NULL)
		{
			if (requested_fields_count >= MAX_COLUMNS)
			{
				fprintf(stderr, "Blad: Przekroczono maksymalna liczbe kolumn w zapytaniu!\n");
				goto cleanup;
			}

			char* r_f_function_context = NULL;
			char r_f_function_separator[] = ":";
			char* r_f_function_token1 = strtok_s(requested_field_token, r_f_function_separator, &r_f_function_context);
			char* r_f_function_token2 = strtok_s(NULL, r_f_function_separator, &r_f_function_context);

			char* function_name = NULL;
			char* field_name = NULL;
			if (r_f_function_token2 == NULL)
			{
				field_name = r_f_function_token1;
			}
			else
			{
				function_name = r_f_function_token1;
				field_name = r_f_function_token2;
			}

			FunctionType functionType = str_to_function_type(function_name);

			Column* col = NULL;
			uint32_t offset = 0;
			for (int i = 0; i < target_table->column_count; i++)
			{
				if (_stricmp(field_name, target_table->columns[i].name) == 0)
				{
					col = &target_table->columns[i];
					offset = column_offsets[i];
					break;
				}
			}

			if (col == NULL)
			{
				fprintf(stderr, "Blad: Nie znaleziono pola '%s' w tabeli %s!\n", requested_field_token, table_name);
				goto cleanup;
			}

			SelectField* field = calloc(1, sizeof(SelectField));
			if (field == NULL)
			{
				fprintf(stderr, "Blad: Nie udalo sie zaalokowac pamieci!\n");
				goto cleanup;
			}
			strncpy_s(field->name, MAX_COL_NAME, col->name, _TRUNCATE);
			field->size = col->size;
			field->type = col->type;
			field->function_type = functionType;
			field->float_temp = 0.0f;
			field->int_temp = 0;
			field->uint_temp = 0;

			requested_fields[requested_fields_count] = field;

			requested_field_offsets[requested_fields_count] = offset;
			requested_fields_count++;
			requested_field_token = strtok_s(NULL, requested_field_separator, &requested_field_context);
		}
	}


	// ============= parse optional arguments =============

	int has_conditions = 0, has_order = 0;
	char* conditions_raw = NULL, * order_raw = NULL;
	uint8_t order_desc = 0;

	if (argc == 7)
	{
		has_conditions = (_strnicmp("where:", argv[6], 6) == 0);

		int has_order_asc = _strnicmp("order:", argv[6], 6) == 0;
		int has_order_desc = _strnicmp("order-desc:", argv[6], 6) == 0;
		has_order = has_order_asc || has_order_desc;
		order_desc = has_order_desc;

		if (!has_conditions && !has_order)
		{
			fprintf(stderr, "Blad: Brak slow kluczowych 'where:' i 'order:' w zapytaniu!\n");
			goto cleanup;
		}

		if (has_conditions)
		{
			conditions_raw = argv[6];
		}
		if (has_order)
		{
			order_raw = argv[6];
		}
	}

	if (argc == 8)
	{
		has_conditions = (_strnicmp("where:", argv[6], 6) == 0);

		int has_order_asc = _strnicmp("order:", argv[7], 6) == 0;
		int has_order_desc = _strnicmp("order-desc:", argv[7], 6) == 0;
		has_order = has_order_asc || has_order_desc;
		order_desc = has_order_desc;

		if (!has_conditions || !has_order)
		{
			fprintf(stderr, "Blad: Brak slow kluczowych 'where:' i 'order(-desc):' w zapytaniu!\n");
			goto cleanup;
		}

		conditions_raw = argv[6];
		order_raw = argv[7];
	}

	// ============= parse conditions =============

	int conditions_count = 0;

	if (has_conditions && parse_conditions(conditions_raw, &conditions, &conditions_count, target_table, column_offsets) != 0)
	{
		goto cleanup;
	}

	// ============= parse order =============

	if (has_order)
	{
		char* order_context = NULL;
		char order_separator[] = ":";

		strtok_s(order_raw, order_separator, &order_context);
		char* order_token = strtok_s(NULL, order_separator, &order_context);

		if (!order_token)
		{
			fprintf(stderr, "Blad: Niepoprawny format 'order:'!\n");
			goto cleanup;
		}

		int order_token_len = (int)strlen(order_token);

		if (order_token_len == 0)
		{
			fprintf(stderr, "Blad: Brak nazwy pola po 'order:'!\n");
			goto cleanup;
		}

		order_by = (char*)malloc(order_token_len + 1);
		if (!order_by)
		{
			fprintf(stderr, "Blad: Blad alokacji pamieci!\n");
			goto cleanup;
		}

		strcpy_s(order_by, order_token_len + 1, order_token);
	}

	// ============= collect items =============

	item_buffer = (char*)malloc(item_payload_size);
	if (!item_buffer)
	{
		fprintf(stderr, "Blad: Nie udalo sie zaalokowac pamieci bufora!\n");
		goto cleanup;
	}

	uint32_t current_offset = target_table->first_record_offset;

	uint32_t* matched_offsets = NULL;
	int matched_count = 0;
	int capacity = 0;

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
			if (matched_count >= capacity)
			{
				capacity = (capacity == 0) ? 16 : capacity * 2;
				uint32_t* temp = realloc(matched_offsets, capacity * sizeof(uint32_t));
				if (!temp)
				{
					fprintf(stderr, "Blad: Nie udalo sie zaalokowac pamieci!\n");
					goto cleanup;
				}
				matched_offsets = temp;
			}

			matched_offsets[matched_count++] = current_offset;
		}

		current_offset = rec_header.next_record_offset;
	}

	// ============= order items =============

	if (matched_count > 1 && order_by != NULL)
	{
		uint32_t column_offset = 0;
		Column* col = NULL;
		for (int i = 0; i < target_table->column_count; i++)
		{
			if (_stricmp(target_table->columns[i].name, order_by) == 0)
			{
				col = &target_table->columns[i];
				break;
			}
			column_offset += target_table->columns[i].size;
		}

		if (col == NULL)
		{
			fprintf(stderr, "Blad: Brak pola o nazwie '%s' w tabeli '%s'!\n", order_by, target_table->name);
			goto cleanup;
		}

		OrderContext order_context;
		order_context.db_file = db_file;
		order_context.data_type = col->type;
		order_context.order_desc = order_desc;
		order_context.payload_size = item_payload_size;
		order_context.column_offset = column_offset;

		qsort_s(
			matched_offsets,
			matched_count,
			sizeof(uint32_t),
			compare_records,
			&order_context
		);
	}

	// ============= print header =============

	printf("\n--- Tabela: %s ---\n", target_table->name);

	printf("LP   ");
	for (int j = 0; j < requested_fields_count; j++)
	{
		char header_buf[32];
		if (requested_fields[j]->function_type == FunctionType_None)
		{
			snprintf(header_buf, sizeof(header_buf), "%s", requested_fields[j]->name);
		}
		else
		{
			snprintf(header_buf, sizeof(header_buf), "%s:%s",
				function_type_to_str(requested_fields[j]->function_type),
				requested_fields[j]->name);
		}

		printf("%-15s", header_buf);
	}
	printf("\n");

	printf("-----");
	for (int j = 0; j < requested_fields_count; j++)
	{
		printf("---------------");
	}
	printf("\n");

	// ============= print items =============

	int index = 0;
	for (; index < matched_count; index++)
	{
		fseek(db_file, matched_offsets[index], SEEK_SET);

		RecordHeader rec_header;
		if (fread(&rec_header, sizeof(RecordHeader), 1, db_file) != 1)
		{
			break;
		}

		if (fread(item_buffer, item_payload_size, 1, db_file) != 1)
		{
			break;
		}

		printf("%-5d", index+1);

		for (int j = 0; j < requested_fields_count; j++)
		{
			SelectField* field = requested_fields[j];
			char* field_ptr = item_buffer + requested_field_offsets[j];

			// match every function type

			if (field->function_type == FunctionType_None)
			{
				if (field->type == TYPE_INT)
				{
					int32_t val = *(int32_t*)field_ptr;
					printf("%-15d", val);
				}
				else if (field->type == TYPE_INT_AUTO_INC)
				{
					uint32_t val = *(uint32_t*)field_ptr;
					printf("%-15u", val);
				}
				else if (field->type == TYPE_FLOAT)
				{
					float val = *(float*)field_ptr;
					printf("%-15.2f", val);
				}
				else if (field->type == TYPE_STRING)
				{
					printf("%-15.*s", (int)field->size, field_ptr);
				}
			}

			if (field->function_type == FunctionType_Max)
			{
				if (field->type == TYPE_INT)
				{
					int32_t val = *(int32_t*)field_ptr;
					if (index == 1)
						field->int_temp = val;
					if (val > field->int_temp)
						field->int_temp = val;
					printf("%-15d", field->int_temp);
				}
				else if (field->type == TYPE_INT_AUTO_INC)
				{
					uint32_t val = *(uint32_t*)field_ptr;
					if (index == 1)
						field->uint_temp = val;
					if (val > field->uint_temp)
						field->uint_temp = val;
					printf("%-15u", field->uint_temp);
				}
				else if (field->type == TYPE_FLOAT)
				{
					float val = *(float*)field_ptr;
					if (index == 1)
						field->float_temp = val;
					if (val > field->float_temp)
						field->float_temp = val;
					printf("%-15.2f", field->float_temp);
				}
			}

			if (field->function_type == FunctionType_Min)
			{
				if (field->type == TYPE_INT)
				{
					int32_t val = *(int32_t*)field_ptr;
					if (index == 1)
						field->int_temp = val;
					if (val < field->int_temp)
						field->int_temp = val;
					printf("%-15d", field->int_temp);
				}
				else if (field->type == TYPE_INT_AUTO_INC)
				{
					uint32_t val = *(uint32_t*)field_ptr;
					if (index == 1)
						field->uint_temp = val;
					if (val < field->uint_temp)
						field->uint_temp = val;
					printf("%-15u", field->uint_temp);
				}
				else if (field->type == TYPE_FLOAT)
				{
					float val = *(float*)field_ptr;
					if (index == 1)
						field->float_temp = val;
					if (val < field->float_temp)
						field->float_temp = val;
					printf("%-15.2f", field->float_temp);
				}
			}

			if (field->function_type == FunctionType_Avg)
			{
				float avg;
				if (field->type == TYPE_INT)
				{
					int32_t val = *(int32_t*)field_ptr;
					field->counter++;
					field->int_temp += val;
					avg = (float)field->int_temp / field->counter;
					printf("%-15.2f", avg);
				}
				else if (field->type == TYPE_INT_AUTO_INC)
				{
					uint32_t val = *(uint32_t*)field_ptr;
					field->counter++;
					field->uint_temp += val;
					avg = (float)field->uint_temp / field->counter;
					printf("%-15.2f", avg);
				}
				else if (field->type == TYPE_FLOAT)
				{
					float val = *(float*)field_ptr;
					field->counter++;
					field->float_temp += val;
					avg = (float)field->float_temp / field->counter;
					printf("%-15.2f", avg);
				}
			}

			if (field->function_type == FunctionType_Sum)
			{
				if (field->type == TYPE_INT)
				{
					int32_t val = *(int32_t*)field_ptr;
					field->int_temp += val;
					printf("%-15d", field->int_temp);
				}
				else if (field->type == TYPE_INT_AUTO_INC)
				{
					uint32_t val = *(uint32_t*)field_ptr;
					field->uint_temp += val;
					printf("%-15u", field->uint_temp);
				}
				else if (field->type == TYPE_FLOAT)
				{
					float val = *(float*)field_ptr;
					field->float_temp += val;
					printf("%-15.2f", field->float_temp);
				}
			}
		}
		printf("\n");
	}

	printf("\n");

	printf("Zwrocono rekordow: %d.\n", index);
	printf("Wszystkich rekordow: %d.\n", target_table->record_count);

	printf("\n");

	// ============= cleanup =============

	status = 0;

cleanup:
	if (requested_fields)
	{
		for (int i = 0; i < requested_fields_count; i++)
		{
			free(requested_fields[i]);
		}
		free(requested_fields);
	}
	free(conditions);
	free(requested_field_offsets);
	free(item_buffer);
	free(order_by);
	if (db_file)
	{
		fclose(db_file);
	}
	return status;
}

int compare_records(void* _context, const void* a, const void* b)
{
	OrderContext* context = (OrderContext*)_context;

	uint32_t offset_a = *(const uint32_t*)a;
	uint32_t offset_b = *(const uint32_t*)b;

	char* buf_a = malloc(context->payload_size);
	char* buf_b = malloc(context->payload_size);

	if (!buf_a || !buf_b)
	{
		free(buf_a);
		free(buf_b);
		return 0;
	}

	fseek(context->db_file, offset_a + sizeof(RecordHeader), SEEK_SET);
	fread(buf_a, context->payload_size, 1, context->db_file);

	fseek(context->db_file, offset_b + sizeof(RecordHeader), SEEK_SET);
	fread(buf_b, context->payload_size, 1, context->db_file);

	void* val_ptr_a = buf_a + context->column_offset;
	void* val_ptr_b = buf_b + context->column_offset;

	int result = 0;
	if (context->data_type == TYPE_INT)
	{
		int32_t va = *(int32_t*)val_ptr_a;
		int32_t vb = *(int32_t*)val_ptr_b;
		result = (va > vb) - (va < vb);
	}
	else if (context->data_type == TYPE_INT_AUTO_INC)
	{
		uint32_t va = *(uint32_t*)val_ptr_a;
		uint32_t vb = *(uint32_t*)val_ptr_b;
		result = (va > vb) - (va < vb);
	}
	else if (context->data_type == TYPE_FLOAT)
	{
		float va = *(float*)val_ptr_a;
		float vb = *(float*)val_ptr_b;
		result = (va > vb) - (va < vb);
	}
	else if (context->data_type == TYPE_STRING)
	{
		result = _stricmp((char*)val_ptr_a, (char*)val_ptr_b);
	}

	free(buf_a);
	free(buf_b);

	return context->order_desc ? -result : result;
}
