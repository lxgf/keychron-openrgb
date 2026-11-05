#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-2.0-or-later
"""
VIA RGB Matrix protocol probe.

Talks to the raw HID interface (usage page 0xFF60, usage 0x61) of a stock
QMK/VIA keyboard and exercises the lighting commands used by the plugin.
Nothing is ever saved to EEPROM unless --save is passed.

    pip install hidapi
    python tools/via_probe.py                   # handshake + read state
    python tools/via_probe.py --probe-effects   # find max effect id
    python tools/via_probe.py --color C600FF    # set color (no save)
"""

import argparse
import colorsys
import sys
import time

import hid

USAGE_PAGE = 0xFF60
USAGE = 0x61
PACKET = 32

CMD_GET_PROTOCOL_VERSION = 0x01
CMD_CUSTOM_SET_VALUE = 0x07
CMD_CUSTOM_GET_VALUE = 0x08
CMD_CUSTOM_SAVE = 0x09
CMD_UNHANDLED = 0xFF

CHANNEL_RGB_MATRIX = 0x03
VAL_BRIGHTNESS = 0x01
VAL_EFFECT = 0x02
VAL_SPEED = 0x03
VAL_COLOR = 0x04


def find_device(vid, pid):
    for info in hid.enumerate(vid, pid):
        if info["usage_page"] == USAGE_PAGE and info["usage"] == USAGE:
            return info
    return None


def xfer(dev, payload, verbose):
    buf = bytes(payload) + bytes(PACKET - len(payload))
    dev.write(b"\x00" + buf)
    t0 = time.perf_counter()
    resp = bytes(dev.read(PACKET, 100))
    dt = (time.perf_counter() - t0) * 1000
    if verbose:
        print(f"  > {buf[:8].hex(' ')}\n  < {resp[:8].hex(' ')}  ({dt:.1f} ms)")
    if not resp:
        raise RuntimeError(f"timeout on {buf[:4].hex(' ')}")
    if resp[0] == CMD_UNHANDLED:
        raise RuntimeError(f"unhandled: {buf[:4].hex(' ')}")
    return resp


def get_value(dev, value_id, verbose):
    resp = xfer(dev, [CMD_CUSTOM_GET_VALUE, CHANNEL_RGB_MATRIX, value_id], verbose)
    return resp[3:5]


def set_value(dev, value_id, data, verbose):
    xfer(dev, [CMD_CUSTOM_SET_VALUE, CHANNEL_RGB_MATRIX, value_id] + list(data), verbose)


def rgb_to_qmk_hs(hex_rgb):
    r, g, b = (int(hex_rgb[i:i + 2], 16) / 255.0 for i in (0, 2, 4))
    h, s, v = colorsys.rgb_to_hsv(r, g, b)
    return round(h * 255) & 0xFF, round(s * 255), round(v * 255)


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--vid", type=lambda x: int(x, 16), default=0x3434)
    ap.add_argument("--pid", type=lambda x: int(x, 16), default=0x0520)
    ap.add_argument("--probe-effects", action="store_true")
    ap.add_argument("--color", help="RRGGBB, applied without saving")
    ap.add_argument("--save", action="store_true", help="send 09 03 at the end")
    ap.add_argument("-v", "--verbose", action="store_true")
    args = ap.parse_args()

    info = find_device(args.vid, args.pid)
    if not info:
        sys.exit(f"no raw HID interface for {args.vid:04x}:{args.pid:04x}")
    print(f"{info['manufacturer_string']} {info['product_string']}  {info['path'].decode()}")

    dev = hid.device()
    dev.open_path(info["path"])
    try:
        resp = xfer(dev, [CMD_GET_PROTOCOL_VERSION], args.verbose)
        print(f"VIA protocol: 0x{(resp[1] << 8) | resp[2]:04X}")

        bri = get_value(dev, VAL_BRIGHTNESS, args.verbose)[0]
        eff = get_value(dev, VAL_EFFECT, args.verbose)[0]
        spd = get_value(dev, VAL_SPEED, args.verbose)[0]
        hue, sat = get_value(dev, VAL_COLOR, args.verbose)
        print(f"brightness={bri} effect={eff} speed={spd} hue={hue} sat={sat}")

        if args.probe_effects:
            set_value(dev, VAL_EFFECT, [0xFF], args.verbose)
            last = get_value(dev, VAL_EFFECT, args.verbose)[0]
            set_value(dev, VAL_EFFECT, [eff], args.verbose)
            back = get_value(dev, VAL_EFFECT, args.verbose)[0]
            print(f"max effect id={last}  (restored effect={back})")

        if args.color:
            h, s, v = rgb_to_qmk_hs(args.color)
            print(f"#{args.color} -> hue={h} (0x{h:02x}) sat={s} (0x{s:02x})  [v={v} ignored]")
            set_value(dev, VAL_COLOR, [h, s], args.verbose)

        if args.save:
            xfer(dev, [CMD_CUSTOM_SAVE, CHANNEL_RGB_MATRIX], args.verbose)
            print("saved")
    finally:
        dev.close()


if __name__ == "__main__":
    main()
