#include "data_type_to_string.h"

const char* data_type_to_string(DataType data_type)
{
	switch (data_type)
	{
	case TYPE_INT:
		return "INT";
	case TYPE_FLOAT:
		return "FLOAT";
	case TYPE_STRING:
		return "STRING";
	default:
		return "UNKNOWN";
	}
}