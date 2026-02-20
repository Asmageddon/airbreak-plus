## Overview

`global_struct()` returns `&globals`, a pointer to an array of pointers. Each pointer leads to a descriptor table that defines a class of firmware variables. The two primary tables are:

- **`globals[4]`** — Main therapy/setting variable descriptors (0x1DF entries, 0x1C-byte records)
- **`globals[8]`** — Menu/UI variable descriptors (0xA5 entries, 0x14-byte records)

Both tables share a common **flags ushort at byte 0** and follow a two-layer architecture: ROM descriptors define static properties, and RAM shadow copies hold mutable runtime state.

---

## `globals[8] (variable_types_table)` — Menu/UI Variable Descriptors

**Record stride:** 0x14 (20 bytes)  
**Entry count:** 0xA5 (165)  
**Variable ID range:** 0x20D – 0x2B1 (mapped via `between_20d_and_260()`: `id - 0x20D = index`)  
**Handler:** `variable_get_20d_to_260_minus_20d()`, handler object size 0x1C  
**RAM shadow:** `RAM_PTR_200104a0 + index * 4` (4 bytes per entry: 2-byte flags + 1-byte state + 1-byte pad)

### Record Layout (0x14 bytes)

| Offset | Size | Type    | Field | Description |
|--------|------|---------|-------|-------------|
| +0x00  | 2    | ushort  | **flags** | Status/permission bitmask (see Flags Ushort below). Copied to RAM on init. |
| +0x02  | 1    | byte    | **callback_id** | Post-change callback index. 0 = none. Non-zero selects a function from a jump table (`FUN_08075f3c`). Checked by `FUN_0806aeb8` as a boolean ("has callback"). |
| +0x03  | 1    | byte    | *(padding/unused)* | |
| +0x04  | 2    | short   | **linked_var_id** | Index into `globals[4]` table (0x7FFF = none/terminator). Used by `global_0x10_ee8()` to walk a chain of up to 4 dependent [4] variables, propagating updates. The chain follows `globals[4][id].next_var_id` at `[4]+0x04`. |
| +0x06  | 2    | short   | **name_str_id** | variable name string_id|
| +0x08  | 1    | byte    | **default_value** | Default state/value byte. Copied to `RAM + index*4 + 2` on init by `variable_table_copy_and_slice()`. |
| +0x09  | 1    | byte    | **num_options** | Number of valid options/entries for this variable. Used as loop bound in string enumeration, and compared against current value for range checking. |
| +0x0A  | 2    | —       | *(unaccessed)* | |
| +0x0C  | 4    | uint32  | **permission_bitmask** | Per-bit permission flags. Tested as `(1 << mode_id) & bitmask` to determine if this variable is accessible in a given device mode. Entry 5's bitmask at absolute offset `base + 0x70` is the **language-enable bitmask** used by `string_get_locale()`. |
| +0x10  | 2    | short   | **base_string_id** | Base string ID for `string_lookup()` via `globals[2]`. Value 0xDE = no string / sentinel. When valid, `base_string_id + option_index` gives the string for each option. |
| +0x12  | 2    | —       | *(unaccessed)* | |

### Cross-reference to `globals[4] (gui_limits)`

For entries with `index >= 0xA3`, the function `arg_minus_0xa1(index)` computes `index - 0xA1` as a corresponding index into `globals[4]`. This is stored in handler object field `[6]` and used for value persistence. So `globals[8]` entries 0xA3–0xA4 map to `globals[4]` entries 0x02–0x03, etc. Entries < 0xA3 return -1 (no [4] mapping).

The `linked_var_id` at +0x04 provides a *direct* link into `globals[4]` for dependency propagation — this is separate from the index-based `arg_minus_0xa1` mapping.

---

## `globals[4] (gui_limits)` — Main Therapy/Setting Variable Descriptors

**Record stride:** 0x1C (28 bytes)  
**Entry count:** 0x1DF (479)  
**Variable ID range:** 0x1E – 0x1FC (mapped via `var_between_30_and_509_minus_30()`: `id - 0x1E = index`)  
**Handler:** `variable_get_type_up_to_0x1df()`, handler object size 0x38  
**RAM shadow:** `DAT_0806cee8 + index * 8` (8 bytes: 2-byte flags + 2-byte pad + 4-byte value)  
**Secondary RAM:** `DAT_0806bfd8 + index * 8` (for sub-handler `variable_0x00_to_0x1b4_init`, same format)

### Record Layout (0x1C bytes)

