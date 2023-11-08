
// Device usage info
#define VID 0x4444
#define PID 0x1111
#define RAW_USAGE_PAGE 0xfacc
#define RAW_USAGE_ID 0x41

#define BUFFER_SIZE 32      // Size of rawhid messages in bytes
#define READ_DELAY 2000     // Timeout for message reads in microseconds

#define PING_INTERVAL 5     // Frequency of ping in seconds
#define PING_RETRIES 12     // Attempt reconnect 12 times
#define PING_TIMEOUT 850    // Timeout for pong response in microseconds

