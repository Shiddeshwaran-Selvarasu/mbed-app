# Embedded Application Flash Layout

This document describes the flash memory layout for the embedded application with bootloader, OTA support, dual application slots (A/B), and persistent storage.

---

## Flash Details
- **Start Address:** `0x08000000`  
- **Total Capacity:** `2 MB` (`0x200000`)  
- **End Address:** `0x081FFFFF`  

---

## Memory Map

| Region             | Start Address | End Address   | Size (bytes) | Size (KB) | Size (Hex) |
|--------------------|--------------:|--------------:|-------------:|----------:|-----------:|
| **Bootloader**     | `0x08000000`  | `0x0801FFFF`  | 131,072      | 128 KB    | `0x20000`  |
| **System Configs** | `0x08020000`  | `0x0803FFFF`  | 131,072      | 128 KB    | `0x20000`  |
| **Reserved**       | `0x08040000`  | `0x0805FFFF`  | 131,072      | 128 KB    | `0x20000`  |
| **Slot A**         | `0x08060000`  | `0x0810FFFF`  | 720,896      | 704 KB    | `0xB0000`  |
| **Slot B**         | `0x08110000`  | `0x081BFFFF`  | 720,896      | 704 KB    | `0xB0000`  |
| **Persistent Data**| `0x081C0000`  | `0x081FFFFF`  | 262,144      | 256 KB    | `0x40000`  |

---

## Design Notes
- **Bootloader (128 KB):**  
  Handles OTA update logic, integrity checks, and switching between Slot A and Slot B.
  
- **System Configs (128 KB):**  
  Stores system parameters, device configuration, and flags for OTA update state.
  
- **Reserved (128 KB):**  
  Reserved for future use (e.g., secure boot metadata, advanced features).
  
- **Slots A & B (704 KB each):**  
  - Hold application firmware images.  
  - OTA updates are applied to the inactive slot while the other runs.  
  - Bootloader decides which slot to boot from based on update success flags.  
  
- **Persistent Data (256 KB):**  
  Stores calibration data, logs, counters, or other data that must survive firmware upgrades.

