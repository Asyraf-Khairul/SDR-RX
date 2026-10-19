// Each scenario runs in a fresh process to isolate function-static DSP state.
#include "receiver_dsp.h"
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <string>

static rx_dsp receiver; // Match firmware static zero-initialization.
int main(int argc, char **argv) {
    if (argc != 2) return 2;
    const std::string scenario = argv[1];
    const bool quiet = scenario.find("quiet") != std::string::npos;
    const bool correction = scenario.find("correction") != std::string::npos;
    const bool recovery = scenario == "recovery_correction";
    const bool carrier = scenario == "carrier";
    const bool lsb = scenario == "lsb" || scenario == "lsb_reject";
    const bool usb = scenario == "usb" || scenario == "usb_reject";
    const bool reject = scenario.find("reject") != std::string::npos;
    const bool fm = scenario == "fm";
    const bool cw = scenario == "cw";
    receiver.set_mode(fm ? FM : cw ? CW : lsb ? LSB : usb ? USB : scenario == "amsync" ? AMSYNC : AM, 2);
    receiver.set_frequency_offset_Hz(-4500);
    receiver.set_agc_control(scenario == "am_agc" ? 3 : 4, scenario == "am_agc" ? 10 : 4);
    receiver.set_gain_cal_dB(62);
    receiver.set_squelch(0, 0);
    receiver.set_iq_correction(correction);
    receiver.set_impulse_threshold(0);
    receiver.set_cw_sidetone_Hz(700);
    receiver.amsync_reset();
    constexpr double pi = 3.14159265358979323846;
    constexpr unsigned blocks = 1024, settle = 512;
    double sum = 0, energy = 0, re = 0, im = 0;
    unsigned count = 0, clipped = 0;
    uint16_t raw[adc_block_size];
    int16_t audio[adc_block_size / decimation_rate];
    uint8_t spectrum[256], waveform[128], scale;
    for (unsigned block = 0; block < blocks; ++block) {
        for (unsigned n = 0; n < adc_block_size; ++n) {
            const double t = double(block * adc_block_size + n) / adc_sample_rate;
            double f = -4500;
            if (lsb || usb) f += ((lsb != reject) ? -1000 : 1000);
            double phase = 2 * pi * f * t;
            if (fm) phase += 0.5 * std::sin(2 * pi * 1000 * t);
            double amplitude = (quiet || (recovery && block < 256)) ? 0 : 180;
            if (!carrier && !lsb && !usb && !fm && !cw)
                amplitude *= 1 + 0.5 * std::cos(2 * pi * 1000 * t);
            const double value = amplitude * ((n & 1) ? std::sin(phase) : std::cos(phase));
            raw[n] = scenario == "quiet_zero_correction" ? 0 :
                std::clamp<long>(std::lround(2048 + value), 0, 4095);
        }
        host_device_us = uint64_t(block) * adc_block_size * 1000000 / adc_sample_rate;
        if (receiver.process_block(raw, audio, nullptr) != adc_block_size / decimation_rate) return 3;
        receiver.get_spectrum(spectrum, scale, 1);
        receiver.get_audio_capture(waveform);
        int16_t i, q;
        while (receiver.get_raw_data(i, q)) {}
        if (block < settle) continue;
        for (const int16_t sample : audio) {
            // The firmware rounds the CW phase increment down to a table step.
            const double cw_frequency = (700 * 2048 / audio_sample_rate) * audio_sample_rate / 2048.0;
            const double angle = 2 * pi * (cw ? cw_frequency : 1000) * count / audio_sample_rate;
            sum += sample; energy += double(sample) * sample;
            re += sample * std::cos(angle); im += sample * std::sin(angle);
            clipped += std::abs(int(sample)) >= 32767;
            ++count;
        }
    }
    const double mean = sum / count;
    const double rms = std::sqrt(std::max(0.0, energy / count - mean * mean));
    const double tone = 2 * std::hypot(re, im) / count;
    std::printf("{\"scenario\":\"%s\",\"rms\":%.6f,\"tone\":%.6f,\"clipped\":%u,\"samples\":%u}\n",
                scenario.c_str(), rms, tone, clipped, count);
}
