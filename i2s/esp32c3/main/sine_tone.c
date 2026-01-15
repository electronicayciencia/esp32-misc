#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/i2s_std.h"
#include "sdkconfig.h"
#include <math.h>        // ← Required for cos()
#include <stdint.h>

// === Auto-select GPIOs ===
#if CONFIG_IDF_TARGET_ESP32C3
    #define I2S_BCLK_GPIO  1
    #define I2S_WS_GPIO    2
    #define I2S_DOUT_GPIO  3
#elif CONFIG_IDF_TARGET_ESP32
    #define I2S_BCLK_GPIO  21
    #define I2S_WS_GPIO    16   // LRCK
    #define I2S_DOUT_GPIO  17   // DIN
#else
    #error "Unsupported target"
#endif

#define SAMPLE_RATE    44100
#define TONE_FREQ      1000
#define AMPLITUDE      20000

// Precompute omega and k = 2*cos(omega)
#define OMEGA          (2.0 * M_PI * ((double)TONE_FREQ) / ((double)SAMPLE_RATE))
#define K              (2.0 * cos(OMEGA))   // This is ~1.9899 for 1kHz@44.1kHz

// Use Q15 format for k (but store as int32_t to avoid overflow in multiply)
#define K_Q15          ((int32_t)(K * 32768.0 + 0.5))

// Oscillator state (Q15 format)
static int32_t s_y1 = 0;  // y[n-1]
static int32_t s_y2 = 0;  // y[n-2]

void init_oscillator(void)
{
    // Start with: y[0] = 0, y[1] = sin(omega) * AMPLITUDE
    double first_val = sin(OMEGA) * AMPLITUDE;
    s_y2 = 0;
    s_y1 = (int32_t)first_val;
}

// Generate next sample using: y[n] = k * y[n-1] - y[n-2]
static inline int16_t recursive_sine(void)
{
    // Compute y[n] = (K_Q15 * s_y1) >> 15 - s_y2
    int64_t product = (int64_t)K_Q15 * s_y1;  // 32x32 → 64-bit
    int32_t yn = (int32_t)(product >> 15) - s_y2;

    // Optional: Amplitude stabilization (prevents drift)
    // Not strictly needed for short runs, but good practice
    if (yn > 32767) yn = 32767;
    else if (yn < -32768) yn = -32768;

    // Update state
    s_y2 = s_y1;
    s_y1 = yn;

    return (int16_t)yn;
}

void app_main(void)
{
    init_oscillator();

    i2s_chan_handle_t tx_handle = NULL;
    i2s_chan_config_t chan_cfg = {
        .id = I2S_NUM_0,
        .role = I2S_ROLE_MASTER,
        .dma_desc_num = 2,
        .dma_frame_num = 512,
        .auto_clear = true,
    };
    ESP_ERROR_CHECK(i2s_new_channel(&chan_cfg, &tx_handle, NULL));

    i2s_std_config_t std_cfg = {
        .clk_cfg = {
            .sample_rate_hz = SAMPLE_RATE,
#if CONFIG_IDF_TARGET_ESP32
            .clk_src = I2S_CLK_SRC_APLL,
#else
            .clk_src = I2S_CLK_SRC_DEFAULT,
#endif
            .mclk_multiple = I2S_MCLK_MULTIPLE_256,
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
        },
    };
    std_cfg.slot_cfg.slot_bit_width = I2S_SLOT_BIT_WIDTH_16BIT;
    ESP_ERROR_CHECK(i2s_channel_init_std_mode(tx_handle, &std_cfg));
    ESP_ERROR_CHECK(i2s_channel_enable(tx_handle));

    const uint32_t buffer_size = 512;
    int16_t *buffer = heap_caps_malloc(buffer_size * 2 * sizeof(int16_t), MALLOC_CAP_DMA);
    if (!buffer) return;

    while (1) {
        for (int i = 0; i < buffer_size; i++) {
            int16_t sample = recursive_sine();
            buffer[i * 2]     = sample; // Left
            buffer[i * 2 + 1] = sample; // Right
        }
        size_t written;
        i2s_channel_write(tx_handle, buffer, buffer_size * 2 * sizeof(int16_t), &written, portMAX_DELAY);
    }

    free(buffer);
}
