#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "config.h"
#include "structures.h"

#include "create_table.h"
#include "drop_table.h"
#include "show_tables.h"
#include "insert_into.h"
#include "select_from.h"
#include "delete_from.h"
#include "print_help.h"
#include "init.h"

int main(int argc, char *argv[]) {
    // HELP

    if (_stricmp(argv[1], "help") == 0) {
        print_help(NULL);
        return 0;
    }

    // INIT

    const char* db_filename = NULL;
    const char* command = NULL;

    if (init(argc, argv, &db_filename, &command) != 0) {
        return 1;
    }

    // WRITE

    if (_stricmp(command, "create-table") == 0) {
        if (create_table(argc, argv) != 0) {
            return 1;
        }
    }

    if (_stricmp(command, "drop-table") == 0) {
        if (drop_table(argc, argv) != 0) {
            return 1;
        }
    }

    if (_stricmp(command, "insert-into") == 0) {
        if (insert_into(argc, argv) != 0) {
            return 1;
        }
    }

    // DELETE

    if (_stricmp(command, "delete-from") == 0) {
        if (delete_from(argc, argv) != 0) {
            return 1;
        }
    }

    // READ

    if (_stricmp(command, "show-tables") == 0) {
        if (show_tables(argc, argv) != 0) {
            return 1;
        }
    }

    if (_stricmp(command, "select-from") == 0) {
        if (select_from(argc, argv) != 0) {
            return 1;
        }
    }

    return 0;
}
