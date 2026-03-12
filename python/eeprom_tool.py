#!/usr/bin/env python3
"""
AirSense 10 / AirCurve 10 EEPROM Tool

Binary protocol host for the EEPROM tool CDX stub firmware.
Supports baud rate negotiation, full read/write/erase, header CRC fix,
and optional FAT filesystem operations.

"""

import serial
import sys
import time
import os
import struct
import argparse
import io
import warnings
from dataclasses import dataclass
from typing import Optional

def eprint(*args, **kwargs):
    print(*args, file=sys.stderr, **kwargs)


warnings.filterwarnings(
    "ignore", message="pkg_resources is deprecated as an API.*",
    category=UserWarning,
)
warnings.filterwarnings(
    "ignore", message="One or more FATs differ, filesystem most likely corrupted.*",
    category=UserWarning,
)
warnings.filterwarnings(
    "ignore", message="Filesystem was not cleanly unmounted.*",
    category=UserWarning,
)


PROTO_SYNC  = 0x55

CMD_PING     = 0x01
CMD_READ     = 0x02
CMD_WRITE    = 0x03
CMD_ERASE    = 0x04
CMD_FIX_CRC  = 0x05
CMD_SET_BAUD = 0x06
CMD_RESET      = 0x07
CMD_WRITE_BULK = 0x08

RSP_ACK  = 0x41
RSP_NACK = 0x4E
RSP_DATA = 0x44

BULK_ACK       = 0x06
BULK_ERR_CRC   = 0x10
BULK_ERR_WRITE = 0x11
BULK_ERR_NAMES = {BULK_ERR_CRC: "CRC", BULK_ERR_WRITE: "WRITE_VERIFY"}

ERR_NAMES = {
    0x01: "BAD_CRC", 0x02: "BAD_CMD", 0x03: "BAD_RANGE",
    0x04: "BAD_LEN", 0x05: "VERIFY",  0x06: "BAD_BAUD",
}

EEPROM_SIZE   = 256 * 1024
PAGE_SIZE     = 256
DEFAULT_BAUD  = 57_600
BAUD_STEPS    = [57_600, 115_200, 460_800, 1_000_000, 2_000_000]
FRAME_TIMEOUT = 5.0

FAT_BASE_ADDR   = 0x200
FAT_SECTOR_SIZE = 512



def crc16_ccitt(data: bytes, crc: int = 0xFFFF) -> int:
    for b in data:
        crc ^= b << 8
        for _ in range(8):
            crc = ((crc << 1) ^ 0x1021) & 0xFFFF if crc & 0x8000 else (crc << 1) & 0xFFFF
    return crc



