

#ifndef Thoroughfare_H
#define Thoroughfare_H

#include <Arduino.h>
#include <SPI.h>

#define ACK (char) 0x06
#define STX (char) 0x02
#define ETX (char) 0x03

#define READBUFFERSIZE 512
#define FILENAMEBUFFERSIZE 64

struct FileMetadata {
    char* name;
};

enum ReadState {
    WAIT,
    READ,
    DONE
};

class Thoroughfare 
{
    public:
        // Constructor
        Thoroughfare(Stream & stream);

        // Call to initiate the parse of a file
        void initiateParse();
    private:  
        Stream *stream;

        ReadState readState;
        uint8_t readBufferIndex;
        size_t bytesAvailable;
        char readBuffer[READBUFFERSIZE + 1];
        int dumpStartIndex;

        FileMetadata returnMetadata;
        ReadState filenameReadState;
        uint8_t filenameBufferIndex;
        char filenameBuffer[FILENAMEBUFFERSIZE];

        void reset();
        FileMetadata* readMetadata();

};

#endif