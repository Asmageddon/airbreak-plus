# flash_patch.tcl — flash patch helpers 
#
# Purpose:
#   Safely patch internal flash at runtime via OpenOCD.
#   Supports byte-level patches, u32/float writes, sector-safe updates,
#   optional verification, and watchdog-safe operation.
#
# Platform support:
#   Detects platform from $::_CHIPNAME:
#     stm32f1x  — S9   (density detection for ancient models) 
#     stm32f4x  — S10  (mixed sector layout)
#
#   CRC fixup (patch::fix_crc) is only supported on:
#     stm32f1x XL-density (DEV_ID 0x430)
#     stm32f4x
#
# Addressing:
#   - Values < 0x08000000 are treated as OFFSETS from flash base
#   - Flash base is assumed to be 0x08000000
#
# Core write functions:
#   patch::bytes <offset|addr> {<byte> <byte> ...} [verify]
#     Write raw byte values (0–255).
#
#   patch::hexstr <offset|addr> <hexstring> [verify]
#     Write bytes parsed from hex (e.g. "e10500003200").
#
#   patch::cesc <offset|addr> <c-escaped-string> [verify]
#     Write bytes parsed from C-style escapes (e.g. "\xE1\x05\x00").
#
# Typed helpers:
#   patch::write_u16 <offset|addr> <value> [-be] [verify]
#     Write a 16-bit unsigned integer (little-endian default, -be for big).
#
#   patch::write_u32 <offset|addr> <value> [-be] [verify]
#     Write a 32-bit unsigned integer (little-endian default, -be for big).
#
#   patch::write_float <offset|addr> <value> [verify]
#     Write a 32-bit IEEE754 float.
#
#   patch::write_string <offset|addr> <string> [-nul] [verify]
#     Write an ASCII string. -nul appends a null terminator.
#
# Read helpers:
#   patch::read <offset|addr> <len>
#     Read raw bytes.
#
#   patch::read_u16 <offset|addr> [-be]
#     Read a 16-bit unsigned integer (little-endian default, -be for big).
#
#   patch::read_u32 <offset|addr> [-be]
#     Read a 32-bit unsigned integer (little-endian default, -be for big).
#
#   patch::read_float <offset|addr>
#     Read a 32-bit float.
#
#   patch::read_string <offset|addr> <maxlen> [-nul]
#     Read ASCII string. -nul stops at first null byte.
#
#   patch::hexdump <offset|addr> <len> [-p] [-w N]
#     Hex dump (default width = 16 bytes, address prefix enabled).
#
# CRC:
#   patch::fix_crc
#     Recompute CRC-16/CCITT-FALSE for all firmware regions and write
#     the checksum words back to flash.  Detects S9/S10 automatically.
#
# Examples:
#   patch::hexstr 0x5014 e10500003200
#   patch::write_u32 0x5014 1500
#   patch::hexdump 0x5000 64
#   patch::fix_crc
#


namespace eval patch {
    # ------------------------------------------------------------
    # Globals / state
    # ------------------------------------------------------------
    variable FLASH_BASE 0x08000000
    variable _sector_cache [dict create]
    variable _guard_depth 0
    variable _guard_iwdg_was_enabled 0

    # ------------------------------------------------------------
    # Address helpers
    # ------------------------------------------------------------
    proc _abs_addr {v} {
        variable FLASH_BASE
        if {$v < $FLASH_BASE} { return [expr {$FLASH_BASE + $v}] }
        return $v
    }

    proc _to_u32 {v} { expr {$v & 0xFFFFFFFF} }


    # ------------------------------------------------------------
    # Platform-aware sector layout
    # ------------------------------------------------------------
    variable FLASH_PAGE_SIZE 0
    variable FLASH_SECTORS {}
    variable _F1_DEV_ID 0

