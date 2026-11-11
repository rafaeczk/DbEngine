#pragma once

#include "config.h"
#include "structures.h"

int select_from(int argc, char* argv[]);

typedef enum
{
	ConditionTypeEqual,
	ConditionTypeNonEqual,
	ConditionTypeSmallerThan,
	ConditionTypeLargerThan
} ConditionType;

typedef struct
{
	Column* col;
	uint32_t offset;
	int32_t int_val;
	float float_val;
	char str_val[256];
	ConditionType condition_type;
} Condition;