class EepromProto:
    """Binary protocol handler"""

    def __init__(self, port: str, baud: int = DEFAULT_BAUD,
                 timeout: float = FRAME_TIMEOUT):
        self.ser = serial.Serial(
            port=port, baudrate=baud,
            timeout=timeout, write_timeout=timeout,
        )
        self.baud = baud
        time.sleep(0.1)
        self.ser.reset_input_buffer()
        self.ser.reset_output_buffer()

    def close(self):
        self.ser.close()

    def __enter__(self):
        return self

    def __exit__(self, *args):
        self.close()

    def _send_frame(self, cmd: int, payload: bytes = b""):
        length = len(payload)
        header = bytes([cmd, (length >> 8) & 0xFF, length & 0xFF])
        body = header + payload
        crc = crc16_ccitt(body)
        frame = bytes([PROTO_SYNC]) + body + bytes([(crc >> 8) & 0xFF, crc & 0xFF])
        self.ser.write(frame)
        self.ser.flush()

    def _recv_frame(self, timeout: Optional[float] = None) -> tuple[int, bytes]:
        old_timeout = self.ser.timeout
        if timeout is not None:
            self.ser.timeout = timeout
        try:
            while True:
                b = self.ser.read(1)
                if not b:
                    raise RuntimeError("Timeout waiting for response sync")
                if b[0] == PROTO_SYNC:
                    break

            hdr = self.ser.read(3)
            if len(hdr) < 3:
                raise RuntimeError("Timeout reading response header")

            rsp_type = hdr[0]
            length = (hdr[1] << 8) | hdr[2]

            payload = self.ser.read(length) if length > 0 else b""
            if len(payload) < length:
                raise RuntimeError(f"Short payload: {len(payload)}/{length}")

            crc_bytes = self.ser.read(2)
            if len(crc_bytes) < 2:
                raise RuntimeError("Timeout reading response CRC")

            crc_rx = (crc_bytes[0] << 8) | crc_bytes[1]
            crc_calc = crc16_ccitt(hdr + payload)
            if crc_rx != crc_calc:
                raise RuntimeError(
                    f"Response CRC mismatch: 0x{crc_rx:04X} vs 0x{crc_calc:04X}")

            return rsp_type, payload
        finally:
            self.ser.timeout = old_timeout

    def _expect_ack(self, timeout: Optional[float] = None) -> bytes:
        rsp_type, payload = self._recv_frame(timeout=timeout)
        if rsp_type == RSP_NACK:
            if len(payload) >= 7 and payload[0] == 0x05:  # ERR_VERIFY detail
                off = (payload[1] << 8) | payload[2]
                exp, got = payload[3], payload[4]
                dlen = (payload[5] << 8) | payload[6]
                raise RuntimeError(
                    f"Device NACK: VERIFY mismatch at offset {off} "
                    f"(wrote 0x{exp:02X}, read 0x{got:02X}, len={dlen})")
            err = payload[0] if payload else 0xFF
            raise RuntimeError(f"Device NACK: {ERR_NAMES.get(err, f'0x{err:02X}')}")
        if rsp_type != RSP_ACK:
            raise RuntimeError(f"Unexpected response: 0x{rsp_type:02X}")
        return payload


    def ping(self) -> str:
        self._send_frame(CMD_PING)
        rsp_type, payload = self._recv_frame()
        if rsp_type == RSP_DATA:
            return payload.decode("ascii", errors="replace")
        raise RuntimeError(f"Unexpected ping response: 0x{rsp_type:02X}")

    def read(self, addr: int, length: int) -> bytes:
        payload = bytes([
            (addr >> 16) & 0xFF, (addr >> 8) & 0xFF, addr & 0xFF,
            (length >> 16) & 0xFF, (length >> 8) & 0xFF, length & 0xFF,
        ])
        self._send_frame(CMD_READ, payload)

        ack_payload = self._expect_ack()
        if len(ack_payload) == 3:
            confirmed = (ack_payload[0] << 16) | (ack_payload[1] << 8) | ack_payload[2]
            if confirmed != length:
                raise RuntimeError(f"Length mismatch: {length} vs {confirmed}")

        total = length + 2
        old_timeout = self.ser.timeout
        self.ser.timeout = max(old_timeout, length / (self.baud / 10) * 2 + 2)
        try:
            data = bytearray()
            remaining = total
            while remaining > 0:
                chunk = self.ser.read(min(remaining, 4096))
                if not chunk:
                    raise RuntimeError(
                        f"Timeout during read stream ({len(data)}/{total})")
                data.extend(chunk)
                remaining -= len(chunk)
        finally:
            self.ser.timeout = old_timeout

        eeprom_data = bytes(data[:length])
        crc_rx = (data[length] << 8) | data[length + 1]
        if crc_rx != crc16_ccitt(eeprom_data):
            raise RuntimeError("Read data CRC mismatch")
        return eeprom_data

    def write_page(self, addr: int, data: bytes):
        payload = bytes([
            (addr >> 16) & 0xFF, (addr >> 8) & 0xFF, addr & 0xFF,
        ]) + data
        self._send_frame(CMD_WRITE, payload)
        self._expect_ack(timeout=2.0)

    def write_bulk(self, addr: int, image: bytes, progress=None):
        """Streaming page write. Sends CMD_WRITE_BULK header, then raw
        page+CRC packets with single-byte ACK per page."""
        if len(image) % PAGE_SIZE:
            raise ValueError("Image size must be multiple of page size")
        count = len(image) // PAGE_SIZE
        if addr % PAGE_SIZE:
            raise ValueError("Start address must be page-aligned")

        payload = bytes([
            (addr >> 16) & 0xFF, (addr >> 8) & 0xFF, addr & 0xFF,
            (count >> 8) & 0xFF, count & 0xFF,
        ])
        self._send_frame(CMD_WRITE_BULK, payload)
        self._expect_ack(timeout=2.0)

        old_timeout = self.ser.timeout
        self.ser.timeout = 2.0  # per-page: write + verify < 10ms typ

        for i in range(count):
            page = image[i * PAGE_SIZE:(i + 1) * PAGE_SIZE]
            crc = crc16_ccitt(page)
            self.ser.write(page + bytes([(crc >> 8) & 0xFF, crc & 0xFF]))
            self.ser.flush()

            resp = self.ser.read(1)
            if not resp:
                self.ser.timeout = old_timeout
                raise RuntimeError(f"Timeout on page {i} (0x{addr + i * PAGE_SIZE:05X})")
            if resp[0] != BULK_ACK:
                # Error: read remaining 6 bytes of detail packet
                detail = self.ser.read(6)
                self.ser.timeout = old_timeout
                code = resp[0]
                if len(detail) == 6:
                    pg = (detail[0] << 8) | detail[1]
                    if code == BULK_ERR_CRC:
                        crc_rx = (detail[2] << 8) | detail[3]
                        crc_exp = (detail[4] << 8) | detail[5]
                        raise RuntimeError(
                            f"Bulk CRC error at page {pg} (0x{addr + pg * PAGE_SIZE:05X}): "
                            f"rx=0x{crc_rx:04X} calc=0x{crc_exp:04X}")
                    else:
                        off = (detail[2] << 8) | detail[3]
                        exp, got = detail[4], detail[5]
                        raise RuntimeError(
                            f"Bulk verify fail at page {pg} (0x{addr + pg * PAGE_SIZE:05X}), "
                            f"offset {off}: wrote 0x{exp:02X}, read 0x{got:02X}")
                name = BULK_ERR_NAMES.get(code, f"0x{code:02X}")
                raise RuntimeError(
                    f"Bulk write failed at page {i} (0x{addr + i * PAGE_SIZE:05X}): {name}")

            if progress:
                progress(i + 1, count)

        self.ser.timeout = old_timeout

    def erase(self):
        self._send_frame(CMD_ERASE)
        self._expect_ack(timeout=30.0)

    def fix_crc(self) -> tuple[int, int]:
        self._send_frame(CMD_FIX_CRC)
        rsp_type, payload = self._recv_frame()
        if rsp_type == RSP_DATA and len(payload) == 4:
            old = (payload[0] << 8) | payload[1]
            new = (payload[2] << 8) | payload[3]
            return old, new
        if rsp_type == RSP_NACK:
            err = payload[0] if payload else 0xFF
            raise RuntimeError(f"Fix CRC NACK: {ERR_NAMES.get(err, f'0x{err:02X}')}")
        raise RuntimeError(f"Unexpected response: 0x{rsp_type:02X}")

    def diag(self) -> bytes:
        self._send_frame(CMD_DIAG)
        rsp_type, payload = self._recv_frame()
        if rsp_type == RSP_DATA:
            return payload
        if rsp_type == RSP_NACK:
            err = payload[0] if payload else 0xFF
            raise RuntimeError(f"Diag NACK: {ERR_NAMES.get(err, f'0x{err:02X}')}")
        raise RuntimeError(f"Unexpected response: 0x{rsp_type:02X}")

    def set_baud(self, baud: int):
        self._send_frame(CMD_SET_BAUD, baud.to_bytes(4, "big"))
        self._expect_ack()
        time.sleep(0.05)
        self.ser.baudrate = baud
        self.baud = baud
        time.sleep(0.05)
        self.ser.reset_input_buffer()

    def reset(self):
        self._send_frame(CMD_RESET)
        try:
            self._expect_ack(timeout=1.0)
        except RuntimeError:
            pass

    def send_raw_command(self, cmd: int, payload: bytes = b"") -> tuple[int, bytes]:
        self._send_frame(cmd, payload)
        return self._recv_frame()



