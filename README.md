<h1 align="center">M5StickC Plus2 BLE Remote</h1>

<p align="center">
  <img src="assets/demo.svg" alt="M5StickC Plus2 BLE Remote — the 5 pages" width="440">
</p>

<p align="center">
  <img src="https://img.shields.io/badge/platform-ESP32-333333?logo=espressif&logoColor=white" alt="ESP32">
  <img src="https://img.shields.io/badge/M5StickC-Plus2-FF5000" alt="M5StickC Plus2">
  <img src="https://img.shields.io/badge/PlatformIO-Arduino-FF7F00?logo=platformio&logoColor=white" alt="PlatformIO">
  <img src="https://img.shields.io/badge/Bluetooth-BLE%20HID-2F81F7?logo=bluetooth&logoColor=white" alt="BLE HID">
  <img src="https://img.shields.io/badge/License-MIT-3FB950" alt="MIT">
</p>

<p align="center">
  <b>A pocket Bluetooth remote for your PC, running on an M5StickC Plus2.</b>
</p>

It pairs as a plain BLE keyboard — no dongle, no driver — and puts media keys,
volume, a mic-mute button for calls, presentation controls and a few Windows
shortcuts on the little screen, split across five pages you flip through with a
button combo.

I built it because I was tired of alt-tabbing just to pause music or mute my mic
in Discord mid-game.

## Features

- 🎹 Pairs as a **BLE keyboard** (Windows / macOS / Linux)
- 📄 **5 pages**, switch with **A + B** together:
  - **Media** — play/pause, mute, next / previous
  - **Volume** — up / down (hold to repeat)
  - **Call** — mute mic + deafen/camera (Discord / Teams / Meet / Zoom presets)
  - **Slides** — next / previous, start / exit slideshow
  - **Macros** — screenshot, lock, show desktop, file explorer (Windows)
- 🤝 **Shake** the stick to mute your mic from any page
- 🔋 Reports its **battery level** to the host
- ✨ Flicker-free UI (rendered to an off-screen canvas)

## Hardware

Just an **M5StickC Plus2**. Nothing to wire, nothing to solder.

## Controls

| Page   | A            | Hold A         | B              | Hold B      |
|--------|--------------|----------------|----------------|-------------|
| Media  | Play / Pause | Mute           | Next           | Previous    |
| Volume | Volume +     | *(repeat)*     | Volume −       | *(repeat)*  |
| Call   | Mute mic     | —              | Deafen / Cam   | —           |
| Slides | Next         | Start (F5)     | Previous       | Exit (Esc)  |
| Macros | Screenshot   | Show desktop   | Lock           | Explorer    |

> **Change page:** press **A and B at the same time.**

## Build & flash

Built with [PlatformIO](https://platformio.org/).

```bash
pio run -t upload
```

Or open the folder in VS Code with the PlatformIO extension and hit **Upload**.
Then pair **"M5 StickC Remote"** from your PC's Bluetooth settings.

## Configuration

Everything lives in [`include/config.h`](include/config.h):

| Setting | What it does |
|---------|--------------|
| `CALL_APP` | `1` Discord · `2` Teams · `3` Meet · `4` Zoom — picks the mute/cam shortcuts |
| `SHAKE_TO_MUTE` / `SHAKE_THRESHOLD` | the shake-to-mute gesture |
| `BLE_NAME` | name shown in your PC's Bluetooth list |
| screen / buzzer | rotation, brightness, buzzer volume |

## Note about the mute button

The call actions send app shortcuts (e.g. Discord's `Ctrl+Shift+M`), which only
fire when the app is focused. To mute while you're in a game, set them as a
**global keybind** in the app instead — for Discord: *Settings → Keybinds →
Toggle Mute → `Ctrl+Shift+M`*.

## License

MIT © illoma
