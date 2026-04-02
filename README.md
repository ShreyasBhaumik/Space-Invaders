# 👾 Space Invaders — C++ / SFML

A polished Space Invaders clone built with **C++17** and **SFML** — pixel-art aliens, scrolling starfield, destructible shields, UFO bonus target, particle explosions, and escalating difficulty. Fully self-contained downloadable app for Windows, Linux, and macOS.

[![Build & Release](https://github.com/YOUR_USERNAME/space-invaders/actions/workflows/release.yml/badge.svg)](https://github.com/YOUR_USERNAME/space-invaders/actions/workflows/release.yml)
[![Latest Release](https://img.shields.io/github/v/release/YOUR_USERNAME/space-invaders?label=download)](https://github.com/YOUR_USERNAME/space-invaders/releases/latest)
[![License: MIT](https://img.shields.io/badge/License-MIT-green.svg)](LICENSE)

---

## ⬇️ Download & Play

**Pre-built binaries — no compiler needed:**

| Platform | File | How to run |
|----------|------|------------|
| 🪟 Windows | [`SpaceInvaders_Windows.zip`](https://github.com/YOUR_USERNAME/space-invaders/releases/latest) | Extract → double-click `space_invaders.exe` |
| 🐧 Linux   | [`SpaceInvaders_Linux.tar.gz`](https://github.com/YOUR_USERNAME/space-invaders/releases/latest) | Extract → `./play.sh` |
| 🍎 macOS   | [`SpaceInvaders_macOS.tar.gz`](https://github.com/YOUR_USERNAME/space-invaders/releases/latest) | Extract → `./play.sh` |

> Replace `YOUR_USERNAME` with your GitHub username.

---

## Features

- 🎨 **Pixel-art sprites** — hand-crafted bitmap aliens (crab, octopus, squid), UFO, and player cannon drawn at runtime — no image files needed
- ⭐ **Scrolling starfield** background
- 🧱 **4 destructible shield bunkers** with 3-hit degradation per cell
- 🔴 **UFO mystery ship** — random score bonus (50–300 pts)
- 💥 **Particle burst** explosions on every kill
- 📺 **Screen flash** feedback on hits
- 🏆 **High score** persists across rounds
- 📈 **Unlimited levels** — speed and fire rate scale each wave
- 👾 Player blinks invincible after being hit

---

## Controls

| Key | Action |
|-----|--------|
| `←` / `A` | Move left |
| `→` / `D` | Move right |
| `Space` / `↑` | Shoot |
| `Escape` | Quit |
| `Enter` / `Space` | Start / Restart |

---

## Scoring

| Alien | Points |
|-------|--------|
| Top rows (orange crab) | 10 |
| Middle rows (purple octopus) | 20 |
| Bottom rows (green squid) | 30 |
| UFO mystery ship | 50 – 300 (random) |

---

## Building from Source

### Requirements
- C++17 compiler (GCC 9+, Clang 9+, MSVC 2019+)
- [SFML 2.5+](https://www.sfml-dev.org/)
- CMake 3.16+

### Linux

```bash
sudo apt install libsfml-dev cmake      # Ubuntu/Debian
# or: sudo dnf install SFML-devel cmake  # Fedora

cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j$(nproc)
./build/space_invaders
```

### macOS

```bash
brew install sfml cmake

cmake -B build -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_PREFIX_PATH="$(brew --prefix sfml)"
cmake --build build -j$(sysctl -n hw.logicalcpu)
./build/space_invaders
```

### Windows (Visual Studio + vcpkg)

```powershell
vcpkg install sfml:x64-windows

cmake -B build -DCMAKE_TOOLCHAIN_FILE="$env:VCPKG_INSTALLATION_ROOT/scripts/buildsystems/vcpkg.cmake"
cmake --build build --config Release

.\build\Release\space_invaders.exe
```

---

## Font (Optional)

The game works without any font files — SFML's default is used as fallback. For the authentic retro look, download [Press Start 2P](https://fonts.google.com/specimen/Press+Start+2P) (free, OFL licence) and place `PressStart2P-Regular.ttf` next to the binary or in an `assets/` subfolder.

The CI build fetches this font automatically.

---

## Project Structure

```
space-invaders/
├── .github/
│   └── workflows/
│       └── release.yml          # Auto-build & publish CI
├── space_invaders.cpp            # Entire game — single source file
├── CMakeLists.txt                # Cross-platform build + packaging
├── PressStart2P-Regular.ttf      # Optional pixel font (not committed)
└── README.md
```

---

## Publishing a Release

```bash
git add .
git commit -m "Release v1.0"
git tag v1.0
git push origin main --tags
```

GitHub Actions compiles for all three platforms and creates a Release with download links automatically.

---

## License

MIT — free to use, modify, and distribute.

---

*Inspired by the original Space Invaders (Taito, 1978).*
