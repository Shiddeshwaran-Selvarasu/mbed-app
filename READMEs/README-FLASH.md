# Embedded Application Flash Layout

This document describes the flash memory layout for the embedded application with bootloader, OTA support, and persistent storage.

---

## Flash Details
- **Start Address:** `0x08000000`  
- **Total Capacity:** `2 MB` (`0x200000`)  
- **End Address:** `0x081FFFFF`  

---

## Memory Map

| Region             | Start Address | End Address   | Size (bytes) | Size (KB) | Size (Hex) |
|--------------------|--------------:|--------------:|-------------:|----------:|-----------:|
| **Bootloader**     | `0x08000000`  | `0x0803FFFF`  | 262,144      | 256 KB    | `0x040000` |
| **System Configs** | `0x08040000`  | `0x0805FFFF`  | 131,072      | 128 KB    | `0x020000` |
| **Reserved**       | `0x08060000`  | `0x080BFFFF`  | 393,216      | 384 KB    | `0x060000` |
| **Persistent Data**| `0x080C0000`  | `0x080FFFFF`  | 262,144      | 256 KB    | `0x040000` |
| **Application**    | `0x08100000`  | `0x081FFFFF`  | 786,432      | 1024 KB   | `0x100000` |

---

## Design Notes
- **Bootloader (192 KB):**  
  Handles OTA update logic, integrity checks, and switching between Slot A and Slot B.
  
- **System Configs (64 KB):**  
  Stores system parameters, device configuration, and flags for OTA update state.
  
- **Reserved (128 KB):**  
  Reserved for future use (e.g., secure boot metadata, advanced features).
  
- **Slots A & B (704 KB each):**  
  - Hold application firmware images.  
  - OTA updates are applied to the inactive slot while the other runs.  
  - Bootloader decides which slot to boot from based on update success flags.  
  
- **Persistent Data (256 KB):**  
  Stores calibration data, logs, counters, or other data that must survive firmware upgrades.
 
 ---
## Reference

- ext_flash_reciever.h
- main.h


