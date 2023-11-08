//gcc main.c rawhid_thread.c rawhid.c -lhidapi-hidraw -lpthread -o rawhid_app

#include <signal.h>
#include "rawhid.h"
#include "rawhid_thread.h"
#include "constants.h"

hid_device* device = NULL;

int main() {
    // Setup signal handling
    signal(SIGINT, signal_handler);

    // Initialize HID
    if (hid_init() != 0) {
        fprintf(stderr, "Failed to initialize HID API.\n");
        return -1;
    }

    // Connect to the device
    device = connect_to_device();
    if (!device) {
        fprintf(stderr, "Failed to open desired device.\n");
        return -1;
    }

    // Start the reconnecting thread
    start_reconnect_thread();

    // Listen for raw HID data
    listen_for_hid_data();

    // Cleanup
    stop_reconnect_thread();
    hid_exit();

    return 0;
}
