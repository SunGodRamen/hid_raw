#include "rawhid_thread.h"
#include "rawhid.h"

// Global reconnect thread variable
static pthread_t reconnect_thread;
static int thread_running = 0;
pthread_mutex_t device_mutex = PTHREAD_MUTEX_INITIALIZER;

// Thread function: Tries to reconnect to the device if the ping fails
static void* reconnect_thread_func(void* arg) {
    time_t lastPingTime = time(NULL) - PING_INTERVAL;

    while (thread_running) {
        time_t currentTime = time(NULL);

        if (difftime(currentTime, lastPingTime) >= PING_INTERVAL) {
            pthread_mutex_lock(&device_mutex);
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
                    thread_running = 0; // Stops the thread gracefully
                }
            }
            pthread_mutex_unlock(&device_mutex);
            lastPingTime = currentTime;
        }

        sleep(1);  // Add a short sleep to prevent the loop from eating up CPU
    }

    return NULL;
}

void start_reconnect_thread() {
    if (!thread_running) {
        thread_running = 1;
        pthread_create(&reconnect_thread, NULL, reconnect_thread_func, NULL);
    }
}

void stop_reconnect_thread() {
    if (thread_running) {
        thread_running = 0;  // Signal the thread to stop
        pthread_join(reconnect_thread, NULL);  // Wait for the thread to finish
        pthread_mutex_destroy(&device_mutex);
    }
}