| Offset | Size | Type    | Field | Description |
|--------|------|---------|-------|-------------|
| +0x00  | 2    | ushort  | **flags** | Status/permission bitmask (see Flags Ushort below). Copied to RAM on init. |
| +0x02  | 1    | byte    | **callback_id** | Post-change callback index (same semantics as [8]+0x02). Referenced via `param_1[4] + 2` in `task_something_maybe`. |
| +0x03  | 1    | byte    | *(unknown/pad)* | |
| +0x04  | 2    | short   | **next_var_id** | Linked-list pointer: index of next dependent variable in `globals[4]` (0x7FFF = end of chain). Used by `global_0x10_ee8()` to walk up to 4 chained variables. `globals[8]` entries point into this chain via their `linked_var_id` at [8]+0x04. |
| +0x06  | 2    | short   | **name_str_id** | variable name string_id|
| +0x08  | 4    | int32   | **default_value** | Default value. Copied to RAM value field on init. Loaded by `FUN_0806b3ba` to reset variable to default. |
| +0x0C  | 4    | int32   | **max_value** | Maximum allowed value (upper clamp). Used by clamping logic: `if (value > max) value = max`. Also used by `FUN_08075650` for display width calculation, and `FUN_0806fcf8` for option count. |
| +0x10  | 4    | int32   | **min_value** | Minimum allowed value (lower clamp). Used by clamping logic: `if (value < min) value = min`. |
| +0x14  | 1    | byte    | **decimal_places** | Display precision / number of decimal places. Passed as 4th argument to `FUN_0806fcf8` (option count calculation) and read directly for formatting. |
| +0x15  | 1    | —       | *(unknown/pad)* | |
| +0x16  | 2    | short   | **scale_factor** | Scaling/precision factor for string↔value conversion. Passed to `FUN_0806ac92` (string-to-value via floating point). If positive: `value = parsed / scale`. If negative: `value = parsed * (-scale)`. |
| +0x18  | 2    | short   | **step_size** | Increment/decrement step. Used by increment (`value += step`), decrement (`value -= step`), and rounding/snap-to-grid (`value mod step`). |
| +0x1A  | 2    | ushort  | **units_string_id** | String ID for the units/suffix label (e.g., "cmH₂O", "min", "%"). Looked up via `string_lookup_also()` through `globals[2]`. |

### Sub-handler Ranges within `globals[4]`

The 0x1DF entries are partitioned into sub-ranges with different handler types:

| Index Range | Variable IDs | Handler | Notes |
|-------------|-------------|---------|-------|
| 0x000 – 0x1B3 | 0x1E – 0x1D1 | `variable_0x00_to_0x1b4_init` | Standard numeric variables. Handler 0x1C bytes. Uses `DAT_0806bfd8` RAM. |
| 0x1B4 – 0x1D3 | 0x1D2 – 0x1F1 | `variable_0x1dd_to_0x1d4_init` | Extended numeric variables. `DAT_0806c770` holds extra max/min pairs (8 bytes each). On init, the +0x0C/+0x10 fields from this range are copied to `DAT_0806cef8`. |
| 0x1D4 – 0x1D8 | 0x1F2 – 0x1F6 | `FUN_0806c02c` | Handler size 0x20. |
| 0x1D9 – 0x1DC | 0x1F7 – 0x1FA | `FUN_0806c406` / `FUN_0806c58e` | Handler size 0x20. |
| 0x1DD – 0x1DE | 0x1FB – 0x1FC | `FUN_0806c68c` | Stores `arg_minus_0x1dd(id)` in handler[7] — cross-index. |

---

## Flags Ushort (byte 0-1) — Shared Format

Both `globals[4]` and `globals[8]` (and `globals[3]`, `globals[6]`) use the same bitmask format at the first 2 bytes of each record. The ushort is copied from ROM to the RAM shadow on init, then bits are dynamically set/cleared at runtime.

### Bit Definitions

