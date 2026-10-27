#pragma once

#include <stdint.h>
#include "config.h"

typedef enum
{
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
	Column columns[MAX_COLUMNS];
} TableEntry;

typedef struct
{
	uint32_t magic;
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
