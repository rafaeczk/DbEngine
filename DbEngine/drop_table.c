#include "drop_table.h"

#include "structures.h"
#include "config.h"
#include "print_help.h"

#include <stdio.h>

int drop_table(int argc, char* argv[]) {
	if (argc != 4) {
		fprintf(stderr, "Blad: Niepoprawna liczba argumentow!\n");
		print_help(argv[2]);
		return 1;
	}

	const char* db_filename = argv[1];
	const char* table_name = argv[3];

	FILE* db_file = NULL;
	fopen_s(&db_file, db_filename, "rb+");
	if (!db_file) {
		fprintf(stderr, "Blad: Nie mozna otworzyc pliku bazy '%s'!\n", db_filename);
		return 1;
	}

	DBHeader header;
	if (fread(&header, sizeof(DBHeader), 1, db_file) != 1 || header.magic != DB_MAGIC) {
		fprintf(stderr, "Blad: Niepoprawny plik bazy danych!\n");
		fclose(db_file);
		return 1;
	}

	int table_index = -1;
	for (uint32_t i = 0; i < header.table_count; i++) {
		if (_stricmp(header.tables[i].name, table_name) == 0) {
			table_index = (int)i;
			break;
		}
	}

	if (table_index == -1) {
		fprintf(stderr, "Blad: Tabela '%s' nie istnieje w bazie!\n", table_name);
		fclose(db_file);
		return 1;
	}

	TableEntry* target_table = &header.tables[table_index];

	uint32_t current_offset = target_table->first_record_offset;
	while (current_offset != 0) {
		fseek(db_file, current_offset, SEEK_SET);

		RecordHeader rec_header;
		if (fread(&rec_header, sizeof(RecordHeader), 1, db_file) != 1) {
			break;
		}

		rec_header.is_deleted = 1;

		fseek(db_file, current_offset, SEEK_SET);
		fwrite(&rec_header, sizeof(RecordHeader), 1, db_file);

		current_offset = rec_header.next_record_offset;
	}

	for (uint32_t i = (uint32_t)table_index; i < header.table_count - 1; i++) {
		header.tables[i] = header.tables[i + 1];
	}

	memset(&header.tables[header.table_count - 1], 0, sizeof(TableEntry));
	header.table_count--;

	fseek(db_file, 0, SEEK_SET);
	fwrite(&header, sizeof(DBHeader), 1, db_file);
	fflush(db_file);
	fclose(db_file);

	printf("Tabela '%s' oraz jej rekordy zostaly usuniete z bazy '%s'.\n", table_name, db_filename);
	return 0;
}