    switch -exact -- $::_CHIPNAME {
        stm32f1x {
            # Read DBGMCU_IDCODE to determine density class
            set idcode [read_memory 0xE0042000 32 1]
            set _F1_DEV_ID [expr {[lindex $idcode 0] & 0xFFF}]

            switch -exact -- [format "0x%03x" $_F1_DEV_ID] {
                0x430 {
                    # XL-density — 2 KiB pages, up to 1 MiB
                    set FLASH_PAGE_SIZE 0x800
                }
                0x414 - 0x428 {
                    # High-density / Value high-density — 2 KiB pages
                    set FLASH_PAGE_SIZE 0x800
                }
                0x410 - 0x412 - 0x420 {
                    # Medium / Low / Value low+med — 1 KiB pages
                    set FLASH_PAGE_SIZE 0x400
                }
                default {
                    error [format "patch: unknown stm32f1x DEV_ID 0x%03X" $_F1_DEV_ID]
                }
            }

            echo [format "stm32f1x: DEV_ID 0x%03X, page size %d bytes" \
                $_F1_DEV_ID $FLASH_PAGE_SIZE]
        }
        stm32f4x {
            # S10: mixed sector geometry (bank 1)
            set FLASH_SECTORS {
                {0x08000000 0x04000}
                {0x08004000 0x04000}
                {0x08008000 0x04000}
                {0x0800C000 0x04000}
                {0x08010000 0x10000}
                {0x08020000 0x20000}
            }
        }
        default {
            error "patch: unsupported _CHIPNAME '$::_CHIPNAME'"
        }
    }

    proc _sectors_for_range {addr len} {
        variable FLASH_BASE
        variable FLASH_PAGE_SIZE
        variable FLASH_SECTORS

        set out {}
        if {$len <= 0} { return $out }

        set end [expr {$addr + $len}]
        set cur $addr

        # uniform flash (stm32f1x)
        if {$FLASH_PAGE_SIZE != 0} {
            set first [expr {
                $FLASH_BASE +
                ((($cur - $FLASH_BASE) / $FLASH_PAGE_SIZE) * $FLASH_PAGE_SIZE)
            }]
            for {set s $first} {$s < $end} {set s [expr {$s + $FLASH_PAGE_SIZE}]} {
                lappend out [list $s $FLASH_PAGE_SIZE]
            }
            return $out
        }

        # mixed flash (stm32f4x)
        while {$cur < $end} {
            set last {}
            foreach sec $FLASH_SECTORS {
                set start [lindex $sec 0]
                set size  [lindex $sec 1]
                if {$cur < $start} { break }
                set last $sec
            }

            # tail sectors repeat at last known size
            set start [lindex $last 0]
            set size  [lindex $last 1]

            if {$cur >= ($start + $size)} {
                set start [expr {
                    $start +
                    ((($cur - $start) / $size) * $size)
                }]
            }

            lappend out [list $start $size]
            set cur [expr {$start + $size}]
        }

        return $out
    }

    # ------------------------------------------------------------
    # watchdog handling
    # ------------------------------------------------------------
    proc _iwdg_is_enabled {} {
        # USER option byte: bit0 = IWDG_SW (0 = HW watchdog enabled)
        set r [$::_CHIPNAME options_read 0]
        if {![regexp {0x([0-9A-Fa-f]{2})} $r -> hex]} {
            error "cannot parse USER option byte: $r"
        }
        scan $hex %x opt
        return [expr {(($opt & 1) == 0)}]
    }

    proc _iwdg_disable {} {
        reset halt
        $::_CHIPNAME options_write 0 0x2c
        sleep 100
        reset halt
    }

    proc _iwdg_enable {} {
        reset halt
        $::_CHIPNAME options_write 0 0xcc
        reset
    }

    # ------------------------------------------------------------
    # Guard
    # ------------------------------------------------------------
    proc _guard_enter {} {
        variable _guard_depth
        variable _guard_iwdg_was_enabled

        if {$_guard_depth > 0} {
            incr _guard_depth
            return
        }

        set _guard_depth 1
        reset halt

        set _guard_iwdg_was_enabled [_iwdg_is_enabled]
        if {$_guard_iwdg_was_enabled} {
            _iwdg_disable
        }
        reset halt
    }

    proc _guard_exit {} {
        variable _guard_depth
        variable _guard_iwdg_was_enabled

        incr _guard_depth -1
        if {$_guard_depth > 0} { return }

        _flush_sector_cache

        if {$_guard_iwdg_was_enabled} {
            _iwdg_enable
        }
        set _guard_iwdg_was_enabled 0
    }

    proc with_guard {body} {
        _guard_enter
        try {
            uplevel 1 $body
        } finally {
            _guard_exit
        }
    }

