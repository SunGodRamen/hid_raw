#include "rawhid.h"

// Handle keyboard interrupt (SIGINT)
void signal_handler(int signum) {
    (void)signum;  // to avoid unused variable warning
    if (device) {
        hid_close(device);
        device = NULL;
    }
    printf("\nInterrupt received, closing the device handle.\n");
    exit(0);
}

// Connect to the device given its VID, PID, and USAGE_PAGE/USAGE_ID
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

// Ping the device to ensure it's still connected and responding
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
    printf("Failed pong\n");
    return 0;
}

// Continuously listen for incoming HID data
void listen_for_hid_data() {
    unsigned char read_data[BUFFER_SIZE];
    printf("Listening for raw HID data...\n");

    while (1) {
        int bytesRead = hid_read_timeout(device, read_data, sizeof(read_data), READ_DELAY);
        if (bytesRead > 0) {
            unsigned short keycode = (read_data[1] << 8) | read_data[0];
            unsigned char pressed = read_data[2];
            printf("Keycode: %04x, Pressed: %02x\n", keycode, pressed);
        }
    }
}
