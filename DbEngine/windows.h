#ifndef WINDOWS_H
#define WINDOWS_H

#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <errno.h>

// Mapowanie funkcji porównywania łańcuchów (Windows -> Linux)
#define _stricmp strcasecmp
#define _strnicmp strncasecmp

// Emulacja bezpiecznego fopen_s
#define fopen_s(pFile, filename, mode) (((*(pFile) = fopen((filename), (mode))) == NULL) ? errno : 0)

// Emulacja funkcji bezpiecznych MSVC (strncpy_s, strtok_s, sprintf_s)
#define _TRUNCATE ((size_t)-1)
#define strncpy_s(dest, destsz, src, count) snprintf((dest), (destsz), "%s", (src))
#define strtok_s(str, delim, context) strtok_r((str), (delim), (context))
#define sprintf_s(buffer, sizeOfBuffer, format, ...) snprintf((buffer), (sizeOfBuffer), (format), __VA_ARGS__)

// Obejście konfliktu typów (const char* vs char*) w check_conditions bez edycji kodu
#define check_conditions check_conditions_fix

#endif

