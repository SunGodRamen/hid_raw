#ifndef RAWHID_THREAD_H
#define RAWHID_THREAD_H

#include <unistd.h>
#include <time.h>
#include "constants.h"

void start_reconnect_thread();
void stop_reconnect_thread();

#endif
