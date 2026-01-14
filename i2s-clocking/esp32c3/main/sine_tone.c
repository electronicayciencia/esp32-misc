#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/i2s_std.h"
#include "math.h"
#include "sdkconfig.h"
#include "esp_log.h"

// === Auto-select GPIOs based on target ===
#if CONFIG_IDF_TARGET_ESP32C3
    #define I2S_BCLK_GPIO  1
    #define I2S_WS_GPIO    2
    #define I2S_DOUT_GPIO  3
#elif CONFIG_IDF_TARGET_ESP32S3
    #define I2S_BCLK_GPIO  12
    #define I2S_WS_GPIO    11
    #define I2S_DOUT_GPIO  13
#elif CONFIG_IDF_TARGET_ESP32
    #define I2S_BCLK_GPIO  26
    #define I2S_WS_GPIO    25
    #define I2S_DOUT_GPIO  22
#else
    #error "Unsupported IDF target"
#endif

#define SAMPLE_RATE    44100
#define TONE_FREQ      1000
#define AMPLITUDE      20000  // < 32767 (16-bit signed)
#define BUFFER_SAMPLES 128

static int16_t s_sine_table[BUFFER_SAMPLES * 2]; // stereo: L, R

void generate_sine_table(void)
{
    for (int i = 0; i < BUFFER_SAMPLES; i++) {
        float sample = AMPLITUDE * sinf(2.0f * M_PI * TONE_FREQ * i / SAMPLE_RATE);
        int16_t val = (int16_t)sample;
        s_sine_table[i * 2]     = val; // Left
        s_sine_table[i * 2 + 1] = val; // Right
    }
}


void app_main(void)
{
    generate_sine_table();

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
#if CONFIG_IDF_TARGET_ESP32
           .clk_src = I2S_CLK_SRC_APLL,
#else
           .clk_src = I2S_CLK_SRC_DEFAULT, // ESP32-C3, ESP32-S3
#endif
           .mclk_multiple = I2S_MCLK_MULTIPLE_256, // optional, helps stability
        },
        .slot_cfg = I2S_STD_PHILIP_SLOT_DEFAULT_CONFIG(
            I2S_DATA_BIT_WIDTH_16BIT,
            I2S_SLOT_MODE_STEREO
        ),
        .gpio_cfg = {
            .bclk = I2S_BCLK_GPIO,
            .ws = I2S_WS_GPIO,
            .dout = I2S_DOUT_GPIO,
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

    // Stream sine wave forever
    while (1) {
        size_t bytes_written;
        i2s_channel_write(tx_handle, s_sine_table, sizeof(s_sine_table), &bytes_written, portMAX_DELAY);
        //ESP_LOG_BUFFER_HEXDUMP("Sent data: ", s_sine_table, bytes_written, ESP_LOG_INFO);
    }
}