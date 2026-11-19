#pragma once

#include "structures.h"

int parse_conditions(const char* conditions_str, Condition** conditions, int* out_count, TableEntry* target_table, const uint32_t* column_offsets);
