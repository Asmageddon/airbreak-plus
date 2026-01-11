# flash_patch.tcl — STM32 flash patch helpers (OpenOCD)
#
# Purpose:
#   Safely patch STM32 internal flash at runtime via OpenOCD.
#   Supports byte-level patches, u32/float writes, sector-safe updates,
#   optional verification, and watchdog-safe operation.
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
#   patch::write_u32 <offset|addr> <value> [verify]
#     Write a 32-bit unsigned integer (little-endian).
#
#   patch::write_float <offset|addr> <value> [verify]
#     Write a 32-bit IEEE754 float.
#
# Read helpers:
#   patch::read <offset|addr> <len>
#     Read raw bytes.
#
#   patch::read_u32 <offset|addr>
#     Read a 32-bit unsigned integer.
#
#   patch::read_float <offset|addr>
#     Read a 32-bit float.
#
#   patch::hexdump <offset|addr> <len> [-p] [-w N]
#     Hex dump (default width = 16 bytes, address prefix enabled).
#
# Watchdog handling:
#   - Automatically detects IWDG state via option byte
#   - Disables watchdog only if needed
#   - Restores original state after patching
#
# Sector handling:
#   - Cross-sector safe
#   - Sector cache avoids unnecessary erase/write cycles
#
# Examples:
#   patch::hexstr 0x5014 e10500003200
#   patch::write_u32 0x5014 1500
#   patch::hexdump 0x5000 64
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
    # STM32F4 sector layout
    # ------------------------------------------------------------
    proc _sector_start {a} {
        if {$a < 0x08004000} { return 0x08000000 }
        if {$a < 0x08008000} { return 0x08004000 }
        if {$a < 0x0800C000} { return 0x08008000 }
        if {$a < 0x08010000} { return 0x0800C000 }
        if {$a < 0x08020000} { return 0x08010000 }
        return [expr {0x08020000 + ((($a - 0x08020000) / 0x20000) * 0x20000)}]
    }

    proc _sector_size {a} {
        if {$a < 0x08010000} { return 0x4000 }    ;# 16 KiB
        if {$a < 0x08020000} { return 0x10000 }   ;# 64 KiB
        return 0x20000                            ;# 128 KiB
    }

    proc _sectors_for_range {addr len} {
        set out {}
        if {$len <= 0} { return $out }
        set end [expr {$addr + $len - 1}]
        set cur $addr
        while {$cur <= $end} {
            set s [_sector_start $cur]
            set sz [_sector_size $cur]
            lappend out [list $s $sz]
            set cur [expr {$s + $sz}]
        }
        return $out
    }

    # ------------------------------------------------------------
    # Deterministic watchdog handling
    # ------------------------------------------------------------
    proc _iwdg_is_enabled {} {
        # USER option byte: bit0 = IWDG_SW (0 = HW watchdog enabled)
        set r [stm32f2x options_read 0]
        if {![regexp {0x([0-9A-Fa-f]{2})} $r -> hex]} {
            error "cannot parse USER option byte: $r"
        }
        scan $hex %x opt
        return [expr {(($opt & 1) == 0)}]
    }

    proc _iwdg_disable {} {
        reset halt
        stm32f2x options_write 0 0x2c
        sleep 100
        reset halt
    }

    proc _iwdg_enable {} {
        reset halt
        stm32f2x options_write 0 0xcc
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

    proc read_u32 {off} {
        set b [read $off 4]
        expr {([lindex $b 0]) |
              ([lindex $b 1] << 8) |
              ([lindex $b 2] << 16) |
              ([lindex $b 3] << 24)}
    }

    proc read_float {off} {
        binary scan [binary format I [read_u32 $off]] f f
        return $f
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

    proc write_u32 {off v {verify 0}} {
        set v [_to_u32 $v]
        bytes $off [list \
            [expr {$v & 0xFF}] \
            [expr {($v >> 8) & 0xFF}] \
            [expr {($v >> 16) & 0xFF}] \
            [expr {($v >> 24) & 0xFF}]] $verify
    }

    proc write_float {off f {verify 0}} {
        binary scan [binary format f $f] I u
        write_u32 $off $u $verify
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
        echo {  patch::write_u32 <off|addr> <value> [verify]}
        echo {  patch::write_float <off|addr> <value> [verify]}
        echo ""
        echo {Read / inspect:}
        echo {  patch::read <off|addr> <len>}
        echo {  patch::read_u32 <off|addr>}
        echo {  patch::read_float <off|addr>}
        echo {  patch::hexdump <off|addr> <len> [-p] [-w N]}
        echo ""
        echo {Guard:}
        echo "  patch::with_guard {"
        echo {      <patch::* calls>}
        echo "  }"
        echo ""
    }
}

patch::help