    # ------------------------------------------------------------
    # Sector cache
    # ------------------------------------------------------------
    proc _cache_get {sector_start sector_size} {
        variable _sector_cache

        if {[dict exists $_sector_cache $sector_start]} {
            return [dict get $_sector_cache $sector_start]
        }

        set path [format "/tmp/patch_sector_%08X.bin" $sector_start]
        dump_image $path $sector_start $sector_size

        set f [::open $path rb]
        fconfigure $f -translation binary
        set data [::read $f]
        ::close $f

        binary scan $data c* raw
        set buf {}
        foreach b $raw { lappend buf [expr {$b & 0xFF}] }

        set entry [dict create size $sector_size path $path buf $buf dirty 0]
        dict set _sector_cache $sector_start $entry
        return $entry
    }

    proc _cache_put {sector_start entry} {
        variable _sector_cache
        dict set _sector_cache $sector_start $entry
    }

    proc _commit_sectors {sector_starts} {
        variable _sector_cache

        foreach s $sector_starts {
            if {![dict exists $_sector_cache $s]} { continue }
            set entry [dict get $_sector_cache $s]
            if {![dict get $entry dirty]} { continue }

            set size [dict get $entry size]
            set path [dict get $entry path]
            set buf  [dict get $entry buf]

            set f [::open $path wb]
            fconfigure $f -translation binary
            set bin ""
            foreach b $buf {
                append bin [binary format c [expr {$b & 0xFF}]]
            }
            ::puts -nonewline $f $bin
            ::close $f

            echo [format "erasing sector @0x%08X (%d bytes)" $s $size]
            flash erase_address $s $size
            flash write_image $path $s

            dict set entry dirty 0
            dict set _sector_cache $s $entry
        }
    }

    proc _flush_sector_cache {} {
        variable _sector_cache
        _commit_sectors [dict keys $_sector_cache]
        set _sector_cache [dict create]
    }

    # ------------------------------------------------------------
    # Read helpers
    # ------------------------------------------------------------
    proc read {off len} {
        read_memory [_abs_addr $off] 8 $len
    }

    proc read_u32 {off args} {
        set be 0
        foreach a $args {
            if {$a eq "-be"} { set be 1
            } else { error "patch::read_u32: unknown option '$a'" }
        }
        set b [read $off 4]
        if {$be} {
            expr {([lindex $b 0] << 24) |
                  ([lindex $b 1] << 16) |
                  ([lindex $b 2] << 8) |
                  [lindex $b 3]}
        } else {
            expr {[lindex $b 0] |
                  ([lindex $b 1] << 8) |
                  ([lindex $b 2] << 16) |
                  ([lindex $b 3] << 24)}
        }
    }

    proc read_float {off} {
        binary scan [binary format I [read_u32 $off]] f f
        return $f
    }

    proc read_u16 {off args} {
        set be 0
        foreach a $args {
            if {$a eq "-be"} { set be 1
            } else { error "patch::read_u16: unknown option '$a'" }
        }
        set b [read $off 2]
        if {$be} {
            expr {([lindex $b 0] << 8) | [lindex $b 1]}
        } else {
            expr {[lindex $b 0] | ([lindex $b 1] << 8)}
        }
    }

    proc read_string {off len args} {
        set nul 0
        foreach a $args {
            if {$a eq "-nul"} { set nul 1
            } else { error "patch::read_string: unknown option '$a'" }
        }
        set b [read $off $len]
        set s ""
        foreach c $b {
            if {$nul && $c == 0} { break }
            append s [format %c $c]
        }
        return $s
    }

    # ------------------------------------------------------------
    # Verification
    # ------------------------------------------------------------
    proc _verify_bytes {off bytes} {
        set got [read $off [llength $bytes]]
        for {set i 0} {$i < [llength $bytes]} {incr i} {
            set want [_to_u32 [lindex $bytes $i]]
            if {[lindex $got $i] != $want} {
                error [format "verify failed at 0x%08X (got 0x%02X want 0x%02X)" \
                    [expr {[_abs_addr $off] + $i}] \
                    [lindex $got $i] $want]
            }
        }
    }

