#include "delete_from.h"

#include "structures.h"
#include "config.h"
#include "print_help.h"

#include <stdio.h>
#include <stdlib.h>

int delete_from(int argc, char* argv[]) {
    if (argc != 5) {
        fprintf(stderr, "Blad: Niepoprawna liczba argumentow!\n");
        print_help(argv[2]);
        return 1;
    }

    const char* db_filename = argv[1];
    const char* table_name = argv[3];
    const char* row_num_str = argv[4];
    
    int row_num = atoi(row_num_str);

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
    int active_row_index = 1;

    while (current_offset != 0) {
        fseek(db_file, current_offset, SEEK_SET);

        RecordHeader rec_header;
        if (fread(&rec_header, sizeof(RecordHeader), 1, db_file) != 1) {
            break;
        }

        if (!rec_header.is_deleted) {
            if (active_row_index == row_num) {
                uint8_t tombstone = 1;

                fseek(db_file, current_offset + offsetof(RecordHeader, is_deleted), SEEK_SET);
                fwrite(&tombstone, sizeof(uint8_t), 1, db_file);

                fflush(db_file);
                fclose(db_file);

                printf("Usunieto wiersz nr %d z tabeli '%s'.\n", row_num, table_name);
                return 0;
            }
            active_row_index++;
        }

        current_offset = rec_header.next_record_offset;
    }

    printf("Nie znaleziono wiersza o numerze %d w tabeli %s.", row_num, table_name);
    return 1;
}
