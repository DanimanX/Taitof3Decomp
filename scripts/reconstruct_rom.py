#!/usr/bin/env python3
"""Reconstruct the dariusg maincpu 2 MiB image offline from roms/dariusg.zip.

Ground truth: MAME 0.289 src/mame/taito/taito_f3.cpp, ROM_START(dariusg):

    ROM_REGION( 0x200000, "maincpu", 0 )
    ROM_LOAD32_BYTE("d87-12.bin", 0x000000, 0x80000, CRC(de78f328) ...)
    ROM_LOAD32_BYTE("d87-11.bin", 0x000001, 0x80000, CRC(f7bed18e) ...)
    ROM_LOAD32_BYTE("d87-10.bin", 0x000002, 0x80000, CRC(4149f66f) ...)
    ROM_LOAD32_BYTE("d87-16.bin", 0x000003, 0x80000, CRC(8f7e5901) ...)

ROM_LOAD32_BYTE = byte interleave with stride 4: byte i of file F lands at
region address F.offset + 4*i.  This script rebuilds that region, then
(optionally) compares SHA-256 against the MAME Lua dump at work/maincpu.bin
to prove the driver-level deinterleave model is exact.

Usage:
    python3 scripts/reconstruct_rom.py [--zip roms/dariusg.zip]
                                       [--out work/maincpu_offline.bin]
                                       [--compare work/maincpu.bin]
"""
from __future__ import annotations

import argparse
import hashlib
import sys
import zipfile
import zlib

# (filename, region offset, size, expected CRC32) — from taito_f3.cpp 0.289
ROM_LOADS = [
    ("d87-12.bin", 0x000000, 0x80000, 0xDE78F328),
    ("d87-11.bin", 0x000001, 0x80000, 0xF7BED18E),
    ("d87-10.bin", 0x000002, 0x80000, 0x4149F66F),
    ("d87-16.bin", 0x000003, 0x80000, 0x8F7E5901),
]
REGION_SIZE = 0x200000
INTERLEAVE_STRIDE = 4


def load_member(zf: zipfile.ZipFile, name: str, size: int, want_crc: int) -> bytes:
    """Find a zip member by name; if absent, fall back to any member with the
    same size and CRC (some dumps ship the same chip under an alternate label,
    e.g. d87-wrld.bin for d87-16.bin)."""
    names = zf.namelist()
    candidates = []
    if name in names:
        candidates.append(name)
    for alt in names:
        if alt == name:
            continue
        data = zf.read(alt)
        if len(data) == size and (zlib.crc32(data) & 0xFFFFFFFF) == want_crc:
            candidates.append(alt)
    if not candidates:
        raise SystemExit(f"ERROR: required ROM '{name}' not found in zip")
    chosen = candidates[0]
    data = zf.read(chosen)
    if len(data) != size:
        raise SystemExit(f"ERROR: {chosen}: size {len(data)} != expected {size}")
    crc = zlib.crc32(data) & 0xFFFFFFFF
    if crc != want_crc:
        raise SystemExit(
            f"ERROR: {chosen}: CRC {crc:08X} != expected {want_crc:08X} "
            f"(dariusg.zip may be a wrong set/clone)"
        )
    if chosen != name:
        print(f"NOTE: '{name}' present under alternate label '{chosen}' "
              f"(identical bytes, CRC {crc:08X})")
    return data


def reconstruct(zf: zipfile.ZipFile) -> bytes:
    region = bytearray(REGION_SIZE)
    for name, offset, size, want_crc in ROM_LOADS:
        data = load_member(zf, name, size, want_crc)
        region[offset::INTERLEAVE_STRIDE] = data
        print(f"loaded {name:12s} -> region[{offset:#08x}] stride "
              f"{INTERLEAVE_STRIDE}, {size:#x} bytes, CRC {want_crc:08x} OK")
    return bytes(region)


def sha256(b: bytes) -> str:
    return hashlib.sha256(b).hexdigest()


def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("--zip", default="roms/dariusg.zip")
    ap.add_argument("--out", default="work/maincpu_offline.bin")
    ap.add_argument("--compare", default="work/maincpu.bin")
    args = ap.parse_args()

    with zipfile.ZipFile(args.zip) as zf:
        region = reconstruct(zf)

    with open(args.out, "wb") as f:
        f.write(region)
    print(f"\nwrote {args.out} ({len(region)} bytes) sha256={sha256(region)}")

    try:
        dumped = open(args.compare, "rb").read()
    except OSError as exc:
        print(f"\nno MAME dump to compare ({exc}); run the Lua dump first")
        return 0

    if dumped == region:
        print(f"ORACLE MATCH: MAME dump == offline reconstruction "
              f"(identical {len(region)} bytes, sha256={sha256(region)})")
        return 0
    print(f"\nMISMATCH: dump len={len(dumped)} sha={sha256(dumped)} "
          f"vs offline len={len(region)} sha={sha256(region)}")
    for i in range(min(len(dumped), len(region))):
        if dumped[i] != region[i]:
            print(f"first diff at {i:#08x}: dump={dumped[i]:02x} offline={region[i]:02x}")
            break
    return 1


if __name__ == "__main__":
    sys.exit(main())
