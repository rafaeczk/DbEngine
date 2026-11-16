#include <stdio.h>
#include <string.h>
#include "print_help.h"

void print_help(const char* command)
{
	printf("\n=================================== POMOC ===================================\n");

	if (command != NULL)
	{
		if (_stricmp(command, "create-table") == 0)
		{
			printf(" Uzycie dla: create-table\n");
			printf("   <db_file> create-table <nazwa_tabeli> <pole1:typ,pole2:typ...>\n\n");
			printf(" - nazwa_tabeli : Max 31 znakow, bez spacji.\n");
			printf(" - pola         : Lista kolumn rozdzielona przecinkami (np. id:INT_AUTO_INC,imie:STRING).\n");
			printf(" - typy danych  : INT, INT_AUTO_INC, FLOAT, STRING.\n");
			printf("                  (INT_AUTO_INC: automatycznie inkrementowany unikalny identyfikator)\n");
			printf(" Przyklad       : mydb.db create-table uzytkownicy id:INT_AUTO_INC,imie:STRING,wiek:INT,saldo:FLOAT\n");
		}
		else if (_stricmp(command, "insert-into") == 0)
		{
			printf(" Uzycie dla: insert-into\n");
			printf("   <db_file> insert-into <nazwa_tabeli> <wartosc1,wartosc2...> [wartosc1,wartosc2...] ...\n\n");
			printf(" - wartosci     : Wartosci po przecinku w kolejnosci kolumn w schemacie.\n");
			printf("                  Dla kolumny INT_AUTO_INC uzyj '_' aby wygenerowac kolejny identyfikator.\n");
			printf(" Przyklad       : mydb.db insert-into uzytkownicy _,Jan,25,1500.50 _,Anna,30,2200.00\n");
		}
		else if (_stricmp(command, "select") == 0)
		{
			printf(" Uzycie dla: select\n");
			printf("   <db_file> select <pole1,pole2,...|*> from <nazwa_tabeli> [where:pole1<op>wartosc1,...]\n\n");
			printf(" - pola         : Nazwy kolumn rozdzielone przecinkami lub '*' dla wszystkich kolumn.\n");
			printf("   przed nazwa pola mozna wywolac funkcje, obliczana na biezaca z uwzglednieniem kolejnych rekordow.\n");
			printf("   Dostepne funkcje: max, min, avg, sum.\n");
			printf(" - operatory    : =, !, >, <, ~ (np. where:wiek>18,imie=Jan,produkty~suszarka)\n");
			printf(" Opis           : Wyswietla aktywne rekordy z tabeli spelniajace warunki.\n");
			printf(" Przyklad 1     : mydb.db select * from uzytkownicy\n");
			printf(" Przyklad 2     : mydb.db select imie,saldo,avg:saldo from uzytkownicy where:wiek>18\n");
		}
		else if (_stricmp(command, "update") == 0)
		{
			printf(" Uzycie dla: update\n");
			printf("   <db_file> update <wartosci...> in <nazwa_tabeli> <where:warunki>\n\n");
			printf(" - wartosci     : Wartosci po przecinku w kolejnosci kolumn w schemacie.\n");
			printf("                  Uzyj '_' aby zachowac dotychczasowa wartosc w polu.\n");
			printf("                  (Dla kolumny INT_AUTO_INC obowiazkowo podaj '_')\n");
			printf(" - warunki      : Warunki filtrowania z operatorami =, !, >, <, ~ (np. where:id=2).\n");
			printf(" Opis           : Modyfikuje rekordy spelniajace warunki w danej tabeli.\n");
			printf(" Przyklad       : mydb.db update _,Jan,Nowak,1995 in uzytkownicy where:id=1\n");
		}
		else if (_stricmp(command, "delete-from") == 0)
		{
			printf(" Uzycie dla: delete-from\n");
			printf("   <db_file> delete-from <nazwa_tabeli> <where:warunki>\n\n");
			printf(" - warunki      : Warunki filtrowania z operatorami =, !, >, < (np. where:id=2).\n");
			printf(" Opis           : Oznacza pasujace rekordy jako usuniete (soft delete).\n");
			printf(" Przyklad       : mydb.db delete-from uzytkownicy where:id=2\n");
		}
		else if (_stricmp(command, "drop-table") == 0)
		{
			printf(" Uzycie dla: drop-table\n");
			printf("   <db_file> drop-table <nazwa_tabeli>\n\n");
			printf(" Opis           : Usuwa tabele wraz z rekordami z naglowka bazy.\n");
			printf(" Przyklad       : mydb.db drop-table uzytkownicy\n");
		}
		else if (_stricmp(command, "show-tables") == 0)
		{
			printf(" Uzycie dla: show-tables\n");
			printf("   <db_file> show-tables\n\n");
			printf(" Opis           : Wyswietla liste wszystkich tabel w bazie wraz ze schematem kolumn i liczba rekordow.\n");
			printf(" Przyklad       : mydb.db show-tables\n");
		}
		else if (_stricmp(command, "vacuum") == 0)
		{
			printf(" Uzycie dla: vacuum\n");
			printf("   <db_file> vacuum\n\n");
			printf(" Opis           : Kompaktuje baze danych, trwale usuwajac skasowane rekordy i zmniejszajac rozmiar pliku.\n");
			printf(" Przyklad       : mydb.db vacuum\n");
		}
		else
		{
			printf(" Nieznana komenda: '%s'\n\n", command);
			command = NULL;
		}
	}

	if (command == NULL)
	{
		printf(" Ogolna struktura wywolania:\n");
		printf("   <sciezka_do_db> <komenda> [argumenty...]\n");
		printf("-----------------------------------------------------------------------------\n");
		printf(" Dostepne komendy:\n");
		printf("   create-table <nazwa_tabeli> <pola>\n");
		printf("   insert-into  <nazwa_tabeli> <wartosci...>\n");
		printf("   select       <pola|*> from <nazwa_tabeli> [where:...]\n");
		printf("   update       <wartosci...> in <nazwa_tabeli> <where:...>\n");
		printf("   delete-from  <nazwa_tabeli> <where:...>\n");
		printf("   drop-table   <nazwa_tabeli>\n");
		printf("   show-tables\n");
		printf("   vacuum\n");
		printf("   help         [nazwa_komendy]\n");
	}

	printf("=============================================================================\n\n");
}
