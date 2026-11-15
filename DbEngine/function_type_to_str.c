#include "function_type_to_str.h"

char* function_type_to_str(FunctionType function_type)
{
    switch (function_type)
    {
    case FunctionType_None:
        return "";
    case FunctionType_Min:
        return "min";
    case FunctionType_Max:
        return "max";
    case FunctionType_Avg:
        return "avg";
    case FunctionType_Sum:
        return "sum";
    default:
        return "";
    }
}