    # ------------------------------------------------------------
    # Patch primitives
    # ------------------------------------------------------------
    proc bytes {off bytes {verify 0}} {
        _guard_enter
        try {
            set addr [_abs_addr $off]
            set len [llength $bytes]
            if {$len <= 0} { error "patch::bytes: empty byte list" }

            set touched {}

            foreach s [_sectors_for_range $addr $len] {
                lassign $s sector_start sector_size
                lappend touched $sector_start

                set entry [_cache_get $sector_start $sector_size]
                set buf [dict get $entry buf]

                set sec_end [expr {$sector_start + $sector_size - 1}]
                set w0 [expr {($addr > $sector_start) ? $addr : $sector_start}]
                set w1 [expr {(($addr + $len - 1) < $sec_end) ? ($addr + $len - 1) : $sec_end}]

                for {set a $w0} {$a <= $w1} {incr a} {
                    set bi [expr {$a - $addr}]
                    set si [expr {$a - $sector_start}]
                    lset buf $si [_to_u32 [lindex $bytes $bi]]
                }

                dict set entry buf $buf
                dict set entry dirty 1
                _cache_put $sector_start $entry
            }

            if {$verify} {
                _commit_sectors $touched
                _verify_bytes $off $bytes
            }

            echo [format "patched %d bytes @0x%08X" $len $addr]
        } finally {
            _guard_exit
        }
    }

    proc hexstr {off hex {verify 0}} {
        set parts [regexp -all -inline {[0-9A-Fa-f]{2}} $hex]

        if {[llength $parts] == 0} {
            error "patch::hexstr: no hex bytes found"
        }

        set b {}
        foreach p $parts {
            scan $p %x v
            lappend b $v
        }

        #::patch::bytes $off $b $verify
        if {[catch { ::patch::bytes $off $b $verify } err]} {
            error "patch::hexstr failed: $err"
        }
    }

    proc cesc {off cstr {verify 0}} {
        set matches [regexp -all -inline {\\x([0-9A-Fa-f]{2})} $cstr]

        if {[llength $matches] == 0} {
            error "patch::cesc: no \\xHH escapes found"
        }

        set b {}
        foreach {full hh} $matches {
            scan $hh %x v
            lappend b $v
        }

        ::patch::bytes $off $b $verify
    }

    proc write_u32 {off v args} {
        set be 0
        set verify 0
        foreach a $args {
            if {$a eq "-be"} { set be 1
            } elseif {$a eq "verify"} { set verify 1
            } else { error "patch::write_u32: unknown option '$a'" }
        }
        set v [_to_u32 $v]
        if {$be} {
            bytes $off [list \
                [expr {($v >> 24) & 0xFF}] \
                [expr {($v >> 16) & 0xFF}] \
                [expr {($v >> 8) & 0xFF}] \
                [expr {$v & 0xFF}]] $verify
        } else {
            bytes $off [list \
                [expr {$v & 0xFF}] \
                [expr {($v >> 8) & 0xFF}] \
                [expr {($v >> 16) & 0xFF}] \
                [expr {($v >> 24) & 0xFF}]] $verify
        }
    }

    proc write_float {off f {verify 0}} {
        binary scan [binary format f $f] I u
        if {$verify} {
            write_u32 $off $u verify
        } else {
            write_u32 $off $u
        }
    }

    proc write_u16 {off v args} {
        set be 0
        set verify 0
        foreach a $args {
            if {$a eq "-be"} { set be 1
            } elseif {$a eq "verify"} { set verify 1
            } else { error "patch::write_u16: unknown option '$a'" }
        }
        set v [expr {$v & 0xFFFF}]
        if {$be} {
            bytes $off [list \
                [expr {($v >> 8) & 0xFF}] \
                [expr {$v & 0xFF}]] $verify
        } else {
            bytes $off [list \
                [expr {$v & 0xFF}] \
                [expr {($v >> 8) & 0xFF}]] $verify
        }
    }

    proc write_string {off str args} {
        set nul 0
        set verify 0
        foreach a $args {
            if {$a eq "-nul"} { set nul 1
            } elseif {$a eq "verify"} { set verify 1
            } else { error "patch::write_string: unknown option '$a'" }
        }
        set b {}
        foreach c [split $str ""] {
            scan $c %c v
            lappend b [expr {$v & 0xFF}]
        }
        if {$nul} { lappend b 0 }
        bytes $off $b $verify
    }

    # ------------------------------------------------------------
    # Hexdump
    # ------------------------------------------------------------
    proc hexdump {off len args} {
        set prefix 1
        set wrap 16

        for {set i 0} {$i < [llength $args]} {incr i} {
            set a [lindex $args $i]
            if {$a eq "-p"} {
                set prefix 0
            } elseif {$a eq "-w"} {
                incr i
                set wrap [lindex $args $i]
            } else {
                error "patch::hexdump: unknown option '$a'"
            }
        }

        if {$wrap == 0} { set wrap $len }

        set addr [_abs_addr $off]
        set b [read $off $len]

        for {set j 0} {$j < $len} {incr j $wrap} {
            set line [lrange $b $j [expr {$j + $wrap - 1}]]
            set hex {}
            foreach x $line { lappend hex [format "%02X" $x] }
            if {$prefix} {
                echo [format "%08X: %s" [expr {$addr + $j}] [join $hex " "]]
            } else {
                echo [join $hex " "]
            }
        }
    }

