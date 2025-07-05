# CHIP-8 Emulator (Terminal Version)

This is a simple CHIP-8 emulator written in C for learning purposes. It is not 100% faithful to the original specification, but it implements enough functionality to load and run many basic CHIP-8 ROMs.

Instead of using a graphical library or sound, the emulator outputs graphics to the terminal using ASCII characters and handles keyboard input directly from `stdin` using terminal control.

## Features

- Minimal CHIP-8 instruction set emulation
- ASCII-based display in terminal (e.g. `#`for pixels)
- Simple input handling from keyboard
- No external libraries used — just standard C and POSIX calls
- Cross-platform support may be limited to Unix-like systems (Linux, macOS)

## How It Works

- The display is rendered as a grid of characters in the terminal.
- Keyboard input is read in raw mode from `stdin` using file descriptor `STDIN_FILENO`.
- The emulator loads a `.ch8` ROM file into memory and begins execution.

## Build

Make sure you have a C compiler (e.g. `gcc`) installed.

```bash
make
```
