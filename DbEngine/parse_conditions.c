#include "parse_conditions.h"

#include "config.h"
#include "structures.h"
#include "print_help.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <math.h>

int parse_conditions(const char* conditions_str, Condition** conditions, int* out_count, TableEntry* target_table, const uint32_t* column_offsets)
{
    if (!conditions_str || !conditions || !out_count || !target_table || !column_offsets)
    {
        fprintf(stderr, "Blad: Niepoprawne argumenty wejsciowe do parse_conditions!\n");
        return 1;
    }

    *conditions = (Condition*)malloc(MAX_COLUMNS * sizeof(Condition));
    if (!(*conditions))
    {
        fprintf(stderr, "Blad: Nie udalo sie zaalokowac pamieci dla conditions!\n");
        return 1;
    }

    int conditions_count = 0;
    const char* conditions_no_where = NULL;

    if (strncmp(conditions_str, "where:", 6) != 0)
    {
        fprintf(stderr, "Blad: Niepoprawne argumenty! Oczekiwano parametru zaczynajacego sie od 'where:'\n");
        free(*conditions);
        *conditions = NULL;
        return 1;
    }
    else
    {
        conditions_no_where = conditions_str + 6;
    }

    char conditions_str_copy[512];
    strncpy_s(conditions_str_copy, sizeof(conditions_str_copy), conditions_no_where, _TRUNCATE);

    char* condition_context = NULL;
    char condition_separator[] = ",";
    char* condition_token = strtok_s(conditions_str_copy, condition_separator, &condition_context);

    while (condition_token != NULL)
    {
        if (conditions_count >= MAX_COLUMNS)
        {
            fprintf(stderr, "Blad: Przekroczono maksymalna liczbe warunkow w zapytaniu!\n");
            free(*conditions);
            *conditions = NULL;
            return 1;
        }

        char condition_type_separator[] = "=!><";
        char* op_ptr = strpbrk(condition_token, condition_type_separator);
        if (op_ptr == NULL)
        {
            fprintf(stderr, "Blad: Brak operatora (=, !, >, <) w warunku.\n");
            free(*conditions);
            *conditions = NULL;
            return 1;
        }

        char operator_char = *op_ptr;
        *op_ptr = '\0';

        char* field_name = condition_token;
        char* field_value = op_ptr + 1;

        if (*field_name == '\0' || *field_value == '\0')
        {
            fprintf(stderr, "Blad: Niepoprawny format warunku (brak nazwy pola lub wartosci).\n");
            free(*conditions);
            *conditions = NULL;
            return 1;
        }

        Column* matched_col = NULL;
        uint32_t col_offset = 0;
        for (int i = 0; i < target_table->column_count; i++)
        {
            if (_stricmp(target_table->columns[i].name, field_name) == 0)
            {
                matched_col = &target_table->columns[i];
                col_offset = column_offsets[i];
                break;
            }
        }

        if (matched_col == NULL)
        {
            fprintf(stderr, "Blad: Niepoprawna nazwa kolumny '%s' w warunku.\n", field_name);
            free(*conditions);
            *conditions = NULL;
            return 1;
        }

        Condition* cond = &((*conditions)[conditions_count]);
        cond->col = matched_col;
        cond->offset = col_offset;

        switch (operator_char)
        {
        case '=':
            cond->condition_type = ConditionTypeEqual; break;
        case '!':
            cond->condition_type = ConditionTypeNonEqual; break;
        case '>':
            cond->condition_type = ConditionTypeLargerThan; break;
        case '<':
            cond->condition_type = ConditionTypeSmallerThan; break;
        }

        if (matched_col->type == TYPE_INT)
        {
            cond->int_val = (int32_t)atoi(field_value);
            // printf("Int val: %d\n", cond->int_val);
        }
        else if (matched_col->type == TYPE_FLOAT)
        {
            cond->float_val = (float)atof(field_value);
            // printf("Float val: %f\n", cond->float_val);
        }
        else if (matched_col->type == TYPE_STRING)
        {
            strncpy_s(cond->str_val, sizeof(cond->str_val), field_value, _TRUNCATE);
            // printf("String val: %s\n", cond->str_val);
        }

        conditions_count++;
        condition_token = strtok_s(NULL, condition_separator, &condition_context);
    }

    *out_count = conditions_count;
    return 0;
}