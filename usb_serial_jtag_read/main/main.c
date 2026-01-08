/*
 * SPDX-FileCopyrightText: 2023 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Unlicense OR CC0-1.0
 */

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/usb_serial_jtag.h"
#include "sdkconfig.h"
#include "esp_log.h"
#include "esp_check.h"

#define RING_BUF_SIZE (256)
#define ECHO_TASK_STACK_SIZE (4096)

// From drivers/usb_serial_jtag.c
// The hardware buffer max size is 64, both for RX and TX.
#define USB_SER_JTAG_ENDP_SIZE          (64)
#define USB_SER_JTAG_RX_MAX_SIZE        (USB_SER_JTAG_ENDP_SIZE)

#include "hal/usb_serial_jtag_ll.h"


static void ll_read_task(void *arg)
{
    printf("Starting direct USB Serial/JTAG read on ESP32-C3...\n");

    uint8_t *data = (uint8_t *) malloc(USB_SER_JTAG_RX_MAX_SIZE);
    if (data == NULL) {
        ESP_LOGE("llread", "no memory for data");
        return;
    }
    
    while (1) {
        int len = usb_serial_jtag_ll_read_rxfifo(data, USB_SER_JTAG_RX_MAX_SIZE);
        ESP_LOGI("llread", "Received %d bytes", len);
        if (len > 0) {
            ESP_LOG_BUFFER_HEXDUMP("Recv str: ", data, len, ESP_LOG_INFO);
        }
        // Non interrupt woken-up
        vTaskDelay(100 / portTICK_PERIOD_MS);
    }
}

static void driver_read_task(void *arg)
{
    // Configure USB SERIAL JTAG
    usb_serial_jtag_driver_config_t usb_serial_jtag_config = {
        .rx_buffer_size = RING_BUF_SIZE,
        .tx_buffer_size = RING_BUF_SIZE,
    };

    ESP_ERROR_CHECK(usb_serial_jtag_driver_install(&usb_serial_jtag_config));
    ESP_LOGI("drvread", "USB_SERIAL_JTAG init done");

    // Configure a temporary buffer for the incoming data
    uint8_t *data = (uint8_t *) malloc(RING_BUF_SIZE);
    if (data == NULL) {
        ESP_LOGE("drvread", "no memory for data");
        return;
    }

    while (1) {
        int len = usb_serial_jtag_read_bytes(data, RING_BUF_SIZE, 100 / portTICK_PERIOD_MS);
        ESP_LOGI("drvread", "Received %d bytes", len);
        if (len) {
            ESP_LOG_BUFFER_HEXDUMP("Recv str: ", data, len, ESP_LOG_INFO);
        }
    }
}

void app_main(void)
{
    xTaskCreate(driver_read_task, "driver", ECHO_TASK_STACK_SIZE, NULL, 10, NULL);
    //xTaskCreate(ll_read_task, "ll", ECHO_TASK_STACK_SIZE, NULL, 10, NULL);
}
