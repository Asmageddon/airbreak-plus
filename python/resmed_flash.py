#!/usr/bin/env python3
"""
ResMed AirSense UART Flash Tool

Flashes firmware to ResMed AirSense devices via the UART bootloader protocol.
Supports individual block or full image flashing with CRC validation and
automatic baud rate negotiation up to 460800.

Flash memory map:
  0x08000000  BLX  16KB   Bootloader
  0x08004000  CCX  240KB  Configuration
  0x08040000  CDX  768KB  Firmware
              CMX  1008KB CCX+CDX combined

"""

import serial
import argparse
import time
import sys
import struct


BLOCKS = {
    'BLX': {'flash_start': 0x08000000, 'file_offset': 0x00000, 'size': 0x04000,
            'name': 'Bootloader', 'erase_cmd': 'P F *BLX 0000'},
    'CCX': {'flash_start': 0x08004000, 'file_offset': 0x04000, 'size': 0x3C000,
            'name': 'Config',     'erase_cmd': 'P F *CCX 0000'},
    'CDX': {'flash_start': 0x08040000, 'file_offset': 0x40000, 'size': 0xC0000,
            'name': 'Firmware',   'erase_cmd': 'P F *CDX 0000'},
    'CMX': {'flash_start': 0x08004000, 'file_offset': 0x04000, 'size': 0xFC000,
            'name': 'Config+FW',  'erase_cmd': 'P F *CMX 0000'},
}

FULL_IMAGE_SIZE = 0x100000   # 1MB

BLOCK_ALIASES = {
    'bootloader': 'BLX', 'boot': 'BLX', 'blx': 'BLX',
    'config': 'CCX', 'conf': 'CCX', 'ccx': 'CCX',
    'firmware': 'CDX', 'fw': 'CDX', 'cdx': 'CDX',
    'all': 'CMX', 'cmx': 'CMX',
}


BDD_RATES = {57600: '0000', 115200: '0001', 460800: '0002'}
BDD_RATES_ORDERED = [460800, 115200, 57600]
#PROBE_RATES = [9600, 19200, 57600, 115200, 460800]
PROBE_RATES = [57600, 115200, 460800]


def crc16_ccitt(data: bytes) -> int:
    crc = 0xFFFF
    for b in data:
        crc ^= b << 8
        for _ in range(8):
            crc = ((crc << 1) ^ 0x1021 if crc & 0x8000 else crc << 1) & 0xFFFF
    return crc

def escape_payload(data: bytes) -> bytes:
    return data.replace(b'U', b'UU')

def build_frame(frame_type: str, payload: bytes) -> bytes:
    escaped = escape_payload(payload)
    frame_len = 1 + 1 + 3 + len(escaped) + 4
    pre_crc = b'U' + frame_type.encode() + f'{frame_len:03X}'.encode() + escaped
    crc = crc16_ccitt(pre_crc)
    return pre_crc + f'{crc:04X}'.encode()

def build_q_frame(cmd: str) -> bytes:
    return build_frame('Q', cmd.encode())

def build_f_frame(block_name: bytes, seq: int, records: bytes) -> bytes:
    return build_frame('F', block_name + b'\x00' + bytes([seq]) + records)

def build_completion_frame(block_name: bytes, seq: int) -> bytes:
    return build_frame('F', block_name + b'F' + bytes([seq]))

def build_record_03(address: int, data: bytes) -> bytes:
    length = 4 + len(data) + 1
    return bytes([0x03, length]) + struct.pack('>I', address) + data + bytes([0])

def parse_responses(data: bytes) -> list:
    responses = []
    i = 0
    while i < len(data):
        if data[i:i+1] == b'U' and i+1 < len(data) and data[i+1:i+2] != b'U':
            try:
                ft = chr(data[i+1])
                if ft in 'EFKLPQR':
                    length = int(data[i+2:i+5], 16)
                    if i + length <= len(data):
                        raw_payload = data[i+5:i+length-4]
                        payload = raw_payload.replace(b'UU', b'U')
                        responses.append({'type': ft, 'payload': payload})
                        i += length
                        continue
            except (ValueError, IndexError):
                pass
        i += 1
    return responses

