#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stddef.h>

#include "delete_from.h"
#include "structures.h"
#include "config.h"
#include "print_help.h"

int delete_from(int argc, char* argv[]) {
    if (argc < 5) {
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

    int records_count_to_delete = argc - 4;

    uint32_t* offsets_to_delete = (uint32_t*)calloc(records_count_to_delete, sizeof(uint32_t));
    if (!offsets_to_delete) {
        fprintf(stderr, "Blad alokacji pamieci!\n");
        fclose(db_file);
        return 1;
    }

    int deleted_records_count = 0;

    for (int i = 4; i < argc; i++) {
        int row_num = atoi(argv[i]);
        if (row_num <= 0) continue;

        uint32_t current_offset = target_table->first_record_offset;
        int active_row_index = 1;

        while (current_offset != 0) {
            fseek(db_file, current_offset, SEEK_SET);

            RecordHeader rec_header;
            if (fread(&rec_header, sizeof(RecordHeader), 1, db_file) != 1) {
                break;
            }

            uint32_t next_offset = rec_header.next_record_offset;

            if (!rec_header.is_deleted) {
                if (active_row_index == row_num) {
                    int already_added = 0;
                    for (int k = 0; k < deleted_records_count; k++) {
                        if (offsets_to_delete[k] == current_offset) {
                            already_added = 1;
                            break;
                        }
                    }

                    if (!already_added) {
                        offsets_to_delete[deleted_records_count++] = current_offset;
                    }

                    break;
                }
                active_row_index++;
            }

            current_offset = next_offset;
        }
    }

    for (int i = 0; i < deleted_records_count; i++) {
        uint8_t tombstone = 1;
        fseek(db_file, offsets_to_delete[i] + offsetof(RecordHeader, is_deleted), SEEK_SET);
        fwrite(&tombstone, sizeof(uint8_t), 1, db_file);
    }

    target_table->record_count -= deleted_records_count;
    fseek(db_file, 0, SEEK_SET);
    fwrite(&header, sizeof(DBHeader), 1, db_file);

    free(offsets_to_delete);
    fflush(db_file);
    fclose(db_file);

    printf("Pomyslnie usunieto %d z %d rekord(ow) z tabeli '%s'.\n", deleted_records_count, records_count_to_delete, table_name);

    return 0;
}
