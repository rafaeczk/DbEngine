#pragma once

#include <stdbool.h>
#include <stdint.h>

bool is_valid_int(const char* str, int32_t* out_val);

bool is_valid_float(const char* str, float* out_val);
