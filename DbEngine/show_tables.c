#include "show_tables.h"

#include <stdio.h>

#include "structures.h"
#include "data_type_to_string.h"

int show_tables(int argc, char* argv[])
{
	int status = 1;
	FILE* db_file = NULL;
	const char* db_filename = argv[1];

	fopen_s(&db_file, db_filename, "rb");
	if (!db_file)
	{
		fprintf(stderr, "Blad: Nie mozna otworzyc pliku bazy '%s'!\n", db_filename);
		goto cleanup;
	}

	DBHeader header;
	if (fread(&header, sizeof(DBHeader), 1, db_file) != 1 || header.magic != DB_MAGIC)
	{
		fprintf(stderr, "Blad: Wybrany plik '%s' nie jest poprawnym plikiem bazy danych!\n", db_filename);
		goto cleanup;
	}

	printf("\n--- Tabele (Liczba tabel: %u) ---\n", header.table_count);
	for (uint32_t i = 0; i < header.table_count; i++)
	{
		printf("\n-- Tabela: %s (Kolumn: %u, Rekordow: %u) --\n", header.tables[i].name, header.tables[i].column_count, header.tables[i].record_count);

		const char* fields_label = "Pola: ";
		printf("%-8s", fields_label);
		for (uint16_t j = 0; j < header.tables[i].column_count; j++)
		{
			Column* col = &header.tables[i].columns[j];
			printf("%-15s", col->name);
		}

		printf("\n");

		const char* types_label = "Typy: ";
		printf("%-8s", types_label);
		for (uint16_t j = 0; j < header.tables[i].column_count; j++)
		{
			Column* col = &header.tables[i].columns[j];
			printf("%-15s", data_type_to_string(col->type));
		}

		printf("\n");
	}

	printf("\n");
	status = 0;

cleanup:
	if (db_file)
	{
		fclose(db_file);
	}
	return status;
}
