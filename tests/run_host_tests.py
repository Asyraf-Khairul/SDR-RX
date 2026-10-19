"""Compile and execute the repository's DSP with single-threaded Pico SDK shims.

Requires Python 3 and a native g++ (or CXX). Does not emulate hardware/timing.
"""
import json
import os
from pathlib import Path
import subprocess

ROOT = Path(__file__).resolve().parents[1]
BUILD = ROOT / "build" / "host-tests"
BUILD.mkdir(parents=True, exist_ok=True)
EXE = BUILD / ("dsp_probe.exe" if os.name == "nt" else "dsp_probe")
sources = ["tests/dsp_probe.cpp", "receiver_dsp.cpp", "fixed_fft.cpp", "fft_bandpass_filter.cpp",
           "noise_reduction.cpp", "cic_compensation.cpp", "dsp_math.cpp", "ring_buffer_lib.c"]
subprocess.run([os.environ.get("CXX", "g++"), "-std=gnu++17", "-O2", "-fwrapv",
                "-Wall", "-Werror", "-Itests/host_pico", "-I.",
                *sources, "-o", str(EXE)], cwd=ROOT, check=True)
results = {}
failures = []
for scenario in ["quiet", "quiet_correction", "quiet_zero_correction", "carrier",
                 "am", "am_agc", "am_correction", "recovery_correction",
                 "amsync", "fm", "lsb", "usb", "lsb_reject", "usb_reject", "cw"]:
    run = subprocess.run([str(EXE), scenario], capture_output=True, text=True, timeout=60)
    if run.returncode:
        failures.append(f"{scenario}: exited {run.returncode}: {run.stderr}")
        print("FAIL", failures[-1])
        continue
    result = json.loads(run.stdout)
    results[scenario] = result
    print(json.dumps(result))
    if result["clipped"]:
        failures.append(f"{scenario}: clipped output")
    if scenario.startswith("quiet") or scenario == "carrier":
        if result["rms"] > 100:
            failures.append(f"{scenario}: unexpected audio RMS {result['rms']}")
    elif "reject" not in scenario:
        if result["tone"] < 100 or result["tone"] ** 2 / max(2 * result["rms"] ** 2, 1) < 0.8:
            failures.append(f"{scenario}: expected dominant recovered tone")
for mode in ["lsb", "usb"]:
    if mode in results and mode + "_reject" in results:
        if results[mode + "_reject"]["rms"] > results[mode]["rms"] / 10:
            failures.append(f"{mode}: opposite sideband rejection below 20 dB")
(BUILD / "results.json").write_text(json.dumps({"results": results, "failures": failures}, indent=2) + "\n")
if failures:
    raise SystemExit("\n".join(failures))
print(f"PASS: {len(results)} DSP scenarios")
