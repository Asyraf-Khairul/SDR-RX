# File audit and rename map

Audited on 19 October 2026 against the project's CMake source lists, local
includes, generated PIO headers, and the existing RP2040 build dependencies.
This audit covers the project's files, not unused internals of the upstream
`external/u8g2` submodule or generated SDK/build trees.

## Files removed

| File | Finding |
| --- | --- |
| `binaries-rp2040-pico.zip` | Generated firmware distribution; not a build input. A fresh UF2 remains under `build/`. |
| `binaries-rp2350-pico2.zip` | Generated Pico 2 firmware distribution; not a build input. |
| `binaries-rp2350-riscv-pico2.zip` | Generated Pico 2 RISC-V firmware distribution; not a build input. |
| `cmake_build` | Unreferenced helper that only exported the original author's lowercase SDK path. |
| `gfxfont.h` | Unreferenced Adafruit font type definitions; absent from local includes and RP2040 compiler dependencies. |
| `logo.h` | Included by the old UI header, but its only bitmap, `crystal`, was never referenced. Removed the unused include as well. |

Every project `.c`, `.cpp`, and `.pio` file was listed in CMake. Among root
headers, only `gfxfont.h` and `tusb_config.h` lacked a direct local include;
`tusb_config.h` is required by TinyUSB and must retain its SDK-defined name.

## Optional files retained

| Current file(s) | Use |
| --- | --- |
| `receiver_clocks_rp2350.cpp` | Pico 2 ARM and RISC-V builds. |
| `button_tuning_encoder.cpp`, `button_tuning_encoder.h` | Alternative controls selected with `BUTTON_ENCODER`. |
| `battery_monitor.cpp` | Separate `battery_check` / `battery_check_pico2` firmware targets. |
| `si5351_oscillator.cpp`, `si5351_oscillator.h` | Optional external oscillator, compiled into the receiver. |
| `tft_display.cpp`, `tft_display.h`, `spectrum_display.cpp`, `spectrum_display.h` | TFT/waterfall support, compiled into the receiver. |
| `build_all.sh` | Optional Bash build/package helper for all three architectures. |
| `.vscode/*.json` | Editor build, flash, and debug support; not needed for command-line CMake. |

These files are not dead code. Removing them would remove a feature or require
changes to CMake and related source code. `external/u8g2` and `.gitmodules` are
required dependencies. `build/` is generated and ignored by Git; deleting it
also deletes local firmware and test results, which must then be rebuilt.

## Renamed files

Apart from removing the unused logo include, source changes in this cleanup
are limited to filenames and their references. Existing
C++ class/function names, PIO program names, CMake targets, and firmware output
names remain compatible. Copyright and license comments are retained.

| Previous filename | Current filename |
| --- | --- |
| `picorx.cpp` | `receiver_main.cpp` |
| `rx.cpp` | `receiver.cpp` |
| `rx.h` | `receiver.h` |
| `rx_dsp.cpp` | `receiver_dsp.cpp` |
| `rx_dsp.h` | `receiver_dsp.h` |
| `rx_definitions.h` | `receiver_constants.h` |
| `ui.cpp` | `receiver_ui.cpp` |
| `ui.h` | `receiver_ui.h` |
| `settings.cpp` | `receiver_settings.cpp` |
| `settings.h` | `receiver_settings.h` |
| `memory.cpp` | `channel_memory.cpp` |
| `memory.h` | `channel_memory.h` |
| `autosave_memory.cpp` | `settings_storage.cpp` |
| `autosave_memory.h` | `settings_storage.h` |
| `utils.cpp` | `dsp_math.cpp` |
| `utils.h` | `dsp_math.h` |
| `fft.cpp` | `fixed_fft.cpp` |
| `fft.h` | `fixed_fft.h` |
| `fft_filter.cpp` | `fft_bandpass_filter.cpp` |
| `fft_filter.h` | `fft_bandpass_filter.h` |
| `cic_corrections.cpp` | `cic_compensation.cpp` |
| `cic_corrections.h` | `cic_compensation.h` |
| `clocks.h` | `receiver_clocks.h` |
| `clocks_pico.cpp` | `receiver_clocks_rp2040.cpp` |
| `clocks_pico2.cpp` | `receiver_clocks_rp2350.cpp` |
| `pins.h` | `board_pins.h` |
| `nco.cpp` | `quadrature_oscillator.cpp` |
| `nco.h` | `quadrature_oscillator.h` |
| `nco.pio` | `quadrature_oscillator.pio` |
| `cat.cpp` | `cat_control.cpp` |
| `cat.h` | `cat_control.h` |
| `button.cpp` | `input_button.cpp` |
| `button.h` | `input_button.h` |
| `encoder.h` | `tuning_encoder.h` |
| `button_encoder.cpp` | `button_tuning_encoder.cpp` |
| `button_encoder.h` | `button_tuning_encoder.h` |
| `rotary_encoder.cpp` | `rotary_tuning_encoder.cpp` |
| `rotary_encoder.h` | `rotary_tuning_encoder.h` |
| `quadrature_encoder.pio` | `rotary_encoder.pio` |
| `ssd1306.c` | `oled_display.c` |
| `ssd1306.h` | `oled_display.h` |
| `ili934x.cpp` | `tft_display.cpp` |
| `ili934x.h` | `tft_display.h` |
| `waterfall.cpp` | `spectrum_display.cpp` |
| `waterfall.h` | `spectrum_display.h` |
| `fonts.h` | `oled_fonts.h` |
| `battery_check.cpp` | `battery_monitor.cpp` |
| `quadrature_si5351.cpp` | `si5351_oscillator.cpp` |
| `quadrature_si5351.h` | `si5351_oscillator.h` |
| `build_all` | `build_all.sh` |

The previous README bench notes are preserved in [HARDWARE_BRINGUP.md](HARDWARE_BRINGUP.md).
