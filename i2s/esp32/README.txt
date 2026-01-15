cd i2s_clock_test
idf.py set-target esp32
idf.py menuconfig  # Verify: Component config → I2S → enable
idf.py build flash monitor