def connect(port: str, baud: int = None) -> EepromProto:
    """Connect with auto baud detection and negotiation."""

    # Try default first (common case), then sweep high-to-low
    probe_order = [DEFAULT_BAUD] + [b for b in reversed(BAUD_STEPS) if b != DEFAULT_BAUD]
    connected_baud = None
    proto = None

    for try_baud in probe_order:
        proto = EepromProto(port, try_baud, timeout=0.8)
        try:
            version = proto.ping()
            connected_baud = try_baud
            eprint(f"Connected: {version}" +
                  (f" (at {try_baud})" if try_baud != DEFAULT_BAUD else ""))
            break
        except Exception:
            proto.close()
            proto = None
            time.sleep(0.1)

    if proto is None:
        raise RuntimeError("No response at any baud rate")

    # negotiate to target baud.
    target = baud if baud else BAUD_STEPS[-1]

    if target == connected_baud:
        return proto

    if baud:
        candidates = [baud]
    else:
        candidates = [b for b in reversed(BAUD_STEPS) if b > connected_baud]

    for try_baud in candidates:
        try:
            proto.set_baud(try_baud)
            proto.ping()
            eprint(f"Switched to {try_baud} baud")
            return proto
        except Exception:
            try:
                proto.set_baud(DEFAULT_BAUD)
                proto.ping()
                connected_baud = DEFAULT_BAUD
            except Exception:
                proto.close()
                time.sleep(5.5)
                proto = EepromProto(port, DEFAULT_BAUD, timeout=2.0)
                try:
                    proto.ping()
                    connected_baud = DEFAULT_BAUD
                except Exception:
                    pass

    return proto


