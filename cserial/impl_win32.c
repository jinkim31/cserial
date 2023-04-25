#include "cserial.h"
#include <windows.h>

struct CS_Handle_{
    HANDLE serialPort;
    bool isOpen;
};

CS_Handle* CS_createHandle()
{
    CS_Handle* handle = malloc(sizeof(CS_Handle));
    handle->isOpen = false;
    return handle;
}

void CS_destroyHandle(CS_Handle *handle)
{
    free(handle);
}

CS_Error CS_getPortNames(char (*portNames)[CS_MAX_PORT_NAME_LEN], int nStrings, size_t *nPortNames)
{
    char portName[CS_MAX_PORT_NAME_LEN];
    char portDir[CS_MAX_PORT_NAME_LEN + 4];
    *nPortNames = 0;

    for(int i=0; i<256; i++)
    {
        // try to open port file
        sprintf_s(portName, CS_MAX_PORT_NAME_LEN, "COM%d", i);
        sprintf_s(portDir, CS_MAX_PORT_NAME_LEN, "\\\\.\\COM%d", i);
        HANDLE port = CreateFile(portDir,GENERIC_READ | GENERIC_WRITE,0,NULL,
            OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL ,NULL);

        // check file exists
        if(port == INVALID_HANDLE_VALUE)
            continue;

        // close handle
        CloseHandle(port);

        // check buffer size and save port name
        if(nStrings <= *nPortNames)
            return CS_ERROR_INSUFFICIENT_PORT_NAME_BUFFER;
        if(strcpy_s(portNames[(*nPortNames)++], CS_MAX_PORT_NAME_LEN, portName) != 0)
            return CS_ERROR_INSUFFICIENT_PORT_NAME_BUFFER;
    }
    return CS_ERROR_NO_ERROR;
}
CS_Error CS_open(CS_Handle* handle, const char* portName, const CS_PortConfig* portConfig)
{
    // check if port is already open
    if(handle->isOpen)
        return CS_ERROR_PORT_ALREADY_OPEN;

    // make file
    char portDir[CS_MAX_PORT_NAME_LEN + 4];
    sprintf_s(portDir, CS_MAX_PORT_NAME_LEN, "\\\\.\\%s", portName);
    handle->serialPort = CreateFile(portDir, GENERIC_READ | GENERIC_WRITE, 0, NULL,
        OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

    // check if port exists
    if(handle->serialPort == INVALID_HANDLE_VALUE)
        return CS_ERROR_NO_SUCH_PORT;

    // flush
    if(!FlushFileBuffers(handle->serialPort))
    {
        printf("[cserial] FlushFileBuffers() failed(code:%d).", GetLastError());
        CloseHandle(handle->serialPort);
        return CS_ERROR_UNKNOWN_ERROR;
    }

    // set timeouts(0)
    COMMTIMEOUTS timeouts = {0};
    timeouts.ReadIntervalTimeout = 0;
    timeouts.ReadTotalTimeoutConstant = 100;
    timeouts.ReadTotalTimeoutMultiplier = 0;
    timeouts.WriteTotalTimeoutConstant = 100;
    timeouts.WriteTotalTimeoutMultiplier = 0;
    if (!SetCommTimeouts(handle->serialPort, &timeouts))
    {
        printf("[cserial] SetCommTimeouts() failed(code:%d).", GetLastError());
        CloseHandle(handle->serialPort);
        return CS_ERROR_UNKNOWN_ERROR;
    }

    // Set the baud rate
    DCB state = {0};
    state.DCBlength = sizeof(DCB);
    state.BaudRate = portConfig->baudRate;
    state.ByteSize = 8;
    state.Parity = NOPARITY;
    state.StopBits = ONESTOPBIT;
    if (!SetCommState(handle->serialPort, &state))
    {
        printf("[cserial] SetCommState() failed(code:%d).", GetLastError());
        CloseHandle(handle->serialPort);
        return CS_ERROR_UNKNOWN_ERROR;
    }

    handle->isOpen = true;
    return CS_ERROR_NO_ERROR;
}

CS_Error CS_close(CS_Handle* handle)
{
    CloseHandle(handle->serialPort);
}

CS_Error CS_write(CS_Handle* handle, const uint8_t* data, size_t len)
{
    DWORD writtenBytes;
    if(!WriteFile(handle->serialPort, data, len, &writtenBytes, NULL) || len != writtenBytes)
    {
        printf("[cserial] WriteFile() failed(code:%d).", GetLastError());
        return CS_ERROR_UNKNOWN_ERROR;
    }
}

size_t CS_read(CS_Handle* handle, uint8_t* buffer, size_t bufferLen)
{
    DWORD receivedBytes;
    if(!ReadFile(handle->serialPort, buffer, bufferLen, &receivedBytes, NULL))
    {
        printf("[cserial] ReadFile() failed(code:%d).", GetLastError());
        return CS_ERROR_UNKNOWN_ERROR;
    }

    return receivedBytes;
}
size_t CS_getBytesAvailable(CS_Handle* handle);