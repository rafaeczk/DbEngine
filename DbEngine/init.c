#include "init.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "config.h"
#include "structures.h"

int init(int argc, char* argv[])
{
	int status = 1;
	FILE* db_file = NULL;
	const char* db_filename = argv[1];

	fopen_s(&db_file, db_filename, "rb+");

	if (!db_file)
	{
		fopen_s(&db_file, db_filename, "wb+");
		if (!db_file)
		{
			fprintf(stderr, "Blad: Nie mozna utworzyc ani otworzyc pliku bazy '%s'!\n", db_filename);
			goto cleanup;
		}

		DBHeader header;
		memset(&header, 0, sizeof(DBHeader));
		header.magic = DB_MAGIC;
		header.version = 1;
		header.table_count = 0;

		if (fwrite(&header, sizeof(DBHeader), 1, db_file) != 1)
		{
			fprintf(stderr, "Blad: Zapis naglowka bazy danych do pliku '%s' nie powiodl sie!\n", db_filename);
			goto cleanup;
		}

		fflush(db_file);
		printf("\nInicjalizacja nowej bazy danych: %s\n\n", db_filename);
	}
	else
	{
		DBHeader header;
		if (fread(&header, sizeof(DBHeader), 1, db_file) != 1 || header.magic != DB_MAGIC)
		{
			fprintf(stderr, "Blad: Wybrany plik '%s' nie jest poprawnym plikiem bazy danych!\n", db_filename);
			goto cleanup;
		}
	}

	status = 0;

cleanup:
	if (db_file)
	{
		fclose(db_file);
	}
	return status;
}
