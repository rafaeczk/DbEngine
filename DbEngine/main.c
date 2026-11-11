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
#include "vacuum.h"
#include "print_help.h"
#include "init.h"

int main(int argc, char* argv[])
{
	if (argc < 3)
	{
		fprintf(stderr, "Blad: Za malo argumentow!\n");
		print_help(NULL);
		return 1;
	}

	// HELP

	if (_stricmp(argv[1], "help") == 0)
	{
		print_help(argv[2] ? argv[2] : NULL);
		return 0;
	}

	// INIT

	if (init(argc, argv) != 0)
	{
		return 1;
	}

	// WRITE

	if (_stricmp(argv[2], "create-table") == 0)
	{
		if (create_table(argc, argv) != 0)
		{
			return 1;
		}
	}

	if (_stricmp(argv[2], "drop-table") == 0)
	{
		if (drop_table(argc, argv) != 0)
		{
			return 1;
		}
	}

	if (_stricmp(argv[2], "insert-into") == 0)
	{
		if (insert_into(argc, argv) != 0)
		{
			return 1;
		}
	}

	// DELETE

	if (_stricmp(argv[2], "delete-from") == 0)
	{
		if (delete_from(argc, argv) != 0)
		{
			return 1;
		}
	}

	// MAINTENANCE

	if (_stricmp(argv[2], "vacuum") == 0)
	{
		if (vacuum(argc, argv) != 0)
		{
			return 1;
		}
	}

	// READ

	if (_stricmp(argv[2], "show-tables") == 0)
	{
		if (show_tables(argc, argv) != 0)
		{
			return 1;
		}
	}

	if (_stricmp(argv[2], "select") == 0)
	{
		if (select_from(argc, argv) != 0)
		{
			return 1;
		}
	}

	return 0;
}
