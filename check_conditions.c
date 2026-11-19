#include "check_conditions.h"

#include "structures.h"

#include <math.h>
#include <string.h>

int check_conditions(int conditions_count, Condition* conditions, char* item_buffer)
{
    for (int k = 0; k < conditions_count; k++)
    {
        Condition* cond = &conditions[k];
        const char* field_ptr = item_buffer + cond->offset;

        if (cond->col->type == TYPE_INT || cond->col->type == TYPE_INT_AUTO_INC)
        {
            int32_t rec_val = *(int32_t*)field_ptr;
            int condition_met = 1;

            switch (cond->condition_type)
            {
            case ConditionType_Equal:
                condition_met = (rec_val == cond->int_val);
                break;
            case ConditionType_NonEqual:
                condition_met = (rec_val != cond->int_val);
                break;
            case ConditionType_LargerThan:
                condition_met = (rec_val > cond->int_val);
                break;
            case ConditionType_SmallerThan:
                condition_met = (rec_val < cond->int_val);
                break;
            default:
                condition_met = 0;
                break;
            }

            if (condition_met == 0)
            {
                return 0;
            }
        }
        else if (cond->col->type == TYPE_FLOAT)
        {
            float rec_val = *(float*)field_ptr;
            int condition_met = 1;
            float eps = 0.0001f;

            switch (cond->condition_type)
            {
            case ConditionType_Equal:
                condition_met = (fabsf(rec_val - cond->float_val) <= eps);
                break;
            case ConditionType_NonEqual:
                condition_met = (fabsf(rec_val - cond->float_val) > eps);
                break;
            case ConditionType_LargerThan:
                condition_met = (rec_val - cond->float_val > eps);
                break;
            case ConditionType_SmallerThan:
                condition_met = (cond->float_val - rec_val > eps);
                break;
            default:
                condition_met = 0;
                break;
            }

            if (condition_met == 0)
            {
                return 0;
            }
        }
        else if (cond->col->type == TYPE_STRING)
        {
            char field_str[256];
            uint16_t max_len = cond->col->size < sizeof(field_str) - 1 ? cond->col->size : (uint16_t)(sizeof(field_str) - 1);
            memcpy(field_str, field_ptr, max_len);
            field_str[max_len] = '\0';

            int cmp_result = _strnicmp(field_str, cond->str_val, cond->col->size);
            int condition_met = 1;

            switch (cond->condition_type)
            {
            case ConditionType_Equal:
                condition_met = (cmp_result == 0);
                break;
            case ConditionType_NonEqual:
                condition_met = (cmp_result != 0);
                break;
            case ConditionType_LargerThan:
                condition_met = (cmp_result > 0);
                break;
            case ConditionType_SmallerThan:
                condition_met = (cmp_result < 0);
                break;
            case ConditionType_In:
                condition_met = strstr(field_str, cond->str_val) != NULL;
                break;
            default:
                condition_met = 0;
                break;
            }

            if (condition_met == 0)
            {
                return 0;
            }
        }
    }

    return 1;
}
