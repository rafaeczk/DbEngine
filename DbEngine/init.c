#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "init.h"
#include "config.h"
#include "structures.h"

int init(int argc, char* argv[])
{
	const char* db_filename = argv[1];

	FILE* db_file = NULL;
	fopen_s(&db_file, db_filename, "rb+");

	if (!db_file)
	{
		fopen_s(&db_file, db_filename, "wb+");
		if (!db_file)
		{
			perror("Blad: Nie mozna utworzyc ani otworzyc pliku bazy");
			return 1;
		}

		DBHeader header;
		memset(&header, 0, sizeof(DBHeader));
		header.magic = DB_MAGIC;
		header.version = 1;
		header.table_count = 0;

		fwrite(&header, sizeof(DBHeader), 1, db_file);
		fflush(db_file);
		printf("Inicjalizacja nowej bazy danych: %s\n", db_filename);
	}
	else
	{
		DBHeader header;
		if (fread(&header, sizeof(DBHeader), 1, db_file) != 1 || header.magic != DB_MAGIC)
		{
			fprintf(stderr, "Blad: Wybrany plik '%s' nie jest poprawnym plikiem bazy danych!\n", db_filename);
			fclose(db_file);
			return 1;
		}
	}

	fclose(db_file);
	return 0;
}
