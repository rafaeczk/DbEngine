#include "type_validator.h"

#include <stdlib.h>
#include <ctype.h>
#include <errno.h>

bool is_valid_int(const char* str, int32_t* out_val)
{
	if (!str)
	{
		return false;
	}

	while (isspace((unsigned char)*str))
	{
		str++;
	}

	if (*str == '\0')
	{
		return false;
	}

	char* end_ptr = NULL;
	errno = 0;
	const long val = strtol(str, &end_ptr, 10);

	if (end_ptr == str)
	{
		return false;
	}

	if (errno == ERANGE || val < INT32_MIN || val > INT32_MAX)
	{
		return false;
	}

	while (isspace((unsigned char)*end_ptr))
	{
		end_ptr++;
	}

	if (*end_ptr != '\0')
	{
		return false;
	}

	if (out_val)
	{
		*out_val = (int32_t)val;
	}

	return true;
}

bool is_valid_float(const char* str, float* out_val)
{
	if (!str)
	{
		return false;
	}

	while (isspace((unsigned char)*str))
	{
		str++;
	}

	if (*str == '\0')
	{
		return false;
	}

	char* end_ptr = NULL;
	errno = 0;
	const float val = strtof(str, &end_ptr);

	if (end_ptr == str)
	{
		return false;
	}

	if (errno == ERANGE)
	{
		return false;
	}

	while (isspace((unsigned char)*end_ptr))
	{
		end_ptr++;
	}

	if (*end_ptr != '\0')
	{
		return false;
	}

	if (out_val)
	{
		*out_val = val;
	}

	return true;
}
