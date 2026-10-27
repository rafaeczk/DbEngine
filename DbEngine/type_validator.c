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

	// Pomin biale znaki na poczatku
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
	long val = strtol(str, &end_ptr, 10);

	// Sprawdz czy cokolwiek zostalo sparsowane
	if (end_ptr == str)
	{
		return false;
	}

	// Sprawdz overflow dla standardowego int32_t
	if (errno == ERANGE || val < INT32_MIN || val > INT32_MAX)
	{
		return false;
	}

	// Pomin ewentualne biale znaki na koncu
	while (isspace((unsigned char)*end_ptr))
	{
		end_ptr++;
	}

	// Jesli zostaly jakies inne znaki (np. "123abc"), to nie jest poprawny int
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

	// Pomin biale znaki na poczatku
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
	float val = strtof(str, &end_ptr);

	// Sprawdz czy cokolwiek zostalo sparsowane
	if (end_ptr == str)
	{
		return false;
	}

	// Sprawdz overflow / underflow
	if (errno == ERANGE)
	{
		return false;
	}

	// Pomin ewentualne biale znaki na koncu
	while (isspace((unsigned char)*end_ptr))
	{
		end_ptr++;
	}

	// Jesli zostaly inne znaki (np. "12.34xyz"), to nie jest poprawny float
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
