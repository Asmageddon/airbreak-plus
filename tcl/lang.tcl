namespace eval lang {

    variable MASK_ADDR  0x2001ff3c
    variable LANG_ADDR  0x200104b6

    # Language table: id -> name
    variable LANG_NAME
    array set LANG_NAME {
        0  "English"
        1  "French"
        2  "German"
        3  "Italian"
        4  "Spanish"
        6  "Portuguese"
        8  "Dutch"
        9  "Swedish"
        10 "Danish"
        11 "Norwegian"
        12 "Finnish"
        13 "Japanese (Katakana)"
        14 "Russian"
        15 "Turkish"
        16 "Chinese (Traditional)"
        17 "Chinese (Simplified)"
        18 "Polish"
        19 "Japanese (Kanji)"
    }

    # Two-letter shortcuts -> id
    variable LANG_SHORT
    array set LANG_SHORT {
        en 0
        fr 1
        de 2
        it 3
        es 4
        pt 6
        nl 8
        sv 9
        da 10
        no 11
        fi 12
        ja 13
        jk 13
        jp 19
        ru 14
        tr 15
        zh 17
        zt 16
        pl 18
    }

    # id -> two-letter code
    variable LANG_CODE
    array set LANG_CODE {
        0  en
        1  fr
        2  de
        3  it
        4  es
        6  pt
        8  nl
        9  sv
        10 da
        11 no
        12 fi
        13 jk
        14 ru
        15 tr
        16 zt
        17 zh
        18 pl
        19 jp
    }

    # ---------------- helpers ----------------

    proc _usage {cmd} {
        switch -- $cmd {
            lang {
                return "Usage: lang::lang ?value?\n\
      Read or set current language\n\
      Value may be numeric or two-letter code\n\
    Examples:\n\
      \tlang::lang\n\
      \tlang::lang en\n\
      \tlang::lang jp"
            }
            mask {
                return "Usage: lang::mask ?value?\n\
      Read or set language enable mask\n\
      Bit 5 and bit 7 are always cleared\n\
    Special values:\n\
      \tall  -> 0x0FFF5F\n\
    Examples:\n\
      \tlang::mask\n\
      \tlang::mask en fr de\n\
      \tlang::mask 0x00040001"
            }
        }
        return ""
    }

    proc help {} {
        set out {}
        lappend out "Language control commands:"
        lappend out ""
        lappend out [_usage lang]
        lappend out ""
        lappend out [_usage mask]
        return [join $out "\n"]
    }


    proc _lang_id {v} {
        variable LANG_SHORT
        if {[string is integer -strict $v]} {
            return $v
        }
        set k [string tolower $v]
        if {[info exists LANG_SHORT($k)]} {
            return $LANG_SHORT($k)
        }
        error "Unknown language '$v'"
    }

    proc _pretty_lang {id} {
        variable LANG_NAME
        variable LANG_CODE

        if {[info exists LANG_NAME($id)]} {
            set code [expr {[info exists LANG_CODE($id)] ? $LANG_CODE($id) : "--"}]
            #return "$id $code ($LANG_NAME($id))"
            return "$id \[$code\] ($LANG_NAME($id))"
        }
        return "$id -- (Unknown)"
    }



    proc _pretty_mask {mask} {
        variable LANG_NAME
        variable LANG_CODE

        set lines {}
        lappend lines [format "%-6s %-7s %-6s %s" "bit" "state" "code" "(language)"]

        foreach id [lsort -integer [array names LANG_NAME]] {
            set bit [expr {1 << $id}]
            set state [expr {($mask & $bit) ? "on" : "off"}]
            set code  [expr {[info exists LANG_CODE($id)] ? $LANG_CODE($id) : "--"}]
            set name  $LANG_NAME($id)

            lappend lines [format "%-6d %-7s %-6s (%s)" \
                $id $state $code $name]
        }

        return [join $lines "\n"]
    }


    # ---------------- commands ----------------

    proc lang {{value ""}} {
        variable LANG_ADDR

        if {$value in {"-h" "--help" "help"}} {
            return [_usage lang]
        }

        if {$value eq ""} {
            set id [expr [mrb $LANG_ADDR]]
            return "Current language: [_pretty_lang $id]"
        } else {
            set id [_lang_id $value]
            mwb $LANG_ADDR [expr {$id & 0xFF}]
            return "Set language to: [_pretty_lang $id]"
        }
    }


    proc mask {args} {
        variable MASK_ADDR

        if {[llength $args] == 1 && [lindex $args 0] in {"-h" "--help" "help"}} {
            return [_usage mask]
        }

        if {[llength $args] == 0} {
            set v [mrw $MASK_ADDR]
            return [format "Language mask: 0x%08X\n%s" $v [_pretty_mask $v]]
        }

        # Setter
        if {[llength $args] == 1 && [string equal -nocase [lindex $args 0] "all"]} {
            set v 0x0FFF5F
        } elseif {[llength $args] == 1 && [string is integer -strict [lindex $args 0]]} {
            set v [expr {int([lindex $args 0])}]
        } else {
            set v 0
            foreach a $args {
                set id [_lang_id $a]
                set v [expr {$v | (1 << $id)}]
            }
        }

        # Clear forbidden bits
        set v [expr {$v & ~(0x20 | 0x80)}]

        mww $MASK_ADDR $v

        return [format "Set language mask: 0x%08X\nEnabled languages: %s" \
            $v [_pretty_mask $v]]
    }


}

if {[info level] == 0} {
    echo [lang::help]
} else {
    echo "[info script] loaded."
    echo {    lang::help for instructions}
}
