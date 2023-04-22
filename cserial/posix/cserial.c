/*
 * POSIX implementation of cserial
 * for MacOS, Linux
 */

#include "../common/cserial.h"
#include <dirent.h>
#include <fcntl.h>
#include <errno.h>
#include <termios.h>
#include <unistd.h>
#include <sys/ioctl.h>

typedef struct {
    CS_StringList portStringList;
    int serialPort;
    bool isOpen;
}PosixHandle;

bool isPortName(char* name)
{
    const char CS_PORT_NAME_PREFIX[][10] = {"ttyS", "ttyUSB", "tty."};
    int portNamePrefixLen = sizeof(CS_PORT_NAME_PREFIX) / sizeof(CS_PORT_NAME_PREFIX[0]);
    for(int i=0; i<portNamePrefixLen; i++)
    {
        if(strncmp(name, CS_PORT_NAME_PREFIX[i], strlen(CS_PORT_NAME_PREFIX[i])) == 0)
        {
            return true;
        }
    }
    return false;
}

void* CS_createHandle()
{
    PosixHandle* pHandle = malloc(sizeof(PosixHandle));
    CS_StringList_init(&pHandle->portStringList, 0);
    pHandle->isOpen=false;
    return pHandle;
}

void CS_destroyHandle(void *handle)
{
    PosixHandle *pHandle = handle;

    // close if open
    if(pHandle->isOpen)
        CS_close(handle);

    // free resources
    CS_StringList_free(&pHandle->portStringList);

    // free itself
    free(handle);
}

CS_Error CS_getPortNames(void *handle, char ***portNames, size_t* nPortNames)
{
    PosixHandle *pHandle = handle;

    DIR *dir;
    struct dirent *ent;

    dir = opendir("/dev/");
    if (dir == NULL)
        return CS_ERROR_NO_PORT_DETECTED;

    // get n port
    size_t nPort=0;
    while ((ent = readdir(dir)) != NULL)
    {
        if (isPortName(ent->d_name))
            nPort++;
    }

    closedir(dir);

    CS_StringList_free(&pHandle->portStringList);
    CS_StringList_init(&pHandle->portStringList, nPort);

    dir = opendir("/dev/");

    while ((ent = readdir(dir)) != NULL)
    {
        if (isPortName(ent->d_name))
            CS_StringList_push(&pHandle->portStringList, ent->d_name);
    }
    closedir(dir);

    *portNames = pHandle->portStringList.strings;
    *nPortNames = pHandle->portStringList.nString;
    return CS_ERROR_NO_ERROR;
}

CS_Error CS_open(void *handle, const char *porName, const CS_PortConfig *portConfig)
{
    PosixHandle* pHandle = handle;

    if(pHandle->isOpen)
        return CS_ERROR_PORT_ALREADY_OPEN;

    // make file name string
    char* fileStr = malloc(sizeof(char) * (strlen(porName) + strlen("/dev/") + 1));
    sprintf(fileStr, "/dev/%s", porName);

    // open port
    pHandle->serialPort = open(fileStr, O_RDWR  | O_NDELAY);

    // lock file descriptor
    fcntl(pHandle->serialPort, F_SETFL, 0);
    if(flock(pHandle->serialPort, LOCK_EX | LOCK_NB) == -1) {
        return CS_ERROR_PORT_ALREADY_BEING_USED;
    }

    // free file name string
    free(fileStr);

    // error handling
    if(pHandle->serialPort < 0)
    {
        switch (errno)
        {
        case 2:
            return CS_ERROR_NO_SUCH_PORT;
        case 13:
            return CS_ERROR_PERMISSION_DENIED;
        default:
        {
            printf("[cserial] %s (errno: %d)\n", strerror(errno), errno);
            return CS_ERROR_UNKNOWN_ERROR;
        }
        }
    }

    struct termios tty;

    if(tcgetattr(pHandle->serialPort, &tty) != 0) {
        printf("Error %i from tcgetattr: %s\n", errno, strerror(errno));
        return 1;
    }

    tty.c_cflag &= ~PARENB; // Clear parity bit, disabling parity (most common)
    tty.c_cflag &= ~CSTOPB; // Clear stop field, only one stop bit used in communication (most common)
    tty.c_cflag &= ~CSIZE; // Clear all bits that set the data size
    tty.c_cflag |= CS8; // 8 bits per byte (most common)
    tty.c_cflag &= ~CRTSCTS; // Disable RTS/CTS hardware flow control (most common)
    tty.c_cflag |= CREAD | CLOCAL; // Turn on READ & ignore ctrl lines (CLOCAL = 1)

    tty.c_lflag &= ~ICANON;
    tty.c_lflag &= ~ECHO; // Disable echo
    tty.c_lflag &= ~ECHOE; // Disable erasure
    tty.c_lflag &= ~ECHONL; // Disable new-line echo
    tty.c_lflag &= ~ISIG; // Disable interpretation of INTR, QUIT and SUSP
    tty.c_iflag &= ~(IXON | IXOFF | IXANY); // Turn off s/w flow ctrl
    tty.c_iflag &= ~(IGNBRK|BRKINT|PARMRK|ISTRIP|INLCR|IGNCR|ICRNL); // Disable any special handling of received bytes

    tty.c_oflag &= ~OPOST; // Prevent special interpretation of output bytes (e.g. newline chars)
    tty.c_oflag &= ~ONLCR; // Prevent conversion of newline to carriage return/line feed
    // tty.c_oflag &= ~OXTABS; // Prevent conversion of tabs to spaces (NOT PRESENT ON LINUX)
    // tty.c_oflag &= ~ONOEOT; // Prevent removal of C-d chars (0x004) in output (NOT PRESENT ON LINUX)

    tty.c_cc[VTIME] = 0;    // Wait for up to 1s (10 deciseconds), returning as soon as any data is received.
    tty.c_cc[VMIN] = 0;

    // Set in/out baud rate to be 9600
    cfsetispeed(&tty, B115200);
    cfsetospeed(&tty, B115200);

    if (tcsetattr(pHandle->serialPort, TCSANOW, &tty) != 0) {
        printf("Error %i from tcsetattr: %s\n", errno, strerror(errno));
        return 1;
    }

    pHandle->isOpen = true;
    return CS_ERROR_NO_ERROR;
}

CS_Error CS_close(void *handle)
{
    PosixHandle* pHandle = handle;

    if(!pHandle->isOpen || pHandle->serialPort < 0)
        return CS_ERROR_NO_PORT_TO_CLOSE;
    close(pHandle->serialPort);
    return CS_ERROR_NO_ERROR;
}

void CS_write(void *handle, const uint8_t *data, size_t len)
{
    PosixHandle* pHandle = handle;
    if(!pHandle->isOpen)
        return;
    size_t bytesWritten = write(pHandle->serialPort, data, len);
    printf("%zu bytes written\n", bytesWritten);
}

size_t CS_read(void *handle, uint8_t *buffer, size_t bufferLen)
{
    PosixHandle* pHandle = handle;
    if(!pHandle->isOpen)
        return 0;
    return read(pHandle->serialPort, buffer, bufferLen);
}

size_t CS_getBytesAvailable(void *handle)
{
    PosixHandle* pHandle = handle;
    int bytesAvailable;
    ioctl(pHandle->serialPort, FIONREAD, &bytesAvailable);
    return bytesAvailable;
}

