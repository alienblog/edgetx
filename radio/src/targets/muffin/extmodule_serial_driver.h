#ifndef __EXTMOD_SERIAL_H
#define __EXTMOD_SERIAL_H

#include "hal/serial_driver.h"

// Return the serial driver to be used as external module
const etx_serial_driver_t* extmoduleGetSerialPort();

#endif  // !__EXTMOD_SERIAL_H