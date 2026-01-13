#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/i2s_std.h"

#define SAMPLE_RATE 44100
#define I2S_BCLK_GPIO 1
#define I2S_WS_GPIO   2
#define I2S_DIN_GPIO  3

void app_main(void)
{
    i2s_chan_handle_t tx_handle = NULL;
    i2s_chan_config_t chan_cfg = {
        .id = I2S_NUM_0,
        .role = I2S_ROLE_MASTER,
        .dma_desc_num = 2,
        .dma_frame_num = 128,
        .auto_clear = true,
    };
    ESP_ERROR_CHECK(i2s_new_channel(&chan_cfg, &tx_handle, NULL));

    i2s_std_config_t std_cfg = {
        .clk_cfg = {
            .sample_rate_hz = SAMPLE_RATE,
            .clk_src = I2S_CLK_SRC_DEFAULT,  // ← CORRECT for ESP32-C3
            .mclk_multiple = I2S_MCLK_MULTIPLE_256, // optional, helps stability
        },
        .slot_cfg = I2S_STD_PHILIP_SLOT_DEFAULT_CONFIG(
            I2S_DATA_BIT_WIDTH_16BIT,
            I2S_SLOT_MODE_STEREO
        ),
        .gpio_cfg = {
            .bclk = I2S_BCLK_GPIO,
            .ws = I2S_WS_GPIO,
            .dout = I2S_DIN_GPIO,
            .din = I2S_GPIO_UNUSED,
            .invert_flags = {
                .bclk_inv = false,
                .ws_inv = false,
            },
        },
    };

    // Fix slot width to match sample rate (16-bit)
    std_cfg.slot_cfg.slot_bit_width = I2S_SLOT_BIT_WIDTH_16BIT;

    ESP_ERROR_CHECK(i2s_channel_init_std_mode(tx_handle, &std_cfg));
    ESP_ERROR_CHECK(i2s_channel_enable(tx_handle));

    uint16_t zeros[128 * 2] = {0}; // stereo silent
    while (1) {
        size_t bytes_written;
        i2s_channel_write(tx_handle, zeros, sizeof(zeros), &bytes_written, portMAX_DELAY);
    }
}