| Bit | Mask | Read by | Written by | Meaning |
|-----|------|---------|------------|---------|
| 0   | 0x01 | `task_something_maybe` (`<< 0x1f` test) | `FUN_0806ae2e` | **Active** — master enable gate. If clear, all value changes are blocked. |
| 1   | 0x02 | `FUN_0806ad6e` (`<< 0x1e` test) | `FUN_0806ae28` | **Visible** — controls whether the variable appears in UI menus. Feeds → vtable+0x78 → GUI bit 4 (0x10). |
| 2   | 0x04 | `FUN_0806ad7c` (`<< 0x1d` test) | `FUN_0806ae34` | **Editable** — controls whether the user can modify the value. Feeds → vtable+0x7c → GUI bit 5 (0x20). |
| 3   | 0x08 | (low nibble `& 0xf` for debug hex dump) | — | Unknown / reserved. Part of 4-bit packed sub-field. |
| 4   | 0x10 | `FUN_0806adac` (`<< 0x18 >> 0x1c & 1 ^ 1`) | `FUN_0806ade8` (clears only) | **Read-only lock** — when set, variable is locked. `FUN_0806adac` returns 0 when set. Feeds → GUI bit 6 (0x40) via `variable_call_status_handler`. |
| 5   | 0x20 | — | `dispatch_0x80_and_0xb0` (sets) | **Dirty/Changed** — set when a value change is committed. |
| 6   | 0x40 | — | ROM only; **always cleared** on boot (`& 0xFFBF`) | **Factory default marker** — present in ROM descriptors, unconditionally cleared in all RAM shadow copies during init. Likely flags entries that have meaningful ROM defaults. |
| 7   | 0x80 | — | — | Unused (low byte). |

### GUI Widget Status Byte (derived layer)

The GUI menu system maintains a separate status byte at `gui_widget + 0x10`, populated by querying the variable handler vtable:

| GUI Bit | Mask | Source | Query |
|---------|------|--------|-------|
| 4 | 0x10 | Descriptor bit 1 (visible) | `dispatch_0x38_0x78` → vtable+0x78 |
| 5 | 0x20 | Descriptor bit 2 (editable) | `dispatch_0x38_0x7c` → vtable+0x7c |
| 6 | 0x40 | Descriptor bit 4 (locked) + vtable 0xac/0x80 | `variable_call_status_handler` |
| 2 | 0x04 | Static/config | Selects greyed-out color palette in `gui_color_palette_lookup` |

---

## RAM Shadow Layout

### For `globals[8]` — 4 bytes per entry at `RAM_PTR_200104a0`

| Offset | Size | Init Source | Description |
|--------|------|-------------|-------------|
| +0x00  | 2    | `[8]+0x00` | Flags ushort (mutable copy) |
| +0x02  | 1    | `[8]+0x08` | Current value / state byte |
| +0x03  | 1    | —           | Padding |

### For `globals[4]` — 8 bytes per entry at `DAT_0806cee8`

| Offset | Size | Init Source | Description |
|--------|------|-------------|-------------|
| +0x00  | 2    | `[4]+0x00` | Flags ushort (mutable copy) |
| +0x02  | 2    | —           | Padding |
| +0x04  | 4    | `[4]+0x08` | Current value (int32, mutable) |

### Secondary for `globals[4]` IDs 0x1B4–0x1DC — 8 bytes per entry at `DAT_0806cef8`

| Offset | Size | Init Source | Description |
|--------|------|-------------|-------------|
| +0x00  | 4    | `[4]+0x0C` (at index 0x1B4+i) | Dynamic max value (mutable) |
| +0x04  | 4    | `[4]+0x10` (at index 0x1B4+i) | Dynamic min value (mutable) |

---

## Dependency Chain: `global_0x10_ee8()`

When a `globals[8]` variable changes, `global_0x10_ee8()` propagates the change to dependent `globals[4]` variables:

```
globals[8] entry
  └─ +0x04: linked_var_id → globals[4][linked_var_id]
       └─ +0x04: next_var_id → globals[4][next_var_id]
            └─ +0x04: next_var_id → ... (up to 4 deep, 0x7FFF terminates)
```

At each node, `variable_0x176_something()` is called and the result stored to `DAT_0806b05c + var_id * 8 + 4`.

---

## Variable Lookup Dispatch

`variable_lookup_handler_maybe(var_id)` routes variable IDs to the correct descriptor table:

```
if var_id <= 0x1D:
    → globals[3], stride 10, handler size 0x18
elif var_id 0x1E..0x1FC:
    → globals[4], stride 0x1C, handler size 0x38
    (internally sub-dispatched by variable_dispatch_by_id)
elif var_id 0x1FD..0x20C:
    → globals[6], stride 0x18, handler size 0x20
elif var_id 0x20D..0x2B1:
    → globals[8], stride 0x14, handler size 0x1C
elif var_id == 0x2B2:
    → globals[9], stride 0x18
elif var_id == 0x2B6:
    → globals[0x11], stride 8
elif var_id == 0x2B7:
    → globals[0x12], stride 8
```

