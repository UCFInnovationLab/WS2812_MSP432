# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

Arduino library for driving WS2812 (NeoPixel) LEDs on the **MSP432P401R** via the hardware SPI peripheral (EUSCI_B0). Targets the **Energia** framework (Arduino-compatible for TI MSP432). Version 1.0.0.

## Building / Testing

There is no automated build or test system. Development workflow:

1. Open the sketch in the **Energia IDE** (Arduino-compatible IDE for TI LaunchPad).
2. Select board: **MSP-EXP432P401R LaunchPad**.
3. Compile and upload via Energia (Ctrl+U / Cmd+U).
4. The `examples/simple/simple.ino` sketch serves as the integration test — it sweeps red across 8 LEDs.

## Architecture

### SPI Bit-Encoding Scheme

The core insight: **one WS2812 data bit = one SPI byte**. With EUSCI_B0 running at 6 MHz (SMCLK 12 MHz ÷ 2), each SPI bit is 167 ns:

- `WS2812_HIGH_CODE` = `0xF8` — 5 HIGH bits (833 ns) + 3 LOW bits (500 ns) → WS2812 "1"
- `WS2812_LOW_CODE`  = `0xC0` — 2 HIGH bits (333 ns) + 6 LOW bits (1000 ns) → WS2812 "0"

This avoids bit-banging entirely; the shift register handles timing.

### Hardware Details

- **SPI peripheral**: EUSCI_B0, SIMO on **P1.6**
- **Pin mux**: MSP432 uses dual-register port select (SEL0 + SEL1). `SEL0=1, SEL1=0` → UCB0SIMO function.
- **Pixel buffer**: stored in GRB order (WS2812 wire format) even though the API accepts RGB.
- **Clock assumption**: SMCLK = 12 MHz (Energia default for MSP432P401R). If the clock is changed, `UCB0BRW` and timing codes must be recalculated.

### `begin()` Behavior — Two Versions

There are two versions of the implementation:

- **`WS2812_MSP432.cpp`** (current): Assigns P1.6 to EUSCI immediately, then delays 50 µs.
- **`WS2812_MSP432.cpp_fix`** (candidate fix): First drives P1.6 LOW as GPIO for 50 µs (guaranteeing a clean WS2812 reset pulse before the SIMO idle state is undefined during EUSCI bring-up), then hands the pin to EUSCI_B0. This is the more correct approach.

### `show()` Timing

Interrupts are disabled for the entire transmission to prevent inter-byte gaps that would corrupt WS2812 timing. After the last byte, the code waits for `UCBUSY` to clear (shift register fully drained) before re-enabling interrupts, then delays 50 µs for the WS2812 reset pulse.
