# SDR-RX

SDR receiver firmware for a Raspberry Pi Pico (RP2040), adapted from
[Jon Dawson's PicoRX](https://github.com/dawsonjon/PicoRX) for a final-year
electrical engineering prototype.

The prototype has four boards: an antenna connector/optional LNA, a low-pass
filter bank, a Tayloe quadrature sampling mixer, and a Pico/UI board. The
recorded bench setup uses the LPF, mixer, and Pico/UI boards, with a signal
generator replacing the antenna.

## Project status

Software checks on **19 October 2026** passed the RP2040 release build and all
**15 host DSP regression scenarios**. The DSP includes a fix for division by
zero when I/Q correction is enabled with a silent input.

After the filename cleanup, fresh RP2040 builds with rotary and button tuning
controls and a Pico 2 ARM build also passed. The RISC-V variant was not rebuilt;
its toolchain is not installed in the validation environment.

**Physical receiver validation is still pending.** No Pico was connected during
the software checks, so the firmware was not flashed or tested for real RF
reception, display operation, USB streaming, or real-time ADC/PWM performance.
The last recorded bench measurements did not demonstrate recovered audio.

The measurements, hypotheses, and diagnostic steps from 5 October 2026 are
preserved in [the hardware bring-up log](docs/HARDWARE_BRINGUP.md).
The project-specific schematic PDF is stored separately and is not included
in this repository.

## Receiver features

- AM, synchronous AM, LSB, USB, FM, and CW receive modes.
- ADC I/Q capture, CIC decimation, FFT filtering, AGC, and squelch.
- Optional I/Q correction, noise reduction, and automatic notch filtering.
- PWM audio, USB audio/IQ streaming, and CAT control.
- SH1106/SSD1306 OLED UI, optional TFT spectrum/waterfall display, and rotary
  or button tuning controls.
- Frequency memories, saved settings, selectable LPF bands, and an optional
  Si5351 oscillator.

SSTV is disabled because the decoder sources are absent. Pico 2 ARM and RISC-V
build configurations are retained; this prototype uses the RP2040 Pico.

## Signal path and connections

```text
Antenna / generator -> AC coupling and DC bias -> selected LPF
  -> Tayloe mixer and I/Q amplifiers -> Pico ADC input networks
  -> receiver DSP -> PWM / USB audio
```

Default pin assignments are defined in [board_pins.h](board_pins.h).

| Function | Pico GPIO |
| --- | --- |
| Quadrature LO outputs | GP0, GP1 |
| LPF selection | GP2, GP3, GP4 |
| I / Q ADC inputs | GP26 / GP27 |
| PWM audio output | GP16 |
| OLED I2C SDA / SCL | GP18 / GP19 |
| Rotary encoder A / B | GP20 / GP21 |
| Encoder push / back / menu | GP5 / GP17 / GP22 |

For the recorded AM bench test, tune to **756 kHz** and apply a carrier with
**1 kHz AM**. The default IF magnitude is 4.5 kHz; a high-side LO is approximately
760.5 kHz. Confirm the actual mixer bias and Pico-side ADC voltages using the
[bring-up procedure](docs/HARDWARE_BRINGUP.md#next-diagnostic-steps) before
connecting I/Q signals. The ADC input range is 0–3.3 V.

## Build

Clone with the required display-library submodule:

```sh
git clone --recurse-submodules https://github.com/Asyraf-Khairul/SDR-RX.git
cd SDR-RX
```

For an existing clone, run `git submodule update --init --recursive`.
Use **Pico SDK 2.1.0**, CMake, and an ARM embedded GCC toolchain. The verified
Windows toolchain is ARM GCC 13.3.1. See [BUILD.rst](BUILD.rst) for dependency
setup, the TinyUSB version constraint, and Windows PowerShell commands using
the installed Pico VS Code tools.

From a shell with those tools available:

```sh
cmake -S . -B build -DPICO_BOARD=pico -DPICO_PLATFORM=rp2040 -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
```

The receiver firmware is **`build/picorx.uf2`**. The separate
`build/battery_check.uf2` image is a battery diagnostic utility.

Source filenames have been clarified; CMake targets and firmware output names
remain compatible. Generated firmware ZIPs are no longer tracked. The optional
`bash build_all.sh` helper builds all three architectures and writes packages
under the ignored `build/artifacts/` directory. It also needs a RISC-V toolchain
and the `zip` utility.

## Flash and test

1. Hold **BOOTSEL** while connecting the RP2040 Pico over USB.
2. Copy `build/picorx.uf2` to the **RPI-RP2** drive. The board then reboots.
3. Verify the display and controls, tune to the test signal, and check recovered
   audio. Before finalizing the hardware release, also verify LPF selection,
   saved settings, USB audio/CAT, and stable operation without watchdog resets.

Run the host DSP suite with Python 3 and a **native GNU C++ compiler** on PATH:

```sh
python tests/run_host_tests.py
```

The 15 scenarios cover silence, I/Q correction and recovery, an unmodulated
carrier, AM with manual/automatic gain, synchronous AM, FM, LSB/USB reception
and opposite-sideband rejection, and CW. Results are written to
`build/host-tests/results.json`. These tests run the receiver DSP with
single-threaded SDK shims; they do not validate the peripherals or timing.

## Code guide

| Files | Responsibility |
| --- | --- |
| `receiver_main.cpp` | Startup, core launch, and main scheduling loop. |
| `receiver.cpp/.h` | ADC capture, tuning, LPF selection, and receiver orchestration. |
| `receiver_dsp.cpp/.h`, `receiver_constants.h` | Demodulation, AGC, squelch, and sample-rate constants. |
| `fixed_fft.*`, `fft_bandpass_filter.*`, `cic_compensation.*`, `dsp_math.*`, `noise_reduction.*` | DSP transforms, filters, and numerical helpers. |
| `quadrature_oscillator.*`, `si5351_oscillator.*`, `receiver_clocks*` | Internal/external LO generation and clock configuration. |
| `receiver_ui.*`, `oled_display.*`, `tft_display.*`, `spectrum_display.*` | Menus, displays, spectrum, and waterfall. |
| `input_button.*`, `tuning_encoder.h`, `*_tuning_encoder.*`, `rotary_encoder.pio` | Front-panel inputs. |
| `receiver_settings.*`, `channel_memory.*`, `settings_storage.*` | Settings, channel memories, and flash-backed storage. |
| `cat_control.*`, `usb_*`, `tusb_config.h`, `pwm_audio_sink.*` | CAT commands and USB/PWM audio. |
| `board_pins.h` | Prototype pin assignments. |
| `external/u8g2/` | Required upstream display library submodule. |
| `tests/` | Host DSP regression runner and SDK shims. |

The [file audit](docs/FILE_AUDIT.md) lists every removed file, optional components
that remain in use, and the complete old-to-new filename mapping.

## Attribution

This project builds on [PicoRX by Jon Dawson](https://github.com/dawsonjon/PicoRX).
Original copyright and license notices remain in the source files. The display
library and other third-party components retain their upstream notices.
For the upstream hardware design, see the
[PicoRX hardware wiki](https://github.com/dawsonjon/PicoRX/wiki/Hardware-Design).
