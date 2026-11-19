#pragma once

#include<stdint.h>

void printh_int(int32_t val, uint8_t human_readable);
void printh_uint(uint32_t val, uint8_t human_readable);
void printh_float(float val, uint8_t human_readable);
void printh_str(char* val, uint8_t human_readable, size_t size);
