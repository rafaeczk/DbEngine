#include <stdio.h>
#include <string.h>
#include "print_help.h"

void print_help(const char* command) {
    printf("=================================== POMOC ===================================\n");

    if (command != NULL) {
        if (_stricmp(command, "create-table") == 0) {
            printf(" Uzycie dla: create-table\n");
            printf("   <db_file> create-table <nazwa_tabeli> <pole1:typ,pole2:typ...>\n\n");
            printf(" - nazwa_tabeli : Max 31 znakow, bez spacji.\n");
            printf(" - pola         : List kolumn rozdzielona przecinkami (np. imie:STRING,wiek:INT).\n");
            printf(" - typy danych  : INT, FLOAT, STRING.\n");
            printf(" Przyklad       : mydb.db create-table uzytkownicy imie:STRING,wiek:INT,saldo:FLOAT\n");
        }
        else if (_stricmp(command, "insert-into") == 0) {
            printf(" Uzycie dla: insert-into\n");
            printf("   <db_file> insert-into <nazwa_tabeli> <wartosc1,wartosc2...> <wartosc3,wartosc4, ...> ...\n\n");
            printf(" - wartosci     : Wartości po przecinku w kolejnosci kolumn w schemacie.\n");
            printf(" Przyklad       : mydb.db insert-into uzytkownicy Jan,25,1500.50\n");
        }
        else if (_stricmp(command, "select-from") == 0) {
            printf(" Uzycie dla: select-from\n");
            printf("   <db_file> select-from <nazwa_tabeli>\n\n");
            printf(" Opis           : Wyswietla wszystkie aktywne rekordy z tabeli.\n");
            printf(" Przyklad       : mydb.db select-from uzytkownicy\n");
        }
        else if (_stricmp(command, "delete-from") == 0) {
            printf(" Uzycie dla: delete-from\n");
            printf("   <db_file> delete-from <nazwa_tabeli> <numer_wiersza1> <numer_wiersza2> ...\n\n");
            printf(" - numer_wiersza: Numer aktywnego wiersza (od 1).\n");
            printf(" Przyklad       : mydb.db delete-from uzytkownicy 2\n");
        }
        else if (_stricmp(command, "drop-table") == 0) {
            printf(" Uzycie dla: drop-table\n");
            printf("   <db_file> drop-table <nazwa_tabeli>\n\n");
            printf(" Opis           : Usuwa tabele wraz z rekordami z naglowka bazy.\n");
            printf(" Przyklad       : mydb.db drop-table uzytkownicy\n");
        }
        else {
            printf(" Nieznana komenda: '%s'\n\n", command);
            command = NULL;
        }
    }

    if (command == NULL) {
        printf(" Ogolna struktura wywolania:\n");
        printf("   <sciezka_do_db> <komenda> [argumenty...]\n");
        printf("-----------------------------------------------------------------------------\n");
        printf(" Dostepne komendy:\n");
        printf("   create-table <nazwa_tabeli> <pola>\n");
        printf("   insert-into  <nazwa_tabeli> <wartosci>\n");
        printf("   select-from  <nazwa_tabeli>\n");
        printf("   delete-from  <nazwa_tabeli> <numer_wiersza>\n");
        printf("   drop-table   <nazwa_tabeli>\n");
        printf("   help         [nazwa_komendy]\n");
    }

    printf("=============================================================================\n");
}