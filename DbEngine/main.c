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
#include "update.h"
#include "vacuum.h"
#include "print_help.h"
#include "init.h"

int main(int argc, char* argv[])
{
	uint8_t human_readable = 0;

	if (argc > 1 && !_stricmp(argv[1], "-h"))
	{
		human_readable = 1;
		argc--;
		argv++;
	}

	// HELP

	if (argc >= 2 && _stricmp(argv[1], "help") == 0)
	{
		print_help(argc >= 3 ? argv[2] : NULL);
		return 0;
	}

	if (argc < 3)
	{
		fprintf(stderr, "Blad: Za malo argumentow!\n");
		print_help(NULL);
		return 1;
	}

	// INIT

	if (init(argc, argv, human_readable) != 0)
	{
		return 1;
	}

	// WRITE

	if (_stricmp(argv[2], "create-table") == 0)
	{
		return create_table(argc, argv, human_readable);
	}

	if (_stricmp(argv[2], "drop-table") == 0)
	{
		return drop_table(argc, argv, human_readable);
	}

	if (_stricmp(argv[2], "insert-into") == 0)
	{
		return insert_into(argc, argv, human_readable);
	}

	if (_stricmp(argv[2], "update") == 0)
	{
		return update(argc, argv, human_readable);
	}

	// DELETE

	if (_stricmp(argv[2], "delete-from") == 0)
	{
		return delete_from(argc, argv, human_readable);
	}

	// MAINTENANCE

	if (_stricmp(argv[2], "vacuum") == 0)
	{
		return vacuum(argc, argv, human_readable);
	}

	// READ

	if (_stricmp(argv[2], "show-tables") == 0)
	{
		return show_tables(argc, argv, human_readable);
	}

	if (_stricmp(argv[2], "select") == 0)
	{
		return select_from(argc, argv, human_readable);
	}

	fprintf(stderr, "Blad: Nieznana komenda '%s'!\n", argv[2]);
	print_help(NULL);
	return 1;
}