def read_eeprom(port: str, outfile: str, addr: Optional[int] = None,
                length: Optional[int] = None, baud: int = None):
    if addr is None:
        addr, length = 0, EEPROM_SIZE
    elif length is None:
        raise ValueError("If addr is specified, length must be too")
    if addr < 0 or addr + length > EEPROM_SIZE:
        raise ValueError("Range out of bounds")

    with connect(port, baud) as proto:
        eprint(f"Reading {length} bytes from 0x{addr:05X}...")
        start = time.time()
        data = proto.read(addr, length)
        elapsed = time.time() - start

    rate = len(data) / elapsed / 1024 if elapsed > 0 else 0
    if outfile == "-":
        sys.stdout.buffer.write(data)
        sys.stdout.buffer.flush()
    else:
        with open(outfile, "wb") as f:
            f.write(data)
        eprint(f"Read {len(data)} bytes in {elapsed:.2f}s ({rate:.1f} KiB/s)")
        eprint(f"Saved to: {outfile}")


def write_eeprom(port: str, infile: str, baud: int = None):
    with open(infile, "rb") as f:
        image = f.read()
    if len(image) != EEPROM_SIZE:
        raise ValueError(f"File must be {EEPROM_SIZE} bytes, got {len(image)}")

    with connect(port, baud) as proto:
        eprint(f"Writing {EEPROM_SIZE} bytes...")
        start = time.time()
        total = EEPROM_SIZE // PAGE_SIZE

        def progress(done, total):
            if done % 32 == 0 or done == total:
                eprint(f"\r  {done * 100 // total}% ({done}/{total})", end="", flush=True)

        proto.write_bulk(0, image, progress=progress)
        elapsed = time.time() - start
        rate = EEPROM_SIZE / elapsed / 1024 if elapsed > 0 else 0
        eprint(f"\nDone: {elapsed:.2f}s ({rate:.1f} KiB/s)")


def erase_eeprom(port: str, baud: int = None):
    with connect(port, baud) as proto:
        eprint("Erasing...")
        start = time.time()
        proto.erase()
        eprint(f"Done ({time.time() - start:.1f}s)")


