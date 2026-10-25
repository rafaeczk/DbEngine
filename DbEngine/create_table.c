#include "create_table.h"

#include "config.h"
#include "structures.h"
#include "print_help.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

int create_table(int argc, char* argv[])
{
	if (argc != 5)
	{
		fprintf(stderr, "Blad: Niepoprawna liczba argumentow!\n");
		print_help(argv[2]);
		return 1;
	}

	const char* db_filename = argv[1];
	const char* table_name = argv[3];

	if (strlen(table_name) > MAX_TABLE_NAME)
	{
		fprintf(stderr, "Blad: Nazwa tabeli '%s' jest za dluga (max %d znakow)!\n", table_name, MAX_TABLE_NAME - 1);
		return 1;
	}

	char table_schema[256];
	strncpy_s(table_schema, sizeof(table_schema), argv[4], _TRUNCATE);

	Column columns[MAX_COLUMNS];
	uint16_t column_count = 0;

	char field_separator[] = ",";
	char* field_context = NULL;

	char* field = strtok_s(table_schema, field_separator, &field_context);

	while (field != NULL)
	{
		if (column_count >= MAX_COLUMNS)
		{
			fprintf(stderr, "Blad: Przekroczono maksymalna liczbe kolumn (%d)!\n", MAX_COLUMNS);
			return 1;
		}

		char type_separator[] = ":";
		char* type_context = NULL;

		char* field_name = strtok_s(field, type_separator, &type_context);
		char* field_type = strtok_s(NULL, type_separator, &type_context);

		if (!field_name || !field_type)
		{
			fprintf(stderr, "Blad: Niepoprawny format kolumny '%s'. Oczekiwano 'nazwa:TYP'\n", field);
			return 1;
		}

		if (strlen(field_name) >= MAX_COL_NAME)
		{
			fprintf(stderr, "Blad: Nazwa kolumny '%s' jest za dluga (max %d znakow)!\n", field_name, MAX_COL_NAME - 1);
			return 1;
		}

		Column* col = &columns[column_count];
		memset(col, 0, sizeof(Column));
		strncpy_s(col->name, MAX_COL_NAME, field_name, _TRUNCATE);

		if (_stricmp(field_type, "INT") == 0)
		{
			col->type = TYPE_INT;
			col->size = sizeof(int32_t);
		}
		else if (_stricmp(field_type, "FLOAT") == 0)
		{
			col->type = TYPE_FLOAT;
			col->size = sizeof(float);
		}
		else if (_stricmp(field_type, "STRING") == 0)
		{
			col->type = TYPE_STRING;
			col->size = 32;
		}
		else
		{
			fprintf(stderr, "Blad: Nieznany typ danych '%s' dla kolumny '%s'!\n", field_type, field_name);
			return 1;
		}

		column_count++;
		field = strtok_s(NULL, field_separator, &field_context);
	}

	if (column_count == 0)
	{
		fprintf(stderr, "Blad: Schemat tabeli nie moze byc pusty!\n");
		return 1;
	}

	FILE* db_file = NULL;
	fopen_s(&db_file, db_filename, "rb+");

	DBHeader header;
	if (fread(&header, sizeof(DBHeader), 1, db_file) != 1 || header.magic != DB_MAGIC)
	{
		fprintf(stderr, "Blad: Wybrany plik '%s' nie jest poprawnym plikiem bazy danych!\n", db_filename);
		fclose(db_file);
		return 1;
	}

	if (header.table_count >= MAX_TABLES)
	{
		fprintf(stderr, "Blad: Osiagnieto maksymalna liczbe tabel (%d) w bazie!\n", MAX_TABLES);
		fclose(db_file);
		return 1;
	}

	for (uint32_t i = 0; i < header.table_count; i++)
	{
		if (_stricmp(header.tables[i].name, table_name) == 0)
		{
			fprintf(stderr, "Blad: Tabela '%s' juz istnieje w bazie!\n", table_name);
			fclose(db_file);
			return 1;
		}
	}

	TableEntry* new_table = &header.tables[header.table_count];
	memset(new_table, 0, sizeof(TableEntry));
	strncpy_s(new_table->name, MAX_TABLE_NAME, table_name, _TRUNCATE);
	new_table->column_count = column_count;

	for (uint16_t i = 0; i < column_count; i++)
	{
		new_table->columns[i] = columns[i];
	}

	header.table_count++;

	// Nadpisujemy nagłówek na początku pliku
	fseek(db_file, 0, SEEK_SET);
	fwrite(&header, sizeof(DBHeader), 1, db_file);
	fflush(db_file);
	fclose(db_file);

	printf("Utworzono tabele '%s' (%u kolumn) w bazie '%s'.\n", table_name, column_count, db_filename);
	return 0;
}