def read_responses(ser, timeout=1.0):
    old_timeout = ser.timeout
    ser.timeout = 0.02
    data = b''
    deadline = time.time() + timeout
    while time.time() < deadline:
        chunk = ser.read(4096)
        if chunk:
            data += chunk
            # Got data but do one more quick read for any trailing bytes
            trail_deadline = time.time() + 0.02
            while time.time() < trail_deadline:
                chunk = ser.read(4096)
                if chunk:
                    data += chunk
                    trail_deadline = time.time() + 0.02  # extend if still coming
            break
    ser.timeout = old_timeout
    return data, parse_responses(data)

def send_cmd(ser, cmd_str, timeout=2.0, quiet=False):
    ser.reset_input_buffer()
    ser.write(build_q_frame(cmd_str))
    ser.flush()
    raw, responses = read_responses(ser, timeout=timeout)
    if not quiet:
        for r in responses:
            print(f"  [{r['type']}] {r['payload'].decode('ascii', errors='replace')}")
    return raw, responses

def sync_uart(ser):
    ser.reset_input_buffer()
    ser.write(b'\x55' * 128)
    ser.flush()
    time.sleep(0.05)
    ser.reset_input_buffer()


def probe_baud(ser):
    for rate in PROBE_RATES:
        ser.baudrate = rate
        ser.reset_input_buffer()
        time.sleep(0.05)
        _, resp = send_cmd(ser, "G S #BID", timeout=0.3, quiet=True)
        if any(b'BID' in r['payload'] for r in resp):
            return rate
    return None

def switch_baud(ser, target, quiet=False):
    if target not in BDD_RATES or ser.baudrate == target:
        return ser.baudrate == target
    old = ser.baudrate
    if not quiet:
        print(f"[*] Switching baud: {old} -> {target} (BDD {BDD_RATES[target]})...")
    ser.reset_input_buffer()
    ser.write(build_q_frame(f"P S #BDD {BDD_RATES[target]}"))
    ser.flush()
    time.sleep(0.3)
    read_responses(ser, timeout=0.5)  # consume ACK at old baud
    ser.baudrate = target
    sync_uart(ser)
    _, resp = send_cmd(ser, "G S #BID", timeout=0.5, quiet=True)
    if any(b'BID' in r['payload'] for r in resp):
        if not quiet:
            print(f"[+] Running at {target} baud")
        return True
    if not quiet:
        print(f"[!] No response at {target}, reverting")
    ser.baudrate = old
    sync_uart(ser)
    return False

def negotiate_best_baud(ser):
    for rate in BDD_RATES_ORDERED:
        if rate == ser.baudrate:
            return rate
        if switch_baud(ser, rate):
            return rate
    return ser.baudrate


def enter_bootloader(ser, max_retries=3):
    bid_frame = build_q_frame("G S #BID")
    preamble = b'\x55' * 128
    for retry in range(max_retries):
        if retry > 0:
            print(f"[*] Retry {retry}/{max_retries-1}...")
            time.sleep(1.0)
        ser.reset_input_buffer()
        print("[*] Checking device...")
        _, resp = send_cmd(ser, "G S #BID", timeout=1.0)
        if not resp:
            continue
        print("[*] Triggering reboot...")
        ser.reset_input_buffer()
        ser.write(build_q_frame("P S #RES 0001"))
        ser.flush()
        time.sleep(0.05)
        ser.reset_input_buffer()
        print("[*] Flooding to catch bootloader...")
        t0 = time.time()
        for _ in range(300):
            ser.write(preamble + bid_frame)
            _, responses = read_responses(ser, timeout=0.05)
            if any(b'BID' in r['payload'] for r in responses):
                print(f"[+] Bootloader caught at t+{time.time()-t0:.2f}s")
                time.sleep(0.2)
                ser.reset_input_buffer()
                return True
        print("[!] Failed to catch bootloader")
    return False

