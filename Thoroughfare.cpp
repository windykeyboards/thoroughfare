#include <stdio.h>
#include <SD.h>
#include <Thoroughfare.h>

Thoroughfare::Thoroughfare(Stream & stream): stream(&stream)
{
    reset();
}

/*
 * Big ol' blocking function for reading from Serial, buffering, then writing to the SD
 */
void Thoroughfare::initiateParse() {
    FileMetadata* metadata = readMetadata();

    if (metadata == NULL) {
        return;
    }

    File file = SD.open(metadata->name, FILE_WRITE);

    // Empty current read buffer
    readState = WAIT;

    // Dump the current buffer
    for (int i = readBufferIndex; readBufferIndex < bytesAvailable; i++)
    {
        switch (readState) {
            case WAIT:
                if (readBuffer[i] == STX) {
                    // Begin reading filename
                    readState = READ;
                    dumpStartIndex = i + 1;
                    continue;
                }
                break;
            case READ:
                if (readBuffer[i] == ETX) {
                    // Terminate and write
                    readBuffer[i] = '\0';

                    file.write((char*) &readBuffer[dumpStartIndex]);
                    file.close();
                    stream->print(ACK);
                    reset();
                    return;
                }
                break; 
            case DONE:
                // no-op
                break;
        }
    }

    if (readState == WAIT) {
        // If this is the case, transmission failed. Close the file and move on
        file.close();
        reset();
        return;
    }

    // If we didn't encounter an ETX in the dump, we need to append a terminating character and write before continuing
    readBuffer[READBUFFERSIZE] = '\0';
    file.write((char*) &readBuffer[dumpStartIndex]);

    // Dump the remaining stream to disk
    while (stream->available())
	{
		bytesAvailable = min(stream->available(), READBUFFERSIZE);
		stream->readBytes(readBuffer, bytesAvailable);

        for (size_t byteNo = 0; byteNo < bytesAvailable; byteNo++)
		{
            if (readBuffer[byteNo] == STX) {
                // Terminate and close
                readBuffer[byteNo] = '\0';
                file.write((char*) readBuffer);
                file.close();
                stream->print(ACK);
                reset();
                return;
            } 
        }

        // Dump
        readBuffer[READBUFFERSIZE] = '\0';
        file.write((char*) readBuffer);
	}
}

FileMetadata* Thoroughfare::readMetadata() {
    while (stream->available())
	{
		size_t bytesAvailable = min(stream->available(), READBUFFERSIZE);
		stream->readBytes(readBuffer, bytesAvailable);

        for (size_t byteNo = 0; byteNo < bytesAvailable; byteNo++)
		{
            readBufferIndex = (uint8_t) byteNo;

            switch (filenameReadState) {
                case WAIT:
                    if (readBuffer[byteNo] == STX) {
                        // Begin reading filename
                        filenameReadState = READ;
                        continue;
                    }
                    break;
                case READ:
                      if (readBuffer[byteNo] == ETX) {
                        // Terminate and dispatch
                        filenameBuffer[filenameBufferIndex] = '\0';
                        filenameReadState = DONE;
                        stream->print(ACK);
                        returnMetadata = (FileMetadata){ (char*) filenameBuffer };
                        return &returnMetadata;
                    } else {
                        if (filenameBufferIndex < FILENAMEBUFFERSIZE) { 
                            // Accumulate
                            filenameBuffer[filenameBufferIndex] = readBuffer[byteNo];
                            filenameBufferIndex++;
                        } else {
                            // Command's longer than the buffer - reset
                            reset();
                        }
                    }
                case DONE:
                    // no-op: unreachable
                    break;
            }
        }
	}

    return NULL;
}

void Thoroughfare::reset() {
    readBufferIndex = 0;
    readState = WAIT;
    filenameBufferIndex = 0;
    filenameReadState = WAIT;
}


