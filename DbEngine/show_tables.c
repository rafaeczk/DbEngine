#include "show_tables.h"

#include "structures.h"
#include "data_type_to_string.h"

#include <stdio.h>

int show_tables(int argc, char* argv[]) {
    const char* db_filename = argv[1];

	FILE* db_file;
	fopen_s(&db_file, db_filename, "r");

    DBHeader header;
    if (fread(&header, sizeof(DBHeader), 1, db_file) != 1 || header.magic != DB_MAGIC) {
        fprintf(stderr, "Blad: Wybrany plik '%s' nie jest poprawnym plikiem bazy danych!\n", db_filename);
        fclose(db_file);
        return 1;
    }
    
    printf("\n--- Tabele ---\n");
    for (int i = 0; i < header.table_count; i++) {
        printf("\n-- %s --\n", header.tables[i].name);

        char* fields_label = "Pola: ";
        printf("%-8s", fields_label);
        for (int j = 0; j < header.tables[i].column_count; j++) {
            Column* col = &header.tables[i].columns[j];
            printf("%-15s", col->name);
        }

        printf("\n");

        char* types_label = "Typy: ";
        printf("%-8s", types_label);
        for (int j = 0; j < header.tables[i].column_count; j++) {
            Column* col = &header.tables[i].columns[j];
            printf("%-15s", data_type_to_string(col->type));
        }

        printf("\n");
    }

    printf("\n");

    return 0;
}