def wait_for_erase(ser, block_id, timeout=30.0):
    print("[*] Waiting for erase completion...")
    t0 = time.time()
    p_count = 0
    while time.time() - t0 < timeout:
        _, responses = read_responses(ser, timeout=0.5)
        for r in responses:
            p = r['payload'].decode('ascii', errors='replace')
            if r['type'] == 'P':
                p_count += 1
                sys.stdout.write(f"\r    Erase progress: {p_count} ACKs...")
                sys.stdout.flush()
            elif r['type'] == 'R':
                print(f"\r    Erase done [{r['type']}]: {p} ({p_count} ACKs)          ")
                return True
            else:
                print(f"\n    [{r['type']}] {p}")
    print(f"\n[!] Erase timeout ({p_count} ACKs)")
    return False


def check_block_crc(data: bytes, block_id: str) -> tuple:
    """Check CRC of a block. Returns (stored_crc, computed_crc, match)."""
    stored = (data[-2] << 8) | data[-1]
    computed = crc16_ccitt(data[:-2])
    return stored, computed, stored == computed

def fix_block_crc(data: bytearray, block_id: str) -> int:
    """Recalculate and patch CRC in last 2 bytes. Returns new CRC."""
    crc = crc16_ccitt(data[:-2])
    data[-2] = (crc >> 8) & 0xFF
    data[-1] = crc & 0xFF
    return crc


SIZE_TO_BLOCK = {
    0x04000: 'BLX',
    0x3C000: 'CCX',
    0xC0000: 'CDX',
    0xFC000: 'CMX',
}

def detect_input(file_data: bytes, block_arg: str, include_bootloader: bool):
    """
    Determine what to flash based on file size and --block argument.
    Returns list of (block_id, data_bytes, flash_start_addr) tuples.
    """
    fsize = len(file_data)
    is_full_image = (fsize == FULL_IMAGE_SIZE)

    # If --block specified, resolve it
    if block_arg:
        block_id = BLOCK_ALIASES.get(block_arg.lower())
        if not block_id:
            print(f"[!] Unknown block '{block_arg}'. Use: config, firmware, all, bootloader")
            return None
        blk = BLOCKS[block_id]
        if is_full_image:
            data = file_data[blk['file_offset']:blk['file_offset'] + blk['size']]
        elif fsize == blk['size']:
            data = file_data
        else:
            print(f"[!] File size {fsize} doesn't match {block_id} ({blk['size']}) or full image ({FULL_IMAGE_SIZE})")
            return None
        if block_id == 'BLX' and not include_bootloader:
            print("[!] Bootloader flash requires --include-bootloader")
            return None
        return [(block_id, bytearray(data), blk['flash_start'])]

    # Auto-detect from file size
    if is_full_image:
        # Default: flash config + firmware (CMX), skip bootloader
        blk = BLOCKS['CMX']
        data = file_data[blk['file_offset']:blk['file_offset'] + blk['size']]
        return [('CMX', bytearray(data), blk['flash_start'])]
    elif fsize in SIZE_TO_BLOCK:
        block_id = SIZE_TO_BLOCK[fsize]
        blk = BLOCKS[block_id]
        if block_id == 'BLX' and not include_bootloader:
            print("[!] Bootloader flash requires --include-bootloader")
            return None
        return [(block_id, bytearray(file_data), blk['flash_start'])]
    else:
        print(f"[!] Unknown file size {fsize}. Use --block to specify target.")
        print(f"    Known sizes: 1MB (full), 240KB (config), 768KB (firmware), 1008KB (config+fw)")
        return None


CHUNK_SIZE = 250

