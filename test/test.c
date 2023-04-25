#include <string.h>
#include "../cserial/cserial.h"
#include <time.h>

int main()
{
    void* handle = CS_createHandle();

    char** portNames;
    size_t nPortNames;
    CS_getPortNames(handle, &portNames, &nPortNames);

    for(int i=0; i<nPortNames; i++)
    {
        printf("%s\n", portNames[i]);
    }
    CS_PortConfig config;
    if(CS_open(handle, portNames[2], &config) == CS_ERROR_NO_ERROR)
        printf("Port opened\n");
    else
    {
        printf("Port open failed\n");
        return 0;
    }

    printf("writing\n");
    unsigned char msg[] = { 'H', 'e', 'l', 'l', 'o', '\r' };
    CS_write(handle, msg, 6);
    printf("writing done\n");

    uint8_t read[100];

    printf("%zu bytes available\n", CS_getBytesAvailable(handle));
    size_t bytes = CS_read(handle, read, 100);
    for(int i=0; i<bytes; i++)
    {
        printf("%c", read[i]);
    }


    CS_close(handle);
    CS_destroyHandle(handle);
}
