#ifndef CSERIAL_UTIL
#define CSERIAL_UTIL

#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>

typedef struct
{
    char** strings;
    size_t capacity;
    size_t nString;
}CS_StringList;

bool CS_StringList_init(CS_StringList* stringList, size_t capacity);
bool CS_StringList_push(CS_StringList* stringList, char* string);
void CS_StringList_free(CS_StringList* stringList);

#endif