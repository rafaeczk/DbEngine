#include "str_to_function_type.h"

#include "structures.h"

#include <string.h>

FunctionType str_to_function_type(const char* str)
{
    if (str == NULL)
    {
        return FunctionType_None;
    }

    if (_stricmp(str, "sum") == 0)
    {
        return FunctionType_Sum;
    }
    if (_stricmp(str, "avg") == 0)
    {
        return FunctionType_Avg;
    }
    if (_stricmp(str, "min") == 0)
    {
        return FunctionType_Min;
    }
    if (_stricmp(str, "max") == 0)
    {
        return FunctionType_Max;
    }

    return FunctionType_None;
}