    # ------------------------------------------------------------
    # CRC-16/CCITT-FALSE
    # ------------------------------------------------------------
    variable _CRC16_TABLE {
        0x0000 0x1021 0x2042 0x3063 0x4084 0x50A5 0x60C6 0x70E7
        0x8108 0x9129 0xA14A 0xB16B 0xC18C 0xD1AD 0xE1CE 0xF1EF
        0x1231 0x0210 0x3273 0x2252 0x52B5 0x4294 0x72F7 0x62D6
        0x9339 0x8318 0xB37B 0xA35A 0xD3BD 0xC39C 0xF3FF 0xE3DE
        0x2462 0x3443 0x0420 0x1401 0x64E6 0x74C7 0x44A4 0x5485
        0xA56A 0xB54B 0x8528 0x9509 0xE5EE 0xF5CF 0xC5AC 0xD58D
        0x3653 0x2672 0x1611 0x0630 0x76D7 0x66F6 0x5695 0x46B4
        0xB75B 0xA77A 0x9719 0x8738 0xF7DF 0xE7FE 0xD79D 0xC7BC
        0x48C4 0x58E5 0x6886 0x78A7 0x0840 0x1861 0x2802 0x3823
        0xC9CC 0xD9ED 0xE98E 0xF9AF 0x8948 0x9969 0xA90A 0xB92B
        0x5AF5 0x4AD4 0x7AB7 0x6A96 0x1A71 0x0A50 0x3A33 0x2A12
        0xDBFD 0xCBDC 0xFBBF 0xEB9E 0x9B79 0x8B58 0xBB3B 0xAB1A
        0x6CA6 0x7C87 0x4CE4 0x5CC5 0x2C22 0x3C03 0x0C60 0x1C41
        0xEDAE 0xFD8F 0xCDEC 0xDDCD 0xAD2A 0xBD0B 0x8D68 0x9D49
        0x7E97 0x6EB6 0x5ED5 0x4EF4 0x3E13 0x2E32 0x1E51 0x0E70
        0xFF9F 0xEFBE 0xDFDD 0xCFFC 0xBF1B 0xAF3A 0x9F59 0x8F78
        0x9188 0x81A9 0xB1CA 0xA1EB 0xD10C 0xC12D 0xF14E 0xE16F
        0x1080 0x00A1 0x30C2 0x20E3 0x5004 0x4025 0x7046 0x6067
        0x83B9 0x9398 0xA3FB 0xB3DA 0xC33D 0xD31C 0xE37F 0xF35E
        0x02B1 0x1290 0x22F3 0x32D2 0x4235 0x5214 0x6277 0x7256
        0xB5EA 0xA5CB 0x95A8 0x8589 0xF56E 0xE54F 0xD52C 0xC50D
        0x34E2 0x24C3 0x14A0 0x0481 0x7466 0x6447 0x5424 0x4405
        0xA7DB 0xB7FA 0x8799 0x97B8 0xE75F 0xF77E 0xC71D 0xD73C
        0x26D3 0x36F2 0x0691 0x16B0 0x6657 0x7676 0x4615 0x5634
        0xD94C 0xC96D 0xF90E 0xE92F 0x99C8 0x89E9 0xB98A 0xA9AB
        0x5844 0x4865 0x7806 0x6827 0x18C0 0x08E1 0x3882 0x28A3
        0xCB7D 0xDB5C 0xEB3F 0xFB1E 0x8BF9 0x9BD8 0xABBB 0xBB9A
        0x4A75 0x5A54 0x6A37 0x7A16 0x0AF1 0x1AD0 0x2AB3 0x3A92
        0xFD2E 0xED0F 0xDD6C 0xCD4D 0xBDAA 0xAD8B 0x9DE8 0x8DC9
        0x7C26 0x6C07 0x5C64 0x4C45 0x3CA2 0x2C83 0x1CE0 0x0CC1
        0xEF1F 0xFF3E 0xCF5D 0xDF7C 0xAF9B 0xBFBA 0x8FD9 0x9FF8
        0x6E17 0x7E36 0x4E55 0x5E74 0x2E93 0x3EB2 0x0ED1 0x1EF0
    }

