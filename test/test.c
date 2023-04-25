#include <string.h>
#include "../cserial/cserial.h"
#include <windows.h>

int main()
{
    char portNames[100][CS_MAX_PORT_NAME_LEN];
    size_t nPort;
    CS_getPortNames(portNames, 100, &nPort);
    for(int i=0; i<nPort; i++)
    {
        printf("%s\n", portNames[i]);
    }

    CS_Handle* handle = CS_createHandle();
    CS_PortConfig config;
    if(CS_open(handle, "COM4", &config) == CS_ERROR_NO_ERROR)
        printf("port opened\n");

    CS_write(handle, "hello", 5);

    uint8_t buffer[100];
    for(int i=0; i<30; i++)
    {
        size_t receivedBytes = CS_read(handle, buffer, 100);
        if(receivedBytes > 0)
        {
            for(int j=0; j<receivedBytes; j++)
            {
                printf("%c", buffer[j]);
            }
            Sleep(10);
        }
    }
    CS_close(handle);
}
