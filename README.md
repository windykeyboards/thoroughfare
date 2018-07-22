# Thoroughfare
Thoroughfare if a library for Serial to SD Card File Transfers on Arduino. It's a bit of a hack, ultimately it would be far better to have circuitry to directly mount the SD card for file transfer, but for now this will do.

## How does it work 
Thoroughfare works by parsing serial metadata, and writing a file to the corresponding directory on the SD Card. The following is an example of a simple file transfer:

1. Metadata sent: `STX` `newdir/hello.txt` `ETX`
2. Acknowledge sent back: `ACK`
3. Transmission, wrapped in start/end control characters 
    ```
    STX
    This is the file contents
    ETX
    ```
4. Acknowledge sent back: `ACK`