def cmd_fixcrc(port: str, baud: int = None):
    with connect(port, baud) as proto:
        old, new = proto.fix_crc()
        eprint(f"Header CRC: 0x{old:04X} -> 0x{new:04X}")


def patch_bytes(port: str, addr: int, data: bytes, baud: int = None):
    if addr + len(data) > EEPROM_SIZE:
        raise ValueError("Patch exceeds EEPROM")
    with connect(port, baud) as proto:
        off, remaining = 0, len(data)
        while remaining > 0:
            page_off = (addr + off) & (PAGE_SIZE - 1)
            n = min(remaining, PAGE_SIZE - page_off)
            proto.write_page(addr + off, data[off:off + n])
            off += n
            remaining -= n
        eprint(f"Patched {len(data)} bytes at 0x{addr:05X}")


def read_eeprom_range(port: str, addr: int, length: int,
                      baud: int = None) -> bytes:
    with connect(port, baud) as proto:
        return proto.read(addr, length)


# FAT12 (optional, requires pyfatfs)

try:
    from pyfatfs.PyFatFS import PyFatBytesIOFS
    HAVE_PYFATFS = True
except Exception:
    PyFatBytesIOFS = None
    HAVE_PYFATFS = False

@dataclass
class FatSource:
    port: Optional[str] = None
    file: Optional[str] = None
    baud: int = None


def _fat_parse_bpb(boot):
    bps = struct.unpack_from("<H", boot, 0x0B)[0]
    ts16 = struct.unpack_from("<H", boot, 0x13)[0]
    ts32 = struct.unpack_from("<I", boot, 0x20)[0]
    if bps == 0:
        raise RuntimeError("BPB: bytes_per_sector=0")
    total = ts16 or ts32
    if total == 0:
        raise RuntimeError("BPB: total_sectors=0")
    return bps, total


def _fat_read_boot_from(reader):
    return reader(FAT_BASE_ADDR, FAT_SECTOR_SIZE)


def _fat_read_region_from(reader):
    boot = _fat_read_boot_from(reader)
    bps, total = _fat_parse_bpb(boot)
    fat_size = bps * total
    return reader(FAT_BASE_ADDR, fat_size), bps, total


def fat_read_region(src):
    if src.port:
        with connect(src.port, src.baud) as proto:
            return _fat_read_region_from(proto.read)
    def file_read(addr, length):
        with open(src.file, "rb") as f:
            f.seek(addr)
            d = f.read(length)
        if len(d) != length:
            raise RuntimeError("Short read")
        return d
    return _fat_read_region_from(file_read)


def _fat_open_fs(fat_bytes):
    fp = io.BytesIO(bytearray(fat_bytes))
    return fp, PyFatBytesIOFS(fp=fp)


def fat_ls(src, path):
    fat, _, _ = fat_read_region(src)
    fp, fs = _fat_open_fs(fat)
    if not path.startswith("/"):
        path = "/" + path
    for info in fs.scandir(path):
        kind = "d" if info.is_dir else "-"
        print(f"{kind} {info.size or 0:8d} {info.name}")
    fs.close()


def fat_get(src, fat_path, outfile):
    fat, _, _ = fat_read_region(src)
    fp, fs = _fat_open_fs(fat)
    if not fat_path.startswith("/"):
        fat_path = "/" + fat_path
    with fs.openbin(fat_path, "r") as f:
        data = f.read()
    fs.close()
    if outfile == "-":
        sys.stdout.buffer.write(data)
        sys.stdout.buffer.flush()
    else:
        with open(outfile, "wb") as f:
            f.write(data)
    eprint(f"fat-get: {fat_path} -> {outfile} ({len(data)} bytes)")


