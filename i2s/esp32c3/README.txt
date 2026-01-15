cd i2s_clock_test
idf.py set-target esp32c3
idf.py menuconfig  # Same as above
idf.py build flash monitor