Pi Pico Rx - Build Instructions
"""""""""""""""""""""""""""""""

Getting the Code
----------------

.. code::

  sudo apt install git
  git clone --recurse-submodules https://github.com/Asyraf-Khairul/SDR-RX.git
  cd SDR-RX

For an existing clone, run ``git submodule update --init --recursive`` before
configuring. The display library in ``external/u8g2`` is required.

Install Build Dependencies
--------------------------

Install the following packages using your distributions package manager:

- `cmake`
- `gcc-arm-none-eabi`

For Ubuntu and other Debian based Linux distributions, the following command
line can be used to install all dependencies at once.

.. code::

  sudo apt install git cmake gcc-arm-none-eabi

Install Pi Pico SDK (Optional)
------------------------------

This step is now optional and only necessary if offline (no internet) builds
are desired. If this step is skipped, the configure step of the CMake build will
pull down the necessary Pico SDK dependencies.

Follow the `Getting started with the Raspberry Pi Pico <https://datasheets.raspberrypi.com/pico/getting-started-with-pico.pdf>`_ quick start guide to install the C/C++ SDK.

.. code::

  sudo apt install wget #if wget not installed
  wget https://raw.githubusercontent.com/raspberrypi/pico-setup/master/pico_setup.sh
  chmod +x pico_setup.sh
  ./pico_setup.sh

**NOTE**: pico-sdk versions 2.1.1 and 2.2.0 both use version 0.18.0 of
the tinyusb library, and a bug in that version of tinyusb prevents the
PicoRx firmware from running correctly. Use pico-sdk 2.1.0, or pico-sdk
2.2.0 with the tinyusb submodule updated to version 0.19.0. If you choose
not to install pico-sdk in your build environment, cmake will download
a known-good version of pico-sdk into your build directory automatically.

Build Project
-------------

Build the project using these cmake commands.

.. code::

  cmake -S . -B build -DPICO_BOARD=pico -DPICO_PLATFORM=rp2040 -DCMAKE_BUILD_TYPE=Release
  cmake --build build --parallel

The receiver image is ``build/picorx.uf2``. ``build/battery_check.uf2`` is a
separate battery-check utility, not the receiver. SSTV is disabled by default
because its decoder sources are absent from this repository.

**NOTE**: When using the installed SDK described in the previous section, ensure
the ``PICO_SDK_PATH`` environment variable exists on your path by running:
``echo ${PICO_SDK_PATH}``. If the output is empty, refresh the environment by
either starting a new shell or by running: ``source ~/.bashrc``.

Windows PowerShell (Pico VS Code tools)
--------------------------------------

With SDK 2.1.0 and the tools used by this repository's VS Code configuration
installed under ``%USERPROFILE%/.pico-sdk``, run from the repository root:

.. code:: powershell

  $picoTools = "$env:USERPROFILE/.pico-sdk"
  $env:PICO_SDK_PATH = "$picoTools/sdk/2.1.0"
  $env:PATH = "$picoTools/toolchain/13_3_Rel1/bin;$picoTools/ninja/v1.12.1;$env:PATH"
  git submodule update --init --recursive
  & "$picoTools/cmake/v3.29.9/bin/cmake.exe" -S . -B build -G Ninja -DPICO_BOARD=pico -DPICO_PLATFORM=rp2040 -DCMAKE_BUILD_TYPE=Release
  & "$picoTools/cmake/v3.29.9/bin/cmake.exe" --build build --parallel

Check that each command succeeds before continuing. Use a separate build
directory if another generator or board was previously configured.

Host DSP regression tests
-------------------------

With Python 3 and a native GNU C++ compiler on PATH (not ``arm-none-eabi-g++``):

.. code::

  python tests/run_host_tests.py

The runner compiles the actual receiver DSP sources and checks 15 scenarios:
silence at midscale and zero, I/Q correction during silence and signal recovery,
an unmodulated carrier, 1 kHz AM with manual/automatic gain, synchronous AM,
FM, LSB/USB reception and opposite-sideband rejection, and the quantized CW
sidetone. It checks recovered tone energy, clipping, and at least 20 dB
opposite-sideband rejection. Results are saved to
``build/host-tests/results.json``; a failed check gives a nonzero exit status.
Set ``CXX`` to select a different native GNU C++ compiler.

These are single-threaded tests with minimal SDK shims. Each scenario starts
in a fresh process and processes 1024 ADC blocks (about 4.37 seconds), with
the first half excluded from audio measurements. Host arithmetic uses
``-fwrapv`` for the CIC's intended wrapping integrators. The tests do not
validate target execution time, concurrent access, peripherals, or all settings.

Run on the receiver
--------------------

Hold BOOTSEL while connecting the RP2040 Pico over USB, then copy the freshly
built ``build/picorx.uf2`` to its ``RPI-RP2`` drive. The Pico reboots into the
receiver firmware. Check the SH1106 display and controls, tune to the desired
frequency, and verify PWM audio and USB operation on the assembled receiver.
For the project's 756 kHz AM test, follow the signal and bias checks in
``docs/HARDWARE_BRINGUP.md`` before applying the test signal.

A successful build or host test does not establish physical reception. Before
finalizing the hardware release, confirm 1 kHz recovered audio, stable operation
without watchdog resets, tuning/LPF selection, controls, settings persistence,
and USB audio/CAT on the RP2040 board. The ``binaries-*.zip`` archives are not
automatically regenerated by the commands above; use the fresh UF2 after
making source changes.

Verification on 19 October 2026
-------------------------------

The RP2040 Release build completed with SDK 2.1.0 and ARM GCC 13.3.1, including
the receiver and battery-check targets. All 15 host DSP scenarios passed.
After the filename cleanup, fresh RP2040 builds with rotary and button tuning
controls and a Pico 2 ARM build also passed. The RISC-V variant was not rebuilt
because its toolchain is not installed in the validation environment.
The tests reproduced a divide-by-zero with silent input and I/Q correction
enabled; the corrected implementation passes silence and recovery scenarios.
Generated ZIPs have been removed from source control. Use the freshly built
``build/picorx.uf2``. The optional ``bash build_all.sh`` helper writes archives
under ``build/artifacts/`` and requires ``zip`` plus a RISC-V toolchain.
Set ``PICO_RISCV_TOOLCHAIN_PATH`` to the RISC-V toolchain directory when it is
not installed at ``/opt/riscv/riscv-toolchain-14``. That helper is not required
for the RP2040 build above.

No accessible Pico was connected, so no firmware was flashed and no physical
receiver run was performed. Hardware release validation remains outstanding.
