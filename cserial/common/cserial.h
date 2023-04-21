#ifndef CSERIAL_H
#define CSERIAL_H

#include <stdio.h>
#include "cserial_common.h"

typedef enum{
    CS_ERROR_NO_ERROR,
    CS_ERROR_NO_PORT_DETECTED,
    CS_ERROR_NO_SUCH_PORT,
    CS_ERROR_PERMISSION_DENIED,
    CS_ERROR_UNKNOWN_ERROR,
    CS_ERROR_NO_PORT_TO_CLOSE,
    CS_ERROR_PORT_ALREADY_OPEN,
    CS_ERROR_PORT_NOT_OPEN,
}CS_Error;

typedef struct{
    int baudRate;
}CS_PortConfig;

void* CS_createHandle();
void CS_destroyHandle(void *handle);
CS_Error CS_getPortNames(void *handle, char*** portNames, size_t* nPortNames);
CS_Error CS_open(void* handle, const char* porName, const CS_PortConfig* portConfig);
CS_Error CS_close(void* handle);
void CS_write(void* handle, const uint8_t* data, size_t len);
size_t CS_read(void* handle, uint8_t* buffer, size_t bufferLen);
size_t CS_getBytesAvailable(void* handle);
#endif