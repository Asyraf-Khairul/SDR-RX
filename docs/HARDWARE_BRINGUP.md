# SDR RX prototype: bring-up and diagnostic log

Last updated: 5 October 2026 (Auckland, New Zealand).

This is the historical bench log from 5 October 2026. See the [project README](../README.md) for current software validation and [build instructions](../BUILD.rst).

This project adapts Jon Dawson's PicoRX firmware to a four-board receiver built on perforated boards for a final-year electrical engineering project. The boards are the antenna connector/optional LNA, low-pass filter (LPF) bank, Tayloe quadrature sampling mixer, and UI/Pico board. The UI board distributes 5 V and a common ground to the other boards in a star arrangement. RF, LO, and I/Q signals travel between boards through jumper cables.

The current bench test uses **only the LPF, mixer, and UI/Pico boards**. A Tektronix AFG1022 substitutes for the antenna at 756 kHz. There is no preamplifier in this three-board setup. The objective is to hear a 1 kHz tone from a known AM test signal before attempting off-air reception of RNZ National on 756 kHz.

## Expected signal path

```text
AFG/antenna -> AC coupling and DC bias -> selected LPF -> Tayloe switch
            -> I/Q amplifiers -> UI-board ADC input networks -> Pico ADC
            -> AM processing -> PWM audio output
```

The AFG produces a 756 kHz carrier with 1 kHz amplitude modulation (AM). One carrier cycle lasts about 1.32 microseconds; one modulation-envelope cycle lasts 1 ms. The firmware's default intermediate frequency (IF) is 4.5 kHz, so a high-side LO is expected near 760.5 kHz when tuned to 756 kHz. Mixing these signals should produce I and Q waveforms near the 4.5 kHz difference, with the 1 kHz AM information preserved in their changing amplitude. The Pico then samples I and Q and extracts the audio.

At 756 kHz, the firmware drives GP2/GP3/GP4 as `0/0/1`. The LPF selector labels its inputs S2/S1/S0, so the same levels read `1/0/0` in that order. This selects the 0–2 MHz path, B5, in the prototype filter schematic. The relevant firmware is in [receiver.cpp](../receiver.cpp), [receiver_settings.h](../receiver_settings.h), and [board_pins.h](../board_pins.h).

## Measurements to date

The table separates observations from conclusions. Scope readings were made under different connection and power conditions; values from those conditions should not be compared as if they were one continuous test.

| Stage and condition | Observation | What it establishes |
| --- | --- | --- |
| AFG1022, AM enabled | Envelope peak-to-peak spacing was **1 ms**. | The generator is applying approximately **1 kHz AM**. |
| LPF test, AM disabled, mixer disconnected | About **68 mVpp at LPF input** and **74.4 mVpp at LPF output** in one test; a later isolated output reading was **756 kHz, 50 mVpp**. | The selected LPF path passes the 756 kHz carrier. The small apparent output increase may reflect measurement uncertainty or circuit loading; it is not evidence of active gain. |
| Pico LO outputs GP0 and GP1 | Both had waveforms near **760 kHz** and approximately **3.6 Vpp**. | The Pico is producing LO signals near the expected 760.5 kHz. This does not yet establish correct LO phase or correct switching at the mixer IC. |
| Receiver signal-strength indication | Turning the AFG output on/off changed the displayed reading by roughly **1–2 dB**. | The displayed magnitude changed, but this does not prove successful mixing or AM demodulation. |
| Mixer output terminals, mixer powered, AFG output off, ADC jumpers disconnected | I: **3.84–3.88 V**; Q: **3.78–3.82 V**. | Both raw mixer outputs sit at a high DC level and show only about 40 mVpp variation without the test signal. Their response to a correctly biased RF test signal remains unmeasured. |
| Mixer RF input, mixer powered, AFG output off | Approximately **3.3 V DC**. With both LO jumpers removed it was **3.26 V DC**. | The LO pins are not directly setting this DC voltage. The RF input may be floating or driven by another circuit connection. |
| Power-off resistance check | `OL` between mixer RF input and each of the two LO input terminals. | No direct DC short was found between RF and either LO input. This does not rule out capacitive coupling or another wiring fault. |

### Readings that need confirmation

- The scope sometimes showed an approximately **760 kHz, 3 Vpp** waveform at the connected LPF-output/mixer-input point. The frequency readout was intermittent. Its origin has not been established.
- A preliminary probe at the Pico ADC pins showed no clear signal at GP26 and an unstable, high-frequency reading at GP27. Scope triggering and measurement settings were uncertain, so this is not a reliable measurement of the I/Q signals.
- One RF-input reading of **560 mVpp** was taken while the mixer board was accidentally unpowered; it is excluded from the powered-mixer diagnosis. After correcting the power state, the AFG-off RF-input reading was about **80 mVpp**.
- With LO jumpers removed, an AFG-on RF-input reading of about **200 mVpp** fell to **0 Vpp** when the AFG output was turned off. That waveform therefore depended on the generator.
- The scope's automatic frequency function required a large AFG amplitude to lock reliably. The minimum AFG setting of 2 mVpp was difficult to see on the scope. Higher amplitudes were used for signal tracing and may overdrive the high-gain mixer; they are not a suitable full-receiver listening level.

## Why a carrier or a small strength change may still be silent

The AM processing in [receiver_dsp.cpp](../receiver_dsp.cpp) subtracts the steady part of the measured amplitude. An **unmodulated** carrier can therefore change the signal-strength indication while producing no tone. Hearing the intended 1 kHz test tone requires the AFG's AM to be enabled **and** valid I/Q signals to reach the ADC and audio path. The 1–2 dB display change alone cannot verify either requirement.

## Bias circuit and current working hypothesis

The separate **SDR RX Prototype Schematic.pdf** has four pages. Page 4 places the **C33 10 nF input coupling capacitor** and the **R12/R13 10 kΩ divider** on the optional antenna/LNA section. The divider makes approximately 2.5 V from the 5 V supply; the capacitor passes RF while blocking a generator's DC connection. Page 3 shows the mixer and I/Q amplifiers. In Dawson's original no-preamp arrangement, the RF path still receives DC bias. A 10 nF series capacitor alone does **not** create that bias.

Because the optional antenna/LNA board is absent from the current three-board test, its coupling and bias components are also absent **unless equivalent parts were added elsewhere on the physical boards**. That makes missing or incorrect RF bias the leading hypothesis for the unusual DC readings. A floating RF node could settle near 3.3 V through leakage or another weak path. A wiring connection could also drive it there. The measured 3.26 V does not distinguish those explanations by itself.

The raw I/Q outputs near 3.8 V deserve investigation, but they do not by themselves prove the Pico ADC pins are at 3.8 V. Page 1 of the prototype schematic shows **470 Ω in series and 1 kΩ to ground** on each ADC input. If those networks were built as drawn, a steady 3.8 V mixer output would become about `3.8 × 1000 / (470 + 1000) = 2.6 V` at the Pico pin. The actual pin voltage, including signal peaks, must be measured before reconnecting GP26/GP27. The Pico ADC operates over 0–3.3 V.

**Current status:** the AFG modulation, selected LPF path, and Pico LO have each shown the expected basic behaviour. A clean 4.5 kHz I/Q signal has not yet been demonstrated, the RF-input DC source has not yet been identified, and audio has not been recovered. This is a hardware diagnosis in progress; neither a missing bias nor a mixer component fault has been confirmed.

## Next diagnostic steps

1. Keep the GP26/GP27 jumpers disconnected. With power off, temporarily connect a **10 kΩ resistor from the mixer RF input to mixer ground**. Keep the AFG output off and LO jumpers removed, then power the mixer and remeasure RF-input DC. A fall toward 0 V suggests a floating node; a voltage remaining near 3.3 V suggests it is being driven. Remove the temporary resistor after this check.
2. If the node is driven, disconnect the LPF-to-mixer RF jumper with power off and trace which side supplies the voltage. If it is floating, compare the physical build to the prototype schematic and provide an AC-coupled, approximately 2.5 V RF bias path for this three-board test. Verify the bias with a multimeter before reconnecting ADC jumpers.
3. With the DC operating point established, set the AFG to **756 kHz sine, AM off, about 2 mVpp**. Probe the mixer I and Q outputs for a signal near **4.5 kHz** and measure their minimum and maximum voltages. Confirm that the 470 Ω/1 kΩ input networks were built as drawn and that the measured mixer output peaks would keep the Pico-side voltage between 0 and 3.3 V. Then reconnect one ADC channel at a time and verify the voltage at its Pico pin.
4. Once both ADC inputs stay within their allowed range and carry valid I/Q, enable **1 kHz AM** on the AFG. Use AM receive mode, adequate volume, S0 squelch, and automatic notch filtering off. If 1 kHz audio is still absent, trace the PWM audio path from GP16 to the speaker/headphones.

## References

- [Jon Dawson's PicoRX hardware design](https://github.com/dawsonjon/PicoRX/wiki/Hardware-Design), including the Tayloe detector, gain, and Pico ADC range.
- [Original PicoRX schematic](https://github.com/dawsonjon/PicoRX/blob/master/PCB/pico_rx/pipicorx.pdf), including the no-preamp bias arrangement.
- The project-specific **SDR RX Prototype Schematic.pdf** is currently stored separately from this repository and should accompany this log when reviewing the physical prototype.
