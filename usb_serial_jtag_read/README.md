# USB Serial JTAG Read Bug

> TLDR: Skip to the bottom of the README to see the buggy driver code.

## Overview

Derived from ESP-IDF 5.5.2 USB serial JTAG echo example to demostrate **ESP-IDF bug in ESP Serial JTAG driver**.

It just reads bytes from USB CDC device and prints them over UART0.

## How to use example

### Hardware Required

The example can be run on development board that supports usb_serial_jtag, that is based on the Espressif SoC. The board shall be connected to a computer with a single USB cable for flashing and monitoring with UART port. 

A second external USB-Serial bridge must be connected to ESP32's UART0 in order to monitor the output.

### Configure the project

Disable the `ESP_CONSOLE_SECONDARY_USB_SERIAL_JTAG` using

```
idf.py menuconfig
```

- Component config → ESP System Settings → Channel for console output: **Default: UART0**
- Component config → ESP System Settings → Channel for console secondary output: **No secondary console**

By the way, VFS component should not be needed, but it actually is due to another known issue: 
[ESP32C3 cannot disable UART log over USB port](https://github.com/espressif/esp-idf/issues/15707).


### Build and Flash

In `main.c` you will see two lines to enable two tasks: 

```
//xTaskCreate(driver_read_task, "driver", ECHO_TASK_STACK_SIZE, NULL, 10, NULL);
xTaskCreate(ll_read_task, "ll", ECHO_TASK_STACK_SIZE, NULL, 10, NULL);
```

Select `driver_read_task` to read CDC bytes using the provided driver. Select `ll_read_task` to read bytes using low level calls and no driver.

Build the project and flash it to the board.

```
idf.py -p PORT flash
```

You will see a warning about the other function not being used, irrelevant here:

```
warning: 'driver_read_task' defined but not used [-Wunused-function]
```

## Demo output

Open two terminals:

- one for the USB serial port
- one for the external serial bridge connected to UART0 port. 

Don't use `idf.py monitor`. Use an actual serial terminal like PuTTY, TeraTerm, minicom, screen, etc.

Paste a big file (more than 1k, for example, must be bigger than `RING_BUF_SIZE` value) to USB CDC serial terminal. Expect to see nothing back in that window.

In the UART0 terminal you should see the whole file.

Example paste:

```
000 001 002 003 004 005 006 007 008 009 010 011 012 013 014 015 016 017 018 019 020 021 022 023 024 025 026 027 028 029 030 031 032 033 034 035 036 037 038 039 040 041 042 043 044 045 046 047 048 049 050 051 052 053 054 055 056 057 058 059 060 061 062 063 064 065 066 067 068 069 070 071 072 073 074 075 076 077 078 079 080 081 082 083 084 085 086 087 088 089 090 091 092 093 094 095 096 097 098 099 100 101 102 103 104 105 106 107 108 109 110 111 112 113 114 115 116 117 118 119 120 121 122 123 124 125 126 127 128 129 130 131 132 133 134 135 136 137 138 139 140 141 142 143 144 145 146 147 148 149 150 151 152 153 154 155 156 157 158 159 160 161 162 163 164 165 166 167 168 169 170 171 172 173 174 175 176 177 178 179 180 181 182 183 184 185 186 187 188 189 190 191 192 193 194 195 196 197 198 199 200 201 202 203 204 205 206 207 208 209 210 211 212 213 214 215 216 217 218 219 220 221 222 223 224 225 226 227 228 229 230 231 232 233 234 235 236 237 238 239 240 241 242 243 244 245 246 247 248 249 250 251 252 253 254 255 256 257 258 259
```

Output with `driver_read_task` (esp-idf 5.5.2 driver) is **truncated** after some bytes, depends on timing and `RING_BUF_SIZE` value:

```
[no other bytes]
I (67067) read: Received 0 bytes
I (67167) read: Received 0 bytes
I (67177) read: Received 12 bytes
I (67177) Recv str: : 0x3fc9291c   30 30 30 20 30 30 31 20  30 30 32 20              |000 001 002 |
I (67177) read: Received 244 bytes
I (67177) Recv str: : 0x3fc9291c   30 30 33 20 30 30 34 20  30 30 35 20 30 30 36 20  |003 004 005 006 |
I (67187) Recv str: : 0x3fc9292c   30 30 37 20 30 30 38 20  30 30 39 20 30 31 30 20  |007 008 009 010 |
I (67197) Recv str: : 0x3fc9293c   30 31 31 20 30 31 32 20  30 31 33 20 30 31 34 20  |011 012 013 014 |
I (67207) Recv str: : 0x3fc9294c   30 31 35 20 30 31 36 20  30 31 37 20 30 31 38 20  |015 016 017 018 |
I (67217) Recv str: : 0x3fc9295c   30 31 39 20 30 32 30 20  30 32 31 20 30 32 32 20  |019 020 021 022 |
I (67227) Recv str: : 0x3fc9296c   30 32 33 20 30 32 34 20  30 32 35 20 30 32 36 20  |023 024 025 026 |
I (67227) Recv str: : 0x3fc9297c   30 32 37 20 30 32 38 20  30 32 39 20 30 33 30 20  |027 028 029 030 |
I (67237) Recv str: : 0x3fc9298c   30 33 31 20 30 33 32 20  30 33 33 20 30 33 34 20  |031 032 033 034 |
I (67247) Recv str: : 0x3fc9299c   30 33 35 20 30 33 36 20  30 33 37 20 30 33 38 20  |035 036 037 038 |
I (67257) Recv str: : 0x3fc929ac   30 33 39 20 30 34 30 20  30 34 31 20 30 34 32 20  |039 040 041 042 |
I (67267) Recv str: : 0x3fc929bc   30 34 33 20 30 34 34 20  30 34 35 20 30 34 36 20  |043 044 045 046 |
I (67277) Recv str: : 0x3fc929cc   30 34 37 20 30 34 38 20  30 34 39 20 30 35 30 20  |047 048 049 050 |
I (67287) Recv str: : 0x3fc929dc   30 35 31 20 30 35 32 20  30 35 33 20 30 35 34 20  |051 052 053 054 |
I (67297) Recv str: : 0x3fc929ec   30 35 35 20 30 35 36 20  30 35 37 20 30 35 38 20  |055 056 057 058 |
I (67307) Recv str: : 0x3fc929fc   30 35 39 20 30 36 30 20  30 36 31 20 30 36 32 20  |059 060 061 062 |
I (67317) Recv str: : 0x3fc92a0c   30 36 33 20                                       |063 |
I (67317) read: Received 12 bytes
I (67327) Recv str: : 0x3fc9291c   30 39 36 20 30 39 37 20  30 39 38 20              |096 097 098 |
I (67337) read: Received 244 bytes
I (67337) Recv str: : 0x3fc9291c   30 39 39 20 31 30 30 20  31 30 31 20 31 30 32 20  |099 100 101 102 |
I (67347) Recv str: : 0x3fc9292c   31 30 33 20 31 30 34 20  31 30 35 20 31 30 36 20  |103 104 105 106 |
I (67357) Recv str: : 0x3fc9293c   31 30 37 20 31 30 38 20  31 30 39 20 31 31 30 20  |107 108 109 110 |
I (67367) Recv str: : 0x3fc9294c   31 31 31 20 31 31 32 20  31 31 33 20 31 31 34 20  |111 112 113 114 |
I (67377) Recv str: : 0x3fc9295c   31 31 35 20 31 31 36 20  31 31 37 20 31 31 38 20  |115 116 117 118 |
I (67377) Recv str: : 0x3fc9296c   31 31 39 20 31 32 30 20  31 32 31 20 31 32 32 20  |119 120 121 122 |
I (67387) Recv str: : 0x3fc9297c   31 32 33 20 31 32 34 20  31 32 35 20 31 32 36 20  |123 124 125 126 |
I (67397) Recv str: : 0x3fc9298c   31 32 37 20 31 32 38 20  31 32 39 20 31 33 30 20  |127 128 129 130 |
I (67407) Recv str: : 0x3fc9299c   31 33 31 20 31 33 32 20  31 33 33 20 31 33 34 20  |131 132 133 134 |
I (67417) Recv str: : 0x3fc929ac   31 33 35 20 31 33 36 20  31 33 37 20 31 33 38 20  |135 136 137 138 |
I (67427) Recv str: : 0x3fc929bc   31 33 39 20 31 34 30 20  31 34 31 20 31 34 32 20  |139 140 141 142 |
I (67437) Recv str: : 0x3fc929cc   31 34 33 20 31 34 34 20  31 34 35 20 31 34 36 20  |143 144 145 146 |
I (67447) Recv str: : 0x3fc929dc   31 34 37 20 31 34 38 20  31 34 39 20 31 35 30 20  |147 148 149 150 |
I (67457) Recv str: : 0x3fc929ec   31 35 31 20 31 35 32 20  31 35 33 20 31 35 34 20  |151 152 153 154 |
I (67467) Recv str: : 0x3fc929fc   31 35 35 20 31 35 36 20  31 35 37 20 31 35 38 20  |155 156 157 158 |
I (67477) Recv str: : 0x3fc92a0c   31 35 39 20                                       |159 |
I (67577) read: Received 0 bytes
I (67677) read: Received 0 bytes
I (67777) read: Received 0 bytes
[no more bytes]
```


Output with `ll_read_task` (no driver, low level reading). After a few seconds the full input is displayed:


```
I (16967) llread: Received 0 bytes
I (17067) llread: Received 0 bytes
I (17167) llread: Received 64 bytes
I (17167) Recv str: : 0x3fc912f8   30 30 30 20 30 30 31 20  30 30 32 20 30 30 33 20  |000 001 002 003 |
I (17167) Recv str: : 0x3fc91308   30 30 34 20 30 30 35 20  30 30 36 20 30 30 37 20  |004 005 006 007 |
I (17177) Recv str: : 0x3fc91318   30 30 38 20 30 30 39 20  30 31 30 20 30 31 31 20  |008 009 010 011 |
I (17177) Recv str: : 0x3fc91328   30 31 32 20 30 31 33 20  30 31 34 20 30 31 35 20  |012 013 014 015 |
I (17287) llread: Received 64 bytes
I (17287) Recv str: : 0x3fc912f8   30 31 36 20 30 31 37 20  30 31 38 20 30 31 39 20  |016 017 018 019 |
I (17287) Recv str: : 0x3fc91308   30 32 30 20 30 32 31 20  30 32 32 20 30 32 33 20  |020 021 022 023 |
I (17297) Recv str: : 0x3fc91318   30 32 34 20 30 32 35 20  30 32 36 20 30 32 37 20  |024 025 026 027 |
I (17297) Recv str: : 0x3fc91328   30 32 38 20 30 32 39 20  30 33 30 20 30 33 31 20  |028 029 030 031 |
I (17407) llread: Received 64 bytes
I (17407) Recv str: : 0x3fc912f8   30 33 32 20 30 33 33 20  30 33 34 20 30 33 35 20  |032 033 034 035 |
I (17407) Recv str: : 0x3fc91308   30 33 36 20 30 33 37 20  30 33 38 20 30 33 39 20  |036 037 038 039 |
I (17417) Recv str: : 0x3fc91318   30 34 30 20 30 34 31 20  30 34 32 20 30 34 33 20  |040 041 042 043 |
I (17417) Recv str: : 0x3fc91328   30 34 34 20 30 34 35 20  30 34 36 20 30 34 37 20  |044 045 046 047 |
I (17527) llread: Received 64 bytes
I (17527) Recv str: : 0x3fc912f8   30 34 38 20 30 34 39 20  30 35 30 20 30 35 31 20  |048 049 050 051 |
I (17527) Recv str: : 0x3fc91308   30 35 32 20 30 35 33 20  30 35 34 20 30 35 35 20  |052 053 054 055 |
I (17537) Recv str: : 0x3fc91318   30 35 36 20 30 35 37 20  30 35 38 20 30 35 39 20  |056 057 058 059 |
I (17537) Recv str: : 0x3fc91328   30 36 30 20 30 36 31 20  30 36 32 20 30 36 33 20  |060 061 062 063 |
I (17647) llread: Received 64 bytes
I (17647) Recv str: : 0x3fc912f8   30 36 34 20 30 36 35 20  30 36 36 20 30 36 37 20  |064 065 066 067 |
I (17647) Recv str: : 0x3fc91308   30 36 38 20 30 36 39 20  30 37 30 20 30 37 31 20  |068 069 070 071 |
I (17657) Recv str: : 0x3fc91318   30 37 32 20 30 37 33 20  30 37 34 20 30 37 35 20  |072 073 074 075 |
I (17657) Recv str: : 0x3fc91328   30 37 36 20 30 37 37 20  30 37 38 20 30 37 39 20  |076 077 078 079 |
I (17767) llread: Received 64 bytes
I (17767) Recv str: : 0x3fc912f8   30 38 30 20 30 38 31 20  30 38 32 20 30 38 33 20  |080 081 082 083 |
I (17767) Recv str: : 0x3fc91308   30 38 34 20 30 38 35 20  30 38 36 20 30 38 37 20  |084 085 086 087 |
I (17777) Recv str: : 0x3fc91318   30 38 38 20 30 38 39 20  30 39 30 20 30 39 31 20  |088 089 090 091 |
I (17777) Recv str: : 0x3fc91328   30 39 32 20 30 39 33 20  30 39 34 20 30 39 35 20  |092 093 094 095 |
I (17887) llread: Received 64 bytes
I (17887) Recv str: : 0x3fc912f8   30 39 36 20 30 39 37 20  30 39 38 20 30 39 39 20  |096 097 098 099 |
I (17887) Recv str: : 0x3fc91308   31 30 30 20 31 30 31 20  31 30 32 20 31 30 33 20  |100 101 102 103 |
I (17897) Recv str: : 0x3fc91318   31 30 34 20 31 30 35 20  31 30 36 20 31 30 37 20  |104 105 106 107 |
I (17897) Recv str: : 0x3fc91328   31 30 38 20 31 30 39 20  31 31 30 20 31 31 31 20  |108 109 110 111 |
I (18007) llread: Received 64 bytes
I (18007) Recv str: : 0x3fc912f8   31 31 32 20 31 31 33 20  31 31 34 20 31 31 35 20  |112 113 114 115 |
I (18007) Recv str: : 0x3fc91308   31 31 36 20 31 31 37 20  31 31 38 20 31 31 39 20  |116 117 118 119 |
I (18017) Recv str: : 0x3fc91318   31 32 30 20 31 32 31 20  31 32 32 20 31 32 33 20  |120 121 122 123 |
I (18017) Recv str: : 0x3fc91328   31 32 34 20 31 32 35 20  31 32 36 20 31 32 37 20  |124 125 126 127 |
I (18127) llread: Received 64 bytes
I (18127) Recv str: : 0x3fc912f8   31 32 38 20 31 32 39 20  31 33 30 20 31 33 31 20  |128 129 130 131 |
I (18127) Recv str: : 0x3fc91308   31 33 32 20 31 33 33 20  31 33 34 20 31 33 35 20  |132 133 134 135 |
I (18137) Recv str: : 0x3fc91318   31 33 36 20 31 33 37 20  31 33 38 20 31 33 39 20  |136 137 138 139 |
I (18137) Recv str: : 0x3fc91328   31 34 30 20 31 34 31 20  31 34 32 20 31 34 33 20  |140 141 142 143 |
I (18247) llread: Received 64 bytes
I (18247) Recv str: : 0x3fc912f8   31 34 34 20 31 34 35 20  31 34 36 20 31 34 37 20  |144 145 146 147 |
I (18247) Recv str: : 0x3fc91308   31 34 38 20 31 34 39 20  31 35 30 20 31 35 31 20  |148 149 150 151 |
I (18257) Recv str: : 0x3fc91318   31 35 32 20 31 35 33 20  31 35 34 20 31 35 35 20  |152 153 154 155 |
I (18257) Recv str: : 0x3fc91328   31 35 36 20 31 35 37 20  31 35 38 20 31 35 39 20  |156 157 158 159 |
I (18367) llread: Received 64 bytes
I (18367) Recv str: : 0x3fc912f8   31 36 30 20 31 36 31 20  31 36 32 20 31 36 33 20  |160 161 162 163 |
I (18367) Recv str: : 0x3fc91308   31 36 34 20 31 36 35 20  31 36 36 20 31 36 37 20  |164 165 166 167 |
I (18377) Recv str: : 0x3fc91318   31 36 38 20 31 36 39 20  31 37 30 20 31 37 31 20  |168 169 170 171 |
I (18377) Recv str: : 0x3fc91328   31 37 32 20 31 37 33 20  31 37 34 20 31 37 35 20  |172 173 174 175 |
I (18487) llread: Received 64 bytes
I (18487) Recv str: : 0x3fc912f8   31 37 36 20 31 37 37 20  31 37 38 20 31 37 39 20  |176 177 178 179 |
I (18487) Recv str: : 0x3fc91308   31 38 30 20 31 38 31 20  31 38 32 20 31 38 33 20  |180 181 182 183 |
I (18497) Recv str: : 0x3fc91318   31 38 34 20 31 38 35 20  31 38 36 20 31 38 37 20  |184 185 186 187 |
I (18497) Recv str: : 0x3fc91328   31 38 38 20 31 38 39 20  31 39 30 20 31 39 31 20  |188 189 190 191 |
I (18607) llread: Received 64 bytes
I (18607) Recv str: : 0x3fc912f8   31 39 32 20 31 39 33 20  31 39 34 20 31 39 35 20  |192 193 194 195 |
I (18607) Recv str: : 0x3fc91308   31 39 36 20 31 39 37 20  31 39 38 20 31 39 39 20  |196 197 198 199 |
I (18617) Recv str: : 0x3fc91318   32 30 30 20 32 30 31 20  32 30 32 20 32 30 33 20  |200 201 202 203 |
I (18617) Recv str: : 0x3fc91328   32 30 34 20 32 30 35 20  32 30 36 20 32 30 37 20  |204 205 206 207 |
I (18727) llread: Received 64 bytes
I (18727) Recv str: : 0x3fc912f8   32 30 38 20 32 30 39 20  32 31 30 20 32 31 31 20  |208 209 210 211 |
I (18727) Recv str: : 0x3fc91308   32 31 32 20 32 31 33 20  32 31 34 20 32 31 35 20  |212 213 214 215 |
I (18737) Recv str: : 0x3fc91318   32 31 36 20 32 31 37 20  32 31 38 20 32 31 39 20  |216 217 218 219 |
I (18737) Recv str: : 0x3fc91328   32 32 30 20 32 32 31 20  32 32 32 20 32 32 33 20  |220 221 222 223 |
I (18847) llread: Received 64 bytes
I (18847) Recv str: : 0x3fc912f8   32 32 34 20 32 32 35 20  32 32 36 20 32 32 37 20  |224 225 226 227 |
I (18847) Recv str: : 0x3fc91308   32 32 38 20 32 32 39 20  32 33 30 20 32 33 31 20  |228 229 230 231 |
I (18857) Recv str: : 0x3fc91318   32 33 32 20 32 33 33 20  32 33 34 20 32 33 35 20  |232 233 234 235 |
I (18857) Recv str: : 0x3fc91328   32 33 36 20 32 33 37 20  32 33 38 20 32 33 39 20  |236 237 238 239 |
I (18967) llread: Received 64 bytes
I (18967) Recv str: : 0x3fc912f8   32 34 30 20 32 34 31 20  32 34 32 20 32 34 33 20  |240 241 242 243 |
I (18967) Recv str: : 0x3fc91308   32 34 34 20 32 34 35 20  32 34 36 20 32 34 37 20  |244 245 246 247 |
I (18977) Recv str: : 0x3fc91318   32 34 38 20 32 34 39 20  32 35 30 20 32 35 31 20  |248 249 250 251 |
I (18977) Recv str: : 0x3fc91328   32 35 32 20 32 35 33 20  32 35 34 20 32 35 35 20  |252 253 254 255 |
I (19087) llread: Received 15 bytes
I (19087) Recv str: : 0x3fc912f8   32 35 36 20 32 35 37 20  32 35 38 20 32 35 39     |256 257 258 259|
I (19187) llread: Received 0 bytes
I (19287) llread: Received 0 bytes
```


## The bug

The Host send data to the device in 64 bytes packets. 

If the hardware FIFO in ESP32 is free, it will send ACK to that packet. If the hardware FIFO is full, ESP32 will send NACK, and the host will retry. This acts as a USB CDC flow control.

In `esp-idf-v5.5.2/components/esp_driver_usb_serial_jtag/src/usb_serial_jtag.c` there is a ISR with this code:

```c
if (usbjtag_intr_status & USB_SERIAL_JTAG_INTR_SERIAL_OUT_RECV_PKT) {
    // Acknowledge interrupt
    usb_serial_jtag_ll_clr_intsts_mask(USB_SERIAL_JTAG_INTR_SERIAL_OUT_RECV_PKT);
    // Read RX FIFO and send available data to ringbuffer.
    uint8_t buf[USB_SER_JTAG_RX_MAX_SIZE];
    uint32_t rx_fifo_len = usb_serial_jtag_ll_read_rxfifo(buf, USB_SER_JTAG_RX_MAX_SIZE);
    xRingbufferSendFromISR(p_usb_serial_jtag_obj->rx_ring_buf, buf, rx_fifo_len, &xTaskWoken);

    if (p_usb_serial_jtag_obj->usj_select_notif_callback) {
        p_usb_serial_jtag_obj->usj_select_notif_callback(USJ_SELECT_READ_NOTIF, &xTaskWoken);
    }
}
```

When data arrives it triggers the interrupt and this ISR is called.

`usb_serial_jtag_ll_read_rxfifo` consumes the ESP32's FIFO (the chip ACKs to the host) and return the bytes. `xRingbufferSendFromISR` adds them to the internal driver ring buffer `RING_BUF_SIZE` is 256 bytes in our example.

When the buffer is full, `xRingbufferSendFromISR` cannot append the data, but since `usb_serial_jtag_ll_read_rxfifo` has cleared the FIFO, ESP32 ACKs that 64 bytes to the Host. The Host then send the following 64 bytes frame, they are read, ack, and discarded. And so on.

`usb_serial_jtag_ll_read_rxfifo` should not be called unless there are 64 bytes free in the buffer. 

That is the reason low level task works but driver miss bytes.

