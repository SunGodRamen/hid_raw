#include <hidapi/hidapi.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>

// Device usage info
#define VID 0x4444
#define PID 0x1111
#define RAW_USAGE_PAGE 0xfacc
#define RAW_USAGE_ID 0x41

#define BUFFER_SIZE 32      // Size of rawhid messages in bytes
#define READ_DELAY 2000     // Timeout for message reads in microseconds

#define PING_INTERVAL 5     // Frequency of ping in seconds
#define PING_RETRIES 12     // Attempt reconnect 12 times
#define PING_TIMEOUT 500    // Timeout for pong response in microseconds

hid_device* device = NULL;


// Runs on keyboard interrupt
void signal_handler(int signum) {
    (void)signum;
    if (device) {
        hid_close(device);
        device = NULL;
    }
    printf("\nInterrupt received, closing the device handle.\n");
    exit(0);
}

// Locate device and usage page to open correct handle
hid_device* connect_to_device() {
    struct hid_device_info* devices = hid_enumerate(VID, PID);
    struct hid_device_info* device_info;
    hid_device* connected_device = NULL;

    for (device_info = devices; device_info != NULL; device_info = device_info->next) {
        if (device_info->usage_page == RAW_USAGE_PAGE && device_info->usage == RAW_USAGE_ID) {
            printf("Found device with VID: %04x, PID: %04x, USAGE_PAGE: %04x, USAGE: %04x\n",
               device_info->vendor_id, device_info->product_id, 
               device_info->usage_page, device_info->usage);
            connected_device = hid_open_path(device_info->path);
            if (connected_device) {
                printf("Successfully opened device.\n");
                hid_set_nonblocking(connected_device, 1);
                break;
            } else {
                fprintf(stderr, "Failed to open device on path: %s\n", device_info->path);
            }
        }
    }
    hid_free_enumeration(devices);
    return connected_device;
}

// Check that device is still connected and responding
int ping_device(hid_device* device) {
    unsigned char ping_request[32] = {0x00, 0x00, 0x00, 0x00};  // Account for the ignored first byte
    ping_request[1] = 0x01;
    if (hid_write(device, ping_request, 2) <= 0) {
        return 0;
    }

    unsigned char buffer[BUFFER_SIZE];
    int bytesRead = hid_read_timeout(device, buffer, sizeof(buffer), PING_TIMEOUT);
    if (bytesRead > 0 && buffer[0] == 0x02) {  // Adjusted to read the second byte
        return 1;
    }
    printf("failed pong\n");
    return 0;
}

int main() {
    signal(SIGINT, signal_handler);

    if (hid_init() != 0) {
        fprintf(stderr, "Failed to initialize HID API.\n");
        return -1;
    }

    printf("Searching for device with VID: %04x, PID: %04x\n", VID, PID);
    device = connect_to_device();

    if (!device) {
        fprintf(stderr, "Failed to open desired device.\n");
        return -1;
    }

    unsigned char read_data[BUFFER_SIZE];
    printf("Listening for raw HID data...\n");

    time_t lastPingTime = time(NULL) - PING_INTERVAL;

    while (1) {
        time_t currentTime = time(NULL);

        if (difftime(currentTime, lastPingTime) >= PING_INTERVAL) {
            if (!ping_device(device)) {
                fprintf(stderr, "Ping failed. Device might be disconnected.\n");
                int retries = 0;
                while (retries < PING_RETRIES) {
                    device = connect_to_device();
                    if (device) {
                        printf("Reconnected to the device.\n");
                        break;
                    }
                    retries++;
                    sleep(10);
                }
                if (retries == PING_RETRIES) {
                    fprintf(stderr, "Failed to reconnect. Exiting...\n");
                    break;
                }
            }
            lastPingTime = currentTime;
        }

        int bytesRead = hid_read_timeout(device, read_data, sizeof(read_data), READ_DELAY);
        if (bytesRead > 0) {
            unsigned short keycode = (read_data[1] << 8) | read_data[0];
            unsigned char pressed = read_data[2];
            printf("Keycode: %04x, Pressed: %02x\n", keycode, pressed);
        }
    }

    hid_close(device);
    device = NULL;
    hid_exit();

    return 0;
}