def flash_block(ser, block_id, data, flash_start, dry_run=False):
    """Erase and flash a single block. Returns True on success."""
    blk = BLOCKS[block_id]
    block_name = block_id.encode()

    # Trim trailing 0xFF
    data_end = len(data)
    while data_end > 0 and data[data_end-1] == 0xFF:
        data_end -= 1
    if data_end == 0:
        print(f"[!] {block_id} data is all 0xFF, nothing to write")
        return True
    data_end = (data_end + 3) & ~3

    print(f"\n{'='*60}")
    print(f"  Block: {block_id} ({blk['name']})")
    print(f"  Flash: 0x{flash_start:08X} — 0x{flash_start + len(data):08X}")
    print(f"  Data:  {data_end:,} bytes (trimmed from {len(data):,})")
    print(f"{'='*60}")

    if dry_run:
        print("  [DRY RUN] Would erase and flash this block")
        return True

    # ERASE
    for attempt in range(3):
        print(f"\n[*] Erasing {block_id} (attempt {attempt+1})...")
        ser.reset_input_buffer()
        ser.write(build_q_frame(blk['erase_cmd']))
        ser.flush()
        if wait_for_erase(ser, block_id, timeout=30.0):
            break
        elif attempt < 2:
            print("[!] Erase stalled, retrying...")
            time.sleep(1.0)
        else:
            print("[!] Erase failed")
            return False

    # WRITE
    seq = 0
    offset = 0
    frame_count = 0
    t0 = time.time()

    print(f"\n[*] Writing {data_end:,} bytes @ {ser.baudrate} baud...")

    while offset < data_end:
        chunk = data[offset:offset + CHUNK_SIZE]
        if not chunk:
            break
        address = flash_start + offset
        f_frame = build_f_frame(block_name, seq, build_record_03(address, bytes(chunk)))
        ser.write(f_frame)
        frame_count += 1
        offset += len(chunk)
        seq = (seq + 1) & 0xFF

        if frame_count % 20 == 0:
            ser.flush()
            elapsed = time.time() - t0
            pct = offset / data_end * 100
            rate = offset / elapsed if elapsed > 0 else 0
            eta = (data_end - offset) / rate if rate > 0 else 0
            sys.stdout.write(
                f"\r    {offset:,}/{data_end:,} ({pct:.0f}%) "
                f"[{rate/1024:.1f} KB/s] ETA {eta:.0f}s"
            )
            sys.stdout.flush()
            time.sleep(0.01)
            _, responses = read_responses(ser, timeout=0.01)
            for r in responses:
                print(f"\n    [{r['type']}] {r['payload'].decode('ascii', errors='replace')}")

    ser.flush()
    elapsed = time.time() - t0
    print(f"\r    {offset:,}/{data_end:,} (100%) in {elapsed:.1f}s, {frame_count} frames          ")

    # Completion frame
    print("[*] Sending completion frame...")
    ser.write(build_completion_frame(block_name, seq))
    ser.flush()
    time.sleep(0.5)
    _, responses = read_responses(ser, timeout=1.0)
    for r in responses:
        print(f"    [{r['type']}] {r['payload'].decode('ascii', errors='replace')}")

    return True


def cmd_info(ser):
    print("\n[*] Probing device...")
    baud = probe_baud(ser)
    if not baud:
        print("[!] Device not responding")
        return 1
    ser.baudrate = baud
    print(f"[+] Device responding at {baud} baud")
    print("\n[*] Device info:")
    for cmd in ["G S #BID", "G S #SID", "G S #CID", "G S #PCB",
                "G S #SRN", "G S #PCD", "G S #PNA"]:
        send_cmd(ser, cmd, timeout=0.5)
    return 0