def fat_getdir(src, fat_path, outdir):
    fat, _, _ = fat_read_region(src)
    fp, fs = _fat_open_fs(fat)
    if not fat_path.startswith("/"):
        fat_path = "/" + fat_path
    os.makedirs(outdir, exist_ok=True)

    count = [0]
    def _walk(fdir, hdir):
        for e in fs.listdir(fdir):
            fp2 = fdir.rstrip("/") + "/" + e
            hp = os.path.join(hdir, e)
            if fs.getinfo(fp2).is_dir:
                os.makedirs(hp, exist_ok=True)
                _walk(fp2, hp)
            else:
                with fs.openbin(fp2, "r") as src_f, open(hp, "wb") as dst_f:
                    while True:
                        chunk = src_f.read(8192)
                        if not chunk:
                            break
                        dst_f.write(chunk)
                count[0] += 1
    try:
        _walk(fat_path, outdir)
    finally:
        fs.close()
    eprint(f"fat-getdir: {fat_path} -> {outdir}/ ({count[0]} files)")


def fat_repair_checksum(data: bytes) -> bytes:
    """Recompute trailing CRC32 checksum.

    SETTINS/*.set files end with hexstr with CRC32 of everything before them.
    """
    import binascii
    if len(data) < 9:
        raise ValueError("File too short for checksum repair")
    payload = data[:-8]
    crc = binascii.crc32(payload) & 0xFFFFFFFF
    return payload + f"{crc:08X}".encode("ascii")


def fat_put(src, infile, fat_path, fixsum=False):
    data = sys.stdin.buffer.read() if infile == "-" else open(infile, "rb").read()
    if fixsum:
        data = fat_repair_checksum(data)
        eprint(f"Checksum repaired: {data[-8:].decode('ascii')}")
    if not fat_path.startswith("/"):
        fat_path = "/" + fat_path

    if src.port:
        with connect(src.port, src.baud) as proto:
            old_fat, _, _ = _fat_read_region_from(proto.read)
            fp, fs = _fat_open_fs(old_fat)
            with fs.openbin(fat_path, "w") as f:
                f.write(data)
            new_fat = fp.getvalue()
            fs.close()
            if len(new_fat) != len(old_fat):
                raise RuntimeError("FAT size changed")

            changed = 0
            i = 0
            while i < len(new_fat):
                if new_fat[i] == old_fat[i]:
                    i += 1
                    continue
                j = i + 1
                while j < len(new_fat) and new_fat[j] != old_fat[j]:
                    j += 1
                off = i
                while off < j:
                    po = (FAT_BASE_ADDR + off) & (PAGE_SIZE - 1)
                    n = min(j - off, PAGE_SIZE - po)
                    proto.write_page(FAT_BASE_ADDR + off, new_fat[off:off + n])
                    changed += n
                    off += n
                i = j
        eprint(f"fat-put: {fat_path} ({len(data)} bytes, {changed} bytes patched)")
    else:
        old_fat, _, _ = fat_read_region(src)
        fp, fs = _fat_open_fs(old_fat)
        with fs.openbin(fat_path, "w") as f:
            f.write(data)
        new_fat = fp.getvalue()
        fs.close()
        if len(new_fat) != len(old_fat):
            raise RuntimeError("FAT size changed")
        with open(src.file, "rb") as f:
            blob = bytearray(f.read())
        blob[FAT_BASE_ADDR:FAT_BASE_ADDR + len(new_fat)] = new_fat
        with open(src.file, "wb") as f:
            f.write(blob)
        eprint(f"fat-put: {fat_path} ({len(data)} bytes) -> {src.file}")


# non-public extensions

_private_registered = False

def _try_register_private(sub):
    global _private_registered
    if _private_registered:
        return
    _private_registered = True
    try:
        import eeprom_private as priv
        if hasattr(priv, "register_commands"):
            priv.register_commands(sub)
    except ImportError:
        pass

def _try_dispatch_private(args):
    try:
        import eeprom_private as priv
        if hasattr(priv, "dispatch_command"):
            return priv.dispatch_command(args, connect)
    except ImportError:
        pass
    return False



