#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <systemd/sd-bus.h>

#ifndef GTEST_SAMPLE
#define GTEST_SAMPLE

extern static const sd_bus_vtable echo_vtable[];

#endif