def main():
    parser = argparse.ArgumentParser(
        description='ResMed AirSense UART Flash Tool',
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog="""
Blocks:
  config (CCX)      Configuration data (240KB)
  firmware (CDX)    Application firmware (768KB)
  all (CMX)         Config + Firmware (1008KB)
  bootloader (BLX)  Bootloader (16KB, requires --include-bootloader)

Examples:
  %(prog)s -p /dev/ttyACM0 -f dump.bin                    Flash config+firmware
  %(prog)s -p /dev/ttyACM0 -f dump.bin --block config      Flash config only
  %(prog)s -p /dev/ttyACM0 -f config.bin                   Auto-detect 240KB config
  %(prog)s -p /dev/ttyACM0 -f dump.bin --fix-crc           Fix CRC before flash
  %(prog)s -p /dev/ttyACM0 -f dump.bin --dry-run           Validate without flashing
  %(prog)s -p /dev/ttyACM0 --info                          Show device info
""")
    parser.add_argument('-p', '--port', required=True, help='Serial port')
    parser.add_argument('-f', '--file', help='Firmware file to flash')
    parser.add_argument('--block', help='Target block: config, firmware, all, bootloader')
    parser.add_argument('--baud', default='auto', help='Transfer baud: auto, 57600, 115200, 460800')
    parser.add_argument('--fix-crc', action='store_true', help='Recalculate and patch CRC')
    parser.add_argument('--force', action='store_true', help='Flash even with bad CRC')
    parser.add_argument('--include-bootloader', action='store_true', help='Allow bootloader writes')
    parser.add_argument('--dry-run', action='store_true', help='Validate only, do not flash')
    parser.add_argument('--no-reset', action='store_true', help='Do not reset device after flash')
    parser.add_argument('--no-enter', action='store_true', help='Skip bootloader entry')
    parser.add_argument('--info', action='store_true', help='Show device info and exit')
    args = parser.parse_args()

    ser = serial.Serial(args.port, 57600, timeout=1.0)

    try:
        if args.info:
            return cmd_info(ser)

        if not args.file:
            parser.error("-f/--file is required (unless using --info)")

        with open(args.file, 'rb') as f:
            file_data = f.read()
        print(f"[*] Loaded: {args.file} ({len(file_data):,} bytes)")

        # detect blocks
        jobs = detect_input(file_data, args.block, args.include_bootloader)
        if not jobs:
            return 1

        print(f"\n[*] Flash plan:")
        for block_id, data, flash_start in jobs:
            blk = BLOCKS[block_id]
            print(f"    {block_id} ({blk['name']}): {len(data):,} bytes @ 0x{flash_start:08X}")


        print(f"\n[*] CRC validation:")
        crc_ok = True
        for block_id, data, _ in jobs:
            if block_id == 'CMX':
                # CMX spans CCX + CDX, check both sub-block CRCs
                sub_blocks = [
                    ('CCX', data[:BLOCKS['CCX']['size']]),
                    ('CDX', data[BLOCKS['CDX']['file_offset'] - BLOCKS['CMX']['file_offset']:]),
                ]
            else:
                sub_blocks = [(block_id, data)]

            for sub_id, sub_data in sub_blocks:
                stored, computed, match = check_block_crc(sub_data, sub_id)
                status = "OK" if match else "MISMATCH"
                icon = "+" if match else "!"
                print(f"    [{icon}] {sub_id}: stored=0x{stored:04X} computed=0x{computed:04X} {status}")

                if not match:
                    if args.fix_crc:
                        new_crc = fix_block_crc(sub_data, sub_id)
                        print(f"        Fixed CRC -> 0x{new_crc:04X}")
                    elif not args.force:
                        crc_ok = False

        if not crc_ok:
            print("\n[!] CRC mismatch. Use --fix-crc to repair or --force to ignore.")
            return 1

        if args.dry_run:
            print("\n[DRY RUN] Validation complete. No changes made.")
            for block_id, data, flash_start in jobs:
                flash_block(ser, block_id, data, flash_start, dry_run=True)
            return 0

        # baud auto or constant
        if args.baud == 'auto':
            print(f"\n[*] Probing device...")
            baud = probe_baud(ser)
            if not baud:
                print("[!] Device not responding at any known baud rate")
                return 1
            print(f"[+] Device responding at {baud} baud")
            ser.baudrate = baud
        else:
            init_baud = int(args.baud)
            ser.baudrate = init_baud
            print(f"\n[*] Connecting at {init_baud} baud...")
            _, resp = send_cmd(ser, "G S #BID", timeout=1.0, quiet=True)
            if not any(b'BID' in r['payload'] for r in resp):
                print(f"[!] No response at {init_baud} baud")
                return 1
            print(f"[+] Device responding at {init_baud} baud")

        if not args.no_enter:
            if ser.baudrate != 57600:
                switch_baud(ser, 57600)
            if not enter_bootloader(ser):
                return 1

        if args.baud == 'auto':
            print("\n[*] Negotiating baud rate...")
            negotiate_best_baud(ser)
        else:
            target = int(args.baud)
            if target != ser.baudrate:
                switch_baud(ser, target)

        for block_id, data, flash_start in jobs:
            if not flash_block(ser, block_id, data, flash_start):
                print(f"\n[!] Failed to flash {block_id}")
                return 1

        if not args.no_reset:
            if ser.baudrate != 57600:
                switch_baud(ser, 57600)
            print("\n[*] Resetting device...")
            send_cmd(ser, "P S #RES 0001", timeout=2.0)

        print("\n[+] Flash complete!")
        return 0

    finally:
        ser.close()

if __name__ == '__main__':
    sys.exit(main())
