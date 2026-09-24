// RCOM 2026/2027
//
// Link layer protocol implementation

#include "link_layer.h"
#include "serial_port.h"

#include <stdio.h>
#include <unistd.h>

// MISC
#define _POSIX_SOURCE 1 // POSIX compliant source
#define BUF_SIZE 256

// Supervision Frame Stuff

#define FLAG_VALUE 0x7E
#define A_TX_CMD 0x03
#define A_RX_CMD 0x01
#define CTRL_SET 0x03
#define CTRL_UA 0x07

const unsigned char SET_FRAME[5] = {FLAG_VALUE, A_TX_CMD, CTRL_SET, A_TX_CMD ^ CTRL_SET, FLAG_VALUE};
const unsigned char UA_FRAME[5] = {FLAG_VALUE, A_TX_CMD, CTRL_UA, A_TX_CMD ^ CTRL_UA, FLAG_VALUE};

//

////////////////////////////////////////////////
// LLOPEN
////////////////////////////////////////////////
int llOpenTx(LinkLayer llParameters)
{
    /*     // ----------------------------------------------------
        // This example code shows how to open the serial port and send a string.
        // TODO: Adapt and extend this code according to the specifications of the project.
        // ----------------------------------------------------

    */
    volatile int STOP = FALSE;
    int receivedBytes = 0;
    int sentBytes = 0;
    int frameIndex = 0;
    unsigned char buf[5] = {0};

    if (openSerialPort(llParameters.serialPort, llParameters.baudRate) < 0)
    {
        perror("openSerialPort");
        return -1;
    }

    printf("Serial port %s opened\n", llParameters.serialPort);

    /*

    // Create string to send
    unsigned char buf[BUF_SIZE] = {0};

    for (int i = 0; i < BUF_SIZE; i++)
    {
        buf[i] = 'a' + i % 26;
    }

    // In non-canonical mode, '\n' does not end the writing.
    // Test this condition by placing a '\n' in the middle of the buffer.
    // The whole buffer must be sent even with the '\n'.
    buf[5] = '\n'; */

    int setBytes = writeBytesSerialPort(SET_FRAME, 5);
    if (setBytes == 5)
    {
        sentBytes += setBytes;
        printf("sent:");
        for (int i = 0; i < 5; i++)
        {
            printf(" 0x%02X", SET_FRAME[i]);
        }
        printf("\n");
    }
    else
    {
        printf("Could not send SET frame\n");
        closeSerialPort();
        return -1;
    }

    // Wait until all bytes have been written to the serial port
    sleep(1);
    printf("received:");
    while (STOP == FALSE)
    {
        // Read one byte from serial port.
        // NOTE: You must check how many bytes were actually read by reading the return value.
        // In this example, we assume that the byte is always read, which may not be true.
        unsigned char byte;
        int bytes = readByteSerialPort(&byte);
        if (bytes > 0)
        {
            if (frameIndex == 0 && byte != FLAG_VALUE)
            {
                receivedBytes++;
                continue;
            }
            buf[frameIndex] = byte;
            frameIndex++;
            receivedBytes++;
            printf(" 0x%02X", byte);
        }
        else if (bytes < 0)
        {
            perror("readByteSerialPort");
            break;
        }

        if (frameIndex == 5)
        {
            STOP = TRUE;
        }
    }
    printf("\n");
    int result = 0;
    if (buf[0] == FLAG_VALUE &&
        buf[1] == A_TX_CMD &&
        buf[2] == CTRL_UA &&
        buf[3] == (buf[1] ^ buf[2]) &&
        buf[4] == FLAG_VALUE)
    {
        printf("Connection Established\n");
    }
    else
    {
        printf("Connection failed: invalid UA frame\n");
        result = -1;
    }
    printf("Total bytes sent: %d\n", sentBytes);

    printf("Total bytes received: %d\n", receivedBytes);

    // Close serial port
    if (closeSerialPort() < 0)
    {
        perror("closeSerialPort");
        return -1;
    }

    printf("Serial port %s closed\n", llParameters.serialPort);

    return result;
}

int llOpenRx(LinkLayer llParameters)
{
    // ----------------------------------------------------
    // This example code shows how to open the serial port and receive a string.
    // TODO: Adapt and extend this code according to the specifications of the project.
    // ----------------------------------------------------

    if (openSerialPort(llParameters.serialPort, llParameters.baudRate) < 0)
    {
        perror("openSerialPort");
        return -1;
    }

    printf("Serial port %s opened\n", llParameters.serialPort);

    // Read from serial port until the 'z' char is received.

    // NOTE: This while() cycle is a simple example showing how to read from the serial port.
    // It must be changed in order to respect the specifications of the protocol indicated in the Lab guide.

    // TODO: Save the received bytes in a buffer array and print it at the end of the program.
    volatile int STOP = FALSE;
    int receivedBytes = 0;
    int sentBytes = 0;
    int frameIndex = 0;
    unsigned char buf[5] = {0};

    // Receive SET_FRAME

    printf("received:");
    while (STOP == FALSE)
    {
        // Read one byte from serial port.
        // NOTE: You must check how many bytes were actually read by reading the return value.
        // In this example, we assume that the byte is always read, which may not be true.
        unsigned char byte;
        int bytes = readByteSerialPort(&byte);
        if (bytes > 0)
        {
            if (frameIndex == 0 && byte != FLAG_VALUE)
            {
                receivedBytes++;
                continue;
            }
            buf[frameIndex] = byte;
            frameIndex++;
            receivedBytes++;
            printf(" 0x%02X", byte);
        }
        else if (bytes < 0)
        {
            perror("readByteSerialPort");
            break;
        }

        if (frameIndex == 5)
        {
            STOP = TRUE;
        }
    }
    printf("\n");

    // If SET_FRAME is valid send UA_FLAG

    int result = 0;
    if (buf[0] == FLAG_VALUE &&
        buf[1] == A_TX_CMD &&
        buf[2] == CTRL_SET &&
        buf[3] == (buf[1] ^ buf[2]) &&
        buf[4] == FLAG_VALUE)
    {
        int uaBytes = writeBytesSerialPort(UA_FRAME, 5);
        if (uaBytes == 5)
        {
            sentBytes += uaBytes;
            printf("sent:");
            for (int i = 0; i < 5; i++)
            {
                printf(" 0x%02X", UA_FRAME[i]);
            }
            printf("\n");

            printf("Connection Established\n");
        }
        else
        {
            printf("Connection failed: could not send UA frame\n");
            result = -1;
        }
    }
    else
    {
        printf("Connection failed: invalid SET frame\n");
        result = -1;
    }

    printf("Total bytes sent: %d\n", sentBytes);
    printf("Total bytes received: %d\n", receivedBytes);

    // Close serial port
    if (closeSerialPort() < 0)
    {
        perror("closeSerialPort");
        return -1;
    }

    printf("Serial port %s closed\n", llParameters.serialPort);

    return result;
}

////////////////////////////////////////////////
// LLSEND
////////////////////////////////////////////////
int llSend(const unsigned char *buf, int bufSize)
{
    // TODO: Implement this function

    return 0;
}

////////////////////////////////////////////////
// LLRECEIVE
////////////////////////////////////////////////
int llReceive(unsigned char *packet)
{
    // TODO: Implement this function

    return 0;
}

////////////////////////////////////////////////
// LLCLOSE
////////////////////////////////////////////////
int llCloseTx()
{
    // TODO: Implement this function

    return 0;
}

int llCloseRx()
{
    // TODO: Implement this function

    return 0;
}
