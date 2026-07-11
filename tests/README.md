# Standalone Hardware Tests

Hardware tests are separate CMake targets. They reuse the production board
bindings but do not start the FreeRTOS application or control algorithms.

## Digital Grayscale Sensor

Build and flash from VS Code with:

- `Build Line Sensor Test`
- `Flash Line Sensor Test (J-Link)`

The test samples CH1 through CH7 every 10 ms and reports every 100 ms through
UART0 at 115200 baud and SEGGER RTT. PB22 toggles every 500 ms as a heartbeat.

```text
$GRAY,ms,ch1,ch2,ch3,ch4,ch5,ch6,ch7,bits,changed,seen0,seen1,edge1,...,edge7
```

`seen0` and `seen1` are bit masks accumulated since reset. After each sensor
has crossed both black and white, both masks should read `0x7F`. The edge
counters expose disconnected, stuck or noisy channels. The electrical meaning
of 0 and 1 is intentionally not assumed until the physical black/white test is
complete.
