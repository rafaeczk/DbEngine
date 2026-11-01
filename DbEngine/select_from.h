#pragma once

#include "config.h"
#include "structures.h"

int select_from(int argc, char* argv[]);

typedef struct
{
	Column* col;
	uint32_t offset;
	int32_t int_val;
	float float_val;
	char str_val[256];
} Condition;
