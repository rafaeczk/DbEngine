#pragma once

#include <stdint.h>
#include <stdio.h>
#include "config.h"

typedef enum
{
	TYPE_INT_AUTO_INC,
	TYPE_INT,
	TYPE_FLOAT,
	TYPE_STRING
} DataType;

#pragma pack(push, 1)

typedef struct
{
	char name[MAX_COL_NAME];
	DataType type;
	uint16_t size;
} Column;

typedef struct
{
	char name[MAX_TABLE_NAME];
	uint32_t first_record_offset;
	uint32_t last_record_offset;
	uint32_t record_count;
	uint16_t column_count;
	uint32_t next_autoincrement_value;
	Column columns[MAX_COLUMNS];
} TableEntry;

typedef struct
{
	uint64_t magic;
	uint16_t version;
	uint32_t table_count;
	TableEntry tables[MAX_TABLES];
} DBHeader;

typedef struct
{
	uint8_t is_deleted;
	uint32_t next_record_offset;
	uint32_t payload_size;
} RecordHeader;

#pragma pack(pop)

typedef enum
{
	ConditionType_Equal,
	ConditionType_NonEqual,
	ConditionType_SmallerThan,
	ConditionType_LargerThan,
	ConditionType_In
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

typedef enum
{
	FunctionType_None,
	FunctionType_Sum,
	FunctionType_Avg,
	FunctionType_Min,
	FunctionType_Max
} FunctionType;

typedef struct
{
	char name[MAX_COL_NAME];
	DataType type;
	uint16_t size;
	FunctionType function_type;
	uint32_t uint_temp;
	int32_t int_temp;
	float float_temp;
	uint32_t counter;
} SelectField;

typedef struct
{
	FILE* db_file;
	uint32_t column_offset;
	DataType data_type;
	uint8_t order_desc;
	uint32_t payload_size;
} OrderContext;
