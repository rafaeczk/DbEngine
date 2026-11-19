cmake_minimum_required(VERSION 4.4)
project(DbEngine C)

set(CMAKE_C_STANDARD 11)

include_directories(.)

add_executable(DbEngine
        check_conditions.c
        check_conditions.h
        config.h
        create_table.c
        create_table.h
        data_type_to_string.c
        data_type_to_string.h
        delete_from.c
        delete_from.h
        drop_table.c
        drop_table.h
        function_type_to_str.c
        function_type_to_str.h
        init.c
        init.h
        insert_into.c
        insert_into.h
        main.c
        parse_conditions.c
        parse_conditions.h
        print_help.c
        print_help.h
        printh.c
        printh.h
        select_from.c
        select_from.h
        show_tables.c
        show_tables.h
        str_to_function_type.c
        str_to_function_type.h
        structures.h
        type_validator.c
        type_validator.h
        update.c
        update.h
        vacuum.c
        vacuum.h
        windows.h)
