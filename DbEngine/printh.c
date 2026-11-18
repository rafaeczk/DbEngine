#include "printh.h"

#include <stdio.h>
#include <stdlib.h>

void printh_int(int32_t val, uint8_t human_readable)
{
	if (human_readable)
		printf("%-15d", val);
	else
		printf("%d", val);
}

void printh_uint(uint32_t val, uint8_t human_readable)
{
	if (human_readable)
		printf("%-15u", val);
	else
		printf("%u", val);
}

void printh_float(float val, uint8_t human_readable)
{
	if (human_readable)
		printf("%-15.2f", val);
	else
		printf("%f", val);
}

void printh_str(char* val, uint8_t human_readable, size_t size)
{
	if (human_readable)
		printf("%-15.*s", (int)size, val);
	else
		printf("%s", val);
}
