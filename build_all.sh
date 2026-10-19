#!/usr/bin/env bash
set -euo pipefail

cd -- "$(dirname -- "${BASH_SOURCE[0]}")"
repo_root="$PWD"
artifact_dir="$repo_root/build/artifacts"
riscv_toolchain="${PICO_RISCV_TOOLCHAIN_PATH:-/opt/riscv/riscv-toolchain-14}"

command -v cmake >/dev/null
command -v zip >/dev/null
mkdir -p "$artifact_dir"

build_and_package() {
    local build_dir="$1" board="$2" platform="$3" firmware="$4" archive="$5"
    shift 5
    cmake -S "$repo_root" -B "$build_dir" -DPICO_BOARD="$board" \
        -DPICO_PLATFORM="$platform" -DCMAKE_BUILD_TYPE=Release "$@"
    cmake --build "$build_dir" --parallel
    # -FS synchronizes archive contents, dropping any obsolete entries.
    (cd "$build_dir" && zip -FS "$artifact_dir/$archive" "$firmware")
}

build_and_package build pico rp2040 picorx.uf2 binaries-rp2040-pico.zip
build_and_package build_pico2 pico2 rp2350 pico2rx.uf2 binaries-rp2350-pico2.zip
build_and_package build_pico2_riscv pico2 rp2350-riscv pico2rx-riscv.uf2 \
    binaries-rp2350-riscv-pico2.zip -DPICO_TOOLCHAIN_PATH="$riscv_toolchain"
