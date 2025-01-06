#ifndef OWN_LOGGER_H

#define OWN_LOGGER_H

#include<linux/kernel.h>


#define AVAILABLE_RPI_I2C_BUS           1U
#define SSH1306_OLED_ADR                0x3C
#define CLIENT_NAME                     "i2c_client_sh1106"

/* these are the fun are exported to the other module so other module can access this function */
void log_debug(const char *fmt, ...);
void log_info(const char *fmt, ...);
void log_warn(const char *fmt, ...);
void log_error(const char *fmt, ...);

#endif