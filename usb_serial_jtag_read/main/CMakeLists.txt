idf_component_register(SRCS "main.c"
                    PRIV_REQUIRES 
                            esp_driver_usb_serial_jtag
                            vfs  ## to prevent logs from serial uart
                            esp_timer
                    INCLUDE_DIRS ".")