    # Compute CRC-16/CCITT-FALSE over a region of flash.
    # Dumps to a temp file for performance — avoids byte-at-a-time
    # read_memory overhead on large regions
    proc _crc16_region {addr len} {
        variable _CRC16_TABLE

        if {$len <= 0} { return 0xFFFF }

        set tmp "/tmp/_patch_crc_[pid]_[clock clicks].bin"

        dump_image $tmp $addr $len

        set f [::open $tmp rb]
        fconfigure $f -translation binary
        set data [::read $f $len]
        ::close $f

        file delete -force $tmp

        binary scan $data cu* bytes

        set crc 0xFFFF
        foreach byte $bytes {
            set idx [expr {(($crc >> 8) ^ $byte) & 0xFF}]
            set crc [expr {(($crc << 8) & 0xFFFF) ^ [lindex $_CRC16_TABLE $idx]}]
        }

        return $crc
    }

    # Recompute and write CRC-16 checksums for all firmware regions.
    # Detects S9 / S10 from $::_CHIPNAME.
    proc fix_crc {} {
        variable _F1_DEV_ID

        switch -exact -- $::_CHIPNAME {
            stm32f1x {
                if {$_F1_DEV_ID != 0x430} {
                    error [format "patch::fix_crc: only XL-density (DEV_ID 0x430) supported, got 0x%03X" $_F1_DEV_ID]
                }
                # S9 regions (XL-density)
                set blocks {
                    {0x08000000 0x00003000}
                    {0x08003000 0x0001D800}
                    {0x08020800 0x000DF800}
                }
            }
            stm32f4x {
                # S10 regions
                set blocks {
                    {0x08000000 0x4000}
                    {0x08004000 0x3c000}
                    {0x08040000 0xc0000}
                }
            }
            default {
                error "patch::fix_crc: unsupported _CHIPNAME '$::_CHIPNAME'"
            }
        }

        _guard_enter
        try {
            echo "Updating CRC-16 checksums ($::_CHIPNAME)..."
            set idx 0

            foreach block $blocks {
                incr idx
                lassign $block addr len

                set payload_len [expr {$len - 2}]
                set crc [_crc16_region $addr $payload_len]
                set crc_addr [expr {$addr + $payload_len}]

                echo [format "  block %d @0x%08X (%d bytes) -> CRC 0x%04X @ 0x%08X" \
                    $idx $addr $payload_len $crc $crc_addr]

                # CRC is stored big-endian (high byte first)
                set hi [expr {($crc >> 8) & 0xFF}]
                set lo [expr {$crc & 0xFF}]
                bytes $crc_addr [list $hi $lo]
            }

            echo "CRC update complete."
        } finally {
            _guard_exit
        }
    }

    # ------------------------------------------------------------
    # Help
    # ------------------------------------------------------------
    proc help {} {
        echo {STM32 Flash Patch Helpers (::patch)}
        echo {=================================}
        echo ""
        echo {Write / patch:}
        echo {  patch::bytes <off|addr> {<byte> ...} [verify]}
        echo {  patch::hexstr <off|addr> <hex> [verify]}
        echo {  patch::cesc <off|addr> <c-escaped> [verify]}
        echo {  patch::write_u16 <off|addr> <value> [-be] [verify]}
        echo {  patch::write_u32 <off|addr> <value> [-be] [verify]}
        echo {  patch::write_float <off|addr> <value> [verify]}
        echo {  patch::write_string <off|addr> <str> [-nul] [verify]}
        echo ""
        echo {Read / inspect:}
        echo {  patch::read <off|addr> <len>}
        echo {  patch::read_u16 <off|addr> [-be]}
        echo {  patch::read_u32 <off|addr> [-be]}
        echo {  patch::read_float <off|addr>}
        echo {  patch::read_string <off|addr> <maxlen> [-nul]}
        echo {  patch::hexdump <off|addr> <len> [-p] [-w N]}
        echo ""
        echo {Guard:}
        echo "  patch::with_guard {"
        echo {      <patch::* calls>}
        echo "  }"
        echo ""
        echo {CRC:}
        echo {  patch::fix_crc    — recompute & write CRC-16 for all firmware regions}
        echo ""
    }
}

if {[info level] == 0} {
    patch::help
} else {
    echo "[info script] loaded."
    echo {    patch::help for instructions}
}
