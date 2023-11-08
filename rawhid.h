#ifndef RAWHID_H
#define RAWHID_H

#include <hidapi/hidapi.h>
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>

#include "constants.h"

extern hid_device* device;

void signal_handler(int signum);
hid_device* connect_to_device();
int ping_device(hid_device* device);
void listen_for_hid_data();

#endif