class SmartHelpParser(argparse.ArgumentParser):
    def format_help(self):
        text = super().format_help().rstrip()
        for action in self._actions:
            if not isinstance(action, argparse._SubParsersAction):
                continue
            rows, examples = [], []
            for name, sp in action.choices.items():
                pos = [a.metavar or a.dest.upper() for a in sp._actions
                       if not a.option_strings and a.dest != "help"]
                desc = (sp.description or "").strip()
                rows.append((name, " ".join(pos), desc))
                ex = getattr(sp, "examples", None)
                if ex:
                    examples.append((name, ex))
            if rows:
                w1, w2 = max(len(r[0]) for r in rows), max(len(r[1]) for r in rows)
                text += "\n\nCommands:\n"
                for n, a, d in rows:
                    text += f"  {n.ljust(w1)}  {a.ljust(w2)}  {d}\n"
            if examples:
                text += "\nExamples:\n"
                for _, exl in examples:
                    for ex in exl:
                        text += f"  {ex}\n"
        return text + "\n"


def main():
    parser = SmartHelpParser(description="Resmed S10 EEPROM tool")
    sub = parser.add_subparsers(dest="cmd", required=True)

    common = argparse.ArgumentParser(add_help=False)
    common.add_argument("-p", "--port", help="Serial port (e.g. /dev/ttyACM0)")
    common.add_argument("--baud", type=int, default=None,
                        help="Target baud (default: auto-negotiate to highest)")

    p = sub.add_parser("ping", parents=[common],
                       help="Ping device",
                       description="Ping device and display firmware version")
    p.examples = ["eeprom_tool.py ping -p /dev/ttyACM0"]

    p = sub.add_parser("read", parents=[common],
                       help="Read EEPROM to file",
                       description="Read full EEPROM contents into a file")
    p.add_argument("outfile")
    p.add_argument("addr", nargs="?", type=lambda x: int(x, 0),
                   help="Start address (optional)")
    p.add_argument("length", nargs="?", type=lambda x: int(x, 0),
                   help="Length (optional)")
    p.examples = ["eeprom_tool.py read -p /dev/ttyACM0 dump.bin",
                  "eeprom_tool.py read -p /dev/ttyACM0 part.bin 0x10 0x100"]

    p = sub.add_parser("write", parents=[common],
                       help="Write EEPROM from file",
                       description="Write full EEPROM contents from a file")
    p.add_argument("infile")
    p.examples = ["eeprom_tool.py write -p /dev/ttyACM0 image.bin"]

    p = sub.add_parser("erase", parents=[common],
                       help="Erase entire EEPROM",
                       description="Erase entire EEPROM contents")
    p.examples = ["eeprom_tool.py erase -p /dev/ttyACM0"]

    p = sub.add_parser("fixcrc", parents=[common],
                       help="Recalculate and store header CRC",
                       description="Recalculate and store EEPROM header CRC")
    p.examples = ["eeprom_tool.py fixcrc -p /dev/ttyACM0"]

    p = sub.add_parser("patch", parents=[common],
                       help="Patch bytes at address",
                       description="Patch raw bytes at a given EEPROM address")
    p.add_argument("addr", type=lambda x: int(x, 0), help="Start address (hex)")
    p.add_argument("data", help="Hex byte string (e.g. '01ABFF')")
    p.examples = ["eeprom_tool.py patch -p /dev/ttyACM0 0x120 01ABFF"]

    p = sub.add_parser("reset", parents=[common],
                       help="Reset device",
                       description="Send reset command to device")
    p.examples = ["eeprom_tool.py reset -p /dev/ttyACM0"]

    if HAVE_PYFATFS:
        p = sub.add_parser("fat-ls", parents=[common],
                           help="List FAT directory",
                           description="List files in embedded FAT filesystem")
        p.add_argument("--file", help="Read from dump file instead of device")
        p.add_argument("path", nargs="?", default="/")
        p.examples = ["eeprom_tool.py fat-ls -p /dev/ttyACM0",
                      "eeprom_tool.py fat-ls --file dump.bin /SETTINGS"]

        p = sub.add_parser("fat-get", parents=[common],
                           help="Read FAT file",
                           description="Dump FAT file to stdout (-) or to an output file")
        p.add_argument("--file", help="Read from dump file instead of device")
        p.add_argument("fat_path"); p.add_argument("outfile")
        p.examples = ["eeprom_tool.py fat-get -p /dev/ttyACM0 /SETTINGS/BGL.set -",
                      "eeprom_tool.py fat-get --file dump.bin /SETTINGS/BGL.set out.bin"]

        p = sub.add_parser("fat-getdir", parents=[common],
                           help="Read FAT directory",
                           description="Dump FAT directory tree to an output path")
        p.add_argument("--file", help="Read from dump file instead of device")
        p.add_argument("fat_path"); p.add_argument("outpath")
        p.examples = ["eeprom_tool.py fat-getdir -p /dev/ttyACM0 /SETTINGS ./out/"]

        p = sub.add_parser("fat-put", parents=[common],
                           help="Write FAT file",
                           description="Upload file into embedded FAT filesystem (use '-' to read from stdin)")
        p.add_argument("--file", help="Write to dump file instead of device")
        p.add_argument("--fixsum", action="store_true",
                       help="Recompute trailing CRC32 checksum after write")
        p.add_argument("infile"); p.add_argument("fat_path")
        p.examples = ["eeprom_tool.py fat-put -p /dev/ttyACM0 in.bin /SETTINGS/BGL.set",
                      "cat in.bin | eeprom_tool.py fat-put --file dump.bin - /SETTINGS/BGL.set"]

    _try_register_private(sub)

    argv = sys.argv[1:]
    cmds = set(sub.choices.keys())
    for i, a in enumerate(argv):
        if a in cmds:
            if i > 0:
                argv = [a] + argv[:i] + argv[i+1:]
            break
    args = parser.parse_args(argv)

    def need_port():
        if not args.port:
            parser.error("-p / --port is required for this command")

    if args.cmd == "ping":
        need_port()
        with connect(args.port, args.baud) as proto:
            print(f"Device: {proto.ping()}")
    elif args.cmd == "read":
        need_port()
        read_eeprom(args.port, args.outfile, args.addr, args.length, args.baud)
    elif args.cmd == "write":
        need_port()
        write_eeprom(args.port, args.infile, args.baud)
    elif args.cmd == "erase":
        need_port()
        erase_eeprom(args.port, args.baud)
    elif args.cmd == "fixcrc":
        need_port()
        cmd_fixcrc(args.port, args.baud)
    elif args.cmd == "patch":
        need_port()
        patch_bytes(args.port, args.addr, bytes.fromhex(args.data), args.baud)
    elif args.cmd == "reset":
        need_port()
        with connect(args.port, DEFAULT_BAUD) as proto:
            proto.reset()
            eprint("Reset sent")
    elif args.cmd == "fat-ls":
        file = getattr(args, 'file', None)
        if not args.port and not file:
            parser.error("fat-ls requires -p/--port or --file")
        fat_ls(FatSource(args.port, file, args.baud), args.path)
    elif args.cmd == "fat-get":
        file = getattr(args, 'file', None)
        if not args.port and not file:
            parser.error("fat-get requires -p/--port or --file")
        fat_get(FatSource(args.port, file, args.baud), args.fat_path, args.outfile)
    elif args.cmd == "fat-getdir":
        file = getattr(args, 'file', None)
        if not args.port and not file:
            parser.error("fat-getdir requires -p/--port or --file")
        fat_getdir(FatSource(args.port, file, args.baud), args.fat_path, args.outpath)
    elif args.cmd == "fat-put":
        file = getattr(args, 'file', None)
        if not args.port and not file:
            parser.error("fat-put requires -p/--port or --file")
        fat_put(FatSource(args.port, file, args.baud), args.infile, args.fat_path,
                fixsum=getattr(args, 'fixsum', False))
    else:
        if not _try_dispatch_private(args):
            parser.error(f"Unknown: {args.cmd}")

if __name__ == "__main__":
    main()
