#include "cserial.h"

typedef struct{

}Win32Handle;

void* CS_createHandle(){}
void CS_destroyHandle(void *handle){}
CS_Error CS_getPortNames(void *handle, char*** portNames, size_t* nPortNames){}
CS_Error CS_open(void* handle, const char* porName, const CS_PortConfig* portConfig){}
CS_Error CS_close(void* handle){}
void CS_write(void* handle, const uint8_t* data, size_t len){}
size_t CS_read(void* handle, uint8_t* buffer, size_t bufferLen){}
size_t CS_getBytesAvailable(void* handle){}