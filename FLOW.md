# mbed-app — System Flows

This document describes how the **Bootloader**, **HostFlashApp** (PC tool), and **Application** work together to download new firmware over UART and run it on a NUCLEO-H755ZI-Q board.

---

## 1. System Overview

```
┌─────────────────┐    USB       ┌──────────────┐    UART (USART2)    ┌──────────────────┐
│  Linux PC       │═════════════▶│  USB-Serial  │════════════════════▶│  STM32H755 board │
│  HostFlashApp   │              │   adapter    │   (PA2 TX / PA3 RX) │  Bootloader/App  │
└─────────────────┘              └──────────────┘                     └──────────────────┘
                                                                              │
                                                                              │ ST-LINK VCP (USART3)
                                                                              ▼
                                                                       PC terminal logs
```

Three independent software pieces:

| Piece | Where it lives | Role |
|-------|----------------|------|
| **HostFlashApp** | `HostFlashApp/PcTool/` | PC binary that reads a `.bin` file and ships it to the board over a USB-serial adapter |
| **Bootloader** | `Bootloader/` | Always boots first. Decides whether to enter download mode or jump to the application |
| **Application** | `App/` | The actual firmware (FreeRTOS-based, blinks LEDs, prints logs) |

---

## 2. Flash Memory Layout

The STM32H755 has dual-bank flash. We use both banks:

| Address | Size | Owner | Contents |
|---------|------|-------|----------|
| `0x08000000` | 1 sector (128 KB) | Bootloader | Bootloader code |
| `0x08020000` | 1 sector (128 KB) | Bootloader | Configuration block (`is_app_flashed`, `app_size`, `app_crc`, valid marker) |
| `0x08040000` … | rest of Bank 1 | unused | — |
| `0x08100000` | many sectors | Application | Application binary (flashed by the bootloader during download) |

The configuration block is what tells the bootloader **on each boot** whether a valid app exists and what its CRC should be.

---

## 3. Boot Flow

Every reset goes through the bootloader first.

```
                         ┌────────────────────────┐
                         │  Power-on / Reset      │
                         └───────────┬────────────┘
                                     │
                                     ▼
                         ┌────────────────────────┐
                         │  Bootloader main()     │
                         │  - HAL init            │
                         │  - Clock config        │
                         │  - GPIO init           │
                         │  - USART2/3 init       │
                         │  - IWDG init (30 s)    │
                         │  - Load config block   │
                         └───────────┬────────────┘
                                     │
                          ┌──────────┴──────────┐
                          │ Download required?  │   (USER button held? / config flag set? / no app yet)
                          └──────────┬──────────┘
                              yes    │    no
                       ┌─────────────┘    └─────────────┐
                       ▼                                ▼
            ┌─────────────────────┐           ┌────────────────────┐
            │ Run download flow   │           │ Is app flashed?    │
            │ (see §4)            │           └────────┬───────────┘
            └──────────┬──────────┘                yes │  no
                       │                               │
                       ▼                               ▼
            ┌─────────────────────┐           ┌────────────────────┐
            │ Verify CRC of app   │◀──────────│ Stay in bootloader │
            │ in flash            │           │ (blink red LED)    │
            └──────────┬──────────┘           └────────────────────┘
                  pass │ fail
                       │
                       ▼
            ┌─────────────────────┐
            │ Jump to App at      │
            │ 0x08100000          │
            └─────────────────────┘
```

### What triggers download mode?

| Trigger | Source |
|---------|--------|
| First-time boot (no app flashed) | `etx_config->is_app_flashed == false` |
| USER button held during boot | GPIO read in bootloader main |
| Application requested re-flash | App writes a flag in config block before resetting |

---

## 4. Application Download Flow (Host ↔ Bootloader)

This is the heart of the system. Both sides maintain a small state machine and exchange frames over UART.

### 4.1 State machine

```
 Bootloader                                   HostFlashApp
 ──────────                                   ─────────────
   IDLE  ──START──▶ HEADER  ──HEADER──▶ DATA  ──DATAᴺ──▶ COMPLETE  ──END──▶ SUCCESS
     ▲                ▲                  ▲                    ▲
     └── NACK ────────┴── NACK ──────────┴── NACK ────────────┘
   (host retransmits up to 3 times)
```

### 4.2 Sequence diagram

```
Host                                                     Bootloader
 │                                                            │
 │── START frame (10 bytes) ─────────────────────────────────▶│  IDLE → HEADER
 │◀────────────────────────────────────────────── ACK (4 B) ──│
 │                                                            │
 │── HEADER frame (size + CRC, 17 bytes) ────────────────────▶│  HEADER → DATA
 │◀────────────────────────────────────────────── ACK (4 B) ──│  Erase Bank 2
 │                                                            │
 │── DATA frame #1 (≤ 10 KB) ────────────────────────────────▶│  Write to flash
 │◀────────────────────────────────────────────── ACK (4 B) ──│
 │── DATA frame #2 ──────────────────────────────────────────▶│  Write to flash
 │◀────────────────────────────────────────────── ACK (4 B) ──│
 │       ⋮  (repeat for ⌈total_size / 10240⌉ chunks)          │
 │── DATA frame #N (final, may be partial) ──────────────────▶│  Write to flash
 │◀────────────────────────────────────────────── ACK (4 B) ──│  Last fragment → COMPLETE
 │                                                            │
 │── END frame (10 bytes) ───────────────────────────────────▶│  COMPLETE → SUCCESS
 │◀────────────────────────────────────────────── ACK (4 B) ──│  Save config (CRC, size, valid)
 │                                                            │
 ▼                                                            ▼
 exit                                                  jump to app
```

### 4.3 What gets re-tried

If the bootloader sends **NACK** (frame failed CRC, wrong type, or bad length), the host retransmits the *same* frame up to **3 times**. After 3 NACKs the host gives up and exits with failure. The bootloader counts NACKs sent and aborts if it sends more than 3 in a row.

---

## 5. Frame Protocol

Two frame types travel over the wire.

### 5.1 Data frame (Host → Bootloader)

```
┌─────┬──────────┬─────┬─────────────┬─────┬─────┐
│ SOF │ pkt_type │ len │   payload   │ CRC │ EOF │
├─────┼──────────┼─────┼─────────────┼─────┼─────┤
│  1B │    1B    │ 2B  │  N bytes    │ 4B  │  1B │
└─────┴──────────┴─────┴─────────────┴─────┴─────┘
  AA      01/02/03    LE      (≤ 10240)    LE     BB
```

| Field | Value | Notes |
|-------|-------|-------|
| `SOF` | `0xAA` | Start-of-frame marker |
| `pkt_type` | `01` (CMD) / `02` (HEADER) / `03` (DATA) | What kind of frame this is |
| `len` | 1..10240 | Payload size in bytes (little-endian) |
| `payload` | N bytes | Command byte / header struct / firmware bytes |
| `CRC` | CRC32 over `[SOF…end of payload]` | Reflected CRC32, polynomial `0xEDB88320`, init `0xFFFFFFFF`, final XOR `~` |
| `EOF` | `0xBB` | End-of-frame marker |

### 5.2 Response frame (Bootloader → Host)

```
┌─────┬──────────┬───────┬─────┐
│ SOF │ pkt_type │ N/ACK │ EOF │
├─────┼──────────┼───────┼─────┤
│  1B │    04    │   1B  │  1B │
└─────┴──────────┴───────┴─────┘
  AA      04         01/02   BB
```

`01` = ACK, `02` = NACK. Always exactly 4 bytes. No CRC (small enough to risk it).

### 5.3 Payload contents per packet type

| `pkt_type` | Payload | Size |
|------------|---------|------|
| `CMD = 01` | One byte: `01`=START, `02`=ABORT, `03`=END | 1 |
| `HEADER = 02` | `[total_size_BE(4) + expected_crc_BE(4)]` | 8 |
| `DATA = 03` | Raw firmware bytes for this chunk | 1..10240 |

---

## 6. UART + DMA Reception (Bootloader Side)

The bootloader can't busy-poll for bytes — frames are variable-length and 10 KB at 115200 baud takes ~890 ms. It uses DMA + idle-line detection:

```
┌────────────────┐  receives bytes  ┌─────────┐  raw bytes  ┌──────────────┐
│ USART2 (RX)    │─────────────────▶│  DMA1   │────────────▶│ rx_buffer_a  │
│ PA3, 115200 8N1│                  │ Stream0 │             │  (10249 B)   │
└────────────────┘                  └─────────┘             └──────────────┘
        │                                                            ▲
        │ "line went idle"                                           │ (ping-pong with rx_buffer_b)
        ▼
   IDLE flag set ──▶ USART2_IRQ ──▶ HAL_UARTEx_RxEventCallback ──▶ swap buffers, set rx_frame_ready
```

Why ping-pong: while the bootloader processes (validates CRC, writes flash) one buffer, DMA can already be filling the other with the next frame. Critical for keeping up with back-to-back transfers.

### Key timing rule

**DMA must be re-armed BEFORE the bootloader sends ACK**, because:

1. Bootloader sends ACK (~350 µs)
2. Host receives ACK (1–5 ms)
3. Host immediately starts next data frame (10 KB)
4. **First byte hits USART RDR within microseconds**

If DMA isn't armed by step 4, the second arriving byte triggers an **overrun (ORE)** and the USART stops accepting bytes. The fix: re-arm DMA *first*, then send ACK.

---

## 7. Flash Write Sequence

For each `DATA` frame the bootloader receives:

```
┌────────────────────────────────────────────────────────────┐
│  Frame valid?  →  yes                                      │
│                                                            │
│  if (first data frame):                                    │
│      erase application area in Bank 2                      │
│                                                            │
│  re-arm DMA on the spare buffer  ◀── overlap with flash    │
│                                                            │
│  HAL_FLASH_Unlock + program 32-byte words to flash         │
│  HAL_FLASH_Lock                                            │
│                                                            │
│  send ACK to host                                          │
└────────────────────────────────────────────────────────────┘
```

After all data frames are written:
1. `END` command arrives.
2. Bootloader saves the config block: `is_app_flashed = true`, `app_size = total`, `app_crc = expected_crc`, `is_app_bootable = true`, `valid_marker = 0xCAFE_BABE`.
3. Sends final ACK.
4. Returns to `main()` which jumps to the application.

---

## 8. CRC Verification

Two CRCs are involved:

| Where | What it covers | When it's checked |
|-------|----------------|-------------------|
| Per-frame CRC | One frame's bytes | Bootloader checks every received frame; mismatch = NACK |
| Whole-app CRC | All firmware bytes | Bootloader checks once, on every boot, against the value stored in the config block |

The whole-app CRC is computed by the **STM32 hardware CRC peripheral** (faster than software). The host computes the same CRC32 in software using the standard reflected algorithm.

A signed/unsigned bug used to mis-classify any CRC with bit 31 set as "invalid" — fixed by changing `get_application_crc()` to return `bool` with an out-pointer.

---

## 9. Independent Watchdog (IWDG)

The bootloader starts the IWDG with a 30-second timeout:

```
hiwdg.Init.Prescaler = IWDG_PRESCALER_256;  // 32 kHz / 256 = 125 Hz
hiwdg.Init.Reload    = 3750U;               // 3750 / 125 = 30 s
```

Once started, **IWDG cannot be stopped**. It survives the bootloader → application jump.

The **application** must therefore refresh it. A dedicated FreeRTOS task does this:

```
vTaskWatchdog (priority 3, 128B stack):
    while (1):
        HAL_IWDG_Refresh(&hiwdg)
        vTaskDelay(5000ms)
```

Refreshing every 5 s on a 30-s timeout = 6× safety margin.

If the FreeRTOS scheduler hangs, this task can't run, IWDG fires, chip resets, control returns to the bootloader.

---

## 10. Application Lifecycle (FreeRTOS)

```
main():
  HAL_NVIC_SetPriorityGrouping(NVIC_PRIORITYGROUP_4)   // FreeRTOS requires 4-bit pri
  HAL_Init()
  SystemClock_Config()
  MX_GPIO_Init()
  MX_USART3_UART_Init()                                // VCP for logs

  xTaskCreate(vTaskApplicationMain, …, prio 1, …)
  xTaskCreate(vTaskGreenBlink,      …, prio 2, …)
  xTaskCreate(vTaskOrangeBlink,     …, prio 2, …)
  xTaskCreate(vTaskRedBlink,        …, prio 2, …)
  xTaskCreate(vTaskWatchdog,        …, prio 3, …)      // highest

  vTaskStartScheduler()
  // never returns
```

Tasks:

| Task | Period | Purpose |
|------|--------|---------|
| Main | 2 s | "Main task is running" log |
| Green Blink | 0.5 s | Toggle LED1 |
| Orange Blink | 1 s | Toggle LED2 |
| Red Blink | 1.5 s | Toggle LED3 |
| Watchdog | 5 s | Refresh IWDG |

---

## 11. Error Recovery Flows

### 11.1 Bad frame on the wire
```
host sends frame → bootloader CRC fails → bootloader sends NACK
                                       → host retries (up to 3 times)
                                       → if still fails, host exits with error
```

### 11.2 USART overrun (ORE)
```
silicon latches ORE → USART stops accepting bytes → bootloader's restart_dma_receive()
detects HAL state inconsistency → calls HAL_UART_AbortReceive() → resets state →
re-arms DMA → next frame received cleanly
```

### 11.3 Application hung (FreeRTOS deadlock)
```
watchdog task can't run → IWDG_RLR not refreshed for 30 s → silicon reset →
bootloader reboots → if config still says valid app, jumps back to app →
app boots fresh
```

### 11.4 Application has corrupt CRC
```
bootloader computes hardware CRC over flash region → mismatch with stored CRC →
bootloader sets is_app_bootable = false → stays in bootloader, blinks red LED →
waits for new download
```

### 11.5 Power loss during flash
```
flash sectors partially written → on next boot, bootloader recomputes CRC →
mismatch → application marked unbootable → bootloader stays in download mode →
host re-flashes
```

---

## 12. Pinout Reference (NUCLEO-H755ZI-Q)

| Signal | MCU pin | Board connector | Notes |
|--------|---------|-----------------|-------|
| USART2 TX | PA2 | CN9 pin 3 (Arduino D1) | To USB-serial adapter RX |
| USART2 RX | PA3 | CN9 pin 1 (Arduino D0) | To USB-serial adapter TX |
| USART3 TX | PD8 | (internal to ST-LINK) | Logs via VCP |
| USART3 RX | PD9 | (internal to ST-LINK) | — |
| LED1 (green) | PB0 | onboard | Application activity |
| LED2 (orange/yellow) | PE1 | onboard | Application activity |
| LED3 (red) | PB14 | onboard | Bootloader-mode indicator |
| USER button | PC13 | onboard blue button | Held → enter download mode |

---

## 13. Build & Run

```sh
# Build bootloader
cd Bootloader && make -j$(nproc)
# Flash via ST-LINK (uses st-flash / OpenOCD / Cube Programmer)

# Build host tool
cd HostFlashApp/PcTool && make -j$(nproc)

# Build application binary
cd App && make -j$(nproc)

# Flash app via bootloader
sudo ./HostFlashApp/PcTool/build/HostFlashApp_v2.0.5 ttyUSB0 App/build/App_v1.3.0.bin
```

UART: **115200 baud, 8N1, no flow control**. `OVERSAMPLING_16` on the MCU (max safe noise margin). The device argument to the host tool is just the tty name (`ttyUSB0`), **not** the full `/dev/ttyUSB0` path — the RS-232 library prepends `/dev/` itself.
