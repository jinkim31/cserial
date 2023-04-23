#include "cserial.h"

bool CS_StringList_init(CS_StringList *stringList, size_t capacity)
{
    stringList->strings = malloc(sizeof(char*) * capacity);
    if(stringList->strings == NULL)
        return false;
    stringList->capacity = capacity;
    stringList->nString = 0;
    return true;
}

bool CS_StringList_push(CS_StringList *stringList, char *string)
{
    if(stringList->capacity <= stringList->nString)
        return false;

    stringList->strings[stringList->nString] = malloc(sizeof(char) * (strlen(string) + 1));
    if(stringList->strings[stringList->nString] == NULL)
        return false;

    strcpy(stringList->strings[stringList->nString], string);
    stringList->nString++;
    return true;
}

void CS_StringList_free(CS_StringList *stringList)
{
    for(int i=0; i< stringList->nString; i++)
        free(stringList->strings[i]);
    free(stringList->strings);
}
