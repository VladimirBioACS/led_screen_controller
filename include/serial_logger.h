/**
 _     _____ ____     __  __    _  _____ ____  _____  __
| |   | ____|  _ \   |  \/  |  / \|_   _|  _ \|_ _\ \/ /
| |   |  _| | | | |  | |\/| | / _ \ | | | |_) || | \  /
| |___| |___| |_| |  | |  | |/ ___ \| | |  _ < | | /  \
|_____|_____|____/___|_|  |_/_/   \_\_| |_| \_\___/_/\_\
                |_____|
****************************************************************************************************
*    @file           : serial_logger.h
*    @brief          : Serial logger setup and macros
****************************************************************************************************
*    @author     Volodymyr Noha
*
*    @description:
*    This is a header file contains Serial logger setup and macros
*
*    @section  HISTORY
*    v1.0  - First version
*
*    @section  LICENSE
*    None
****************************************************************************************************
*/

#ifndef SERIAL_LOGGER_H
#define SERIAL_LOGGER_H

#include <Arduino.h>
#include <stdio.h>

#define BAUDRATE  115200

/* Serial log setup */
#ifdef SERIAL_LOGGER_ENABLED

#define LOG_INFO(fmt, ...)              printf("[INFO]: "); \
                                        printf(fmt "\r\n", ##__VA_ARGS__)
#define LOG_ERROR(fmt, ...)             printf("[ERROR]: "); \
                                        printf(fmt "\r\n", ##__VA_ARGS__)
#define LOG_DEBUG(fmt, ...)             printf("[DEBUG]: "); \
                                        printf(fmt "\r\n", ##__VA_ARGS__)
FILE f_out;

int sput(char c, __attribute__((unused)) FILE* f) {
  return !Serial.write(c);
}

/**
 * @brief Setup serial logger
 */
void setup_serial_logger(void) {

    /* Setup serial port (UART1)*/
    Serial.begin(BAUDRATE);

    /* Redirect stdout to Serial port (UART1)*/
    fdev_setup_stream(&f_out, sput, NULL, _FDEV_SETUP_WRITE);
    stdout = &f_out;
}

#pragma message("Debug enabled")

#else
#define LOG_INFO(fmt, ...)
#define LOG_ERROR(fmt, ...)
#define LOG_DEBUG(fmt, ...)
void setup_serial_logger(void) {
      /* Setup serial port (UART1)*/
    Serial.begin(BAUDRATE);
}
#pragma message("Debug disabled")
#endif

#endif // SERIAL_LOGGER_H

