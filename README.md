# 👾 Space Invaders — C++ / SFML

A classic Space Invaders clone built with **C++** and **SFML** — featuring alien waves, destructible shields, a UFO bonus target, particle explosions, and escalating difficulty across levels.

---

## Screenshots

> *(Add your own gameplay screenshots here)*

---

## Features

- 🛸 **55 aliens** across 5 rows with 3 types (different point values & colors)
- 🧱 **4 destructible shield bunkers** — both player and alien bullets chip them away
- 🔴 **UFO bonus target** — flies across the top for 150 points
- 💥 **Particle explosion effects** on kills
- 📈 **Escalating difficulty** — aliens get faster and shoot more frequently each level
- 🎮 Smooth 60 FPS gameplay with frame-rate independent movement

---

## Controls

| Key | Action |
|-----|--------|
| `←` / `A` | Move left |
| `→` / `D` | Move right |
| `Space` / `↑` | Shoot |
| `Escape` | Quit |
| `Enter` | Start / Restart |

---

## Requirements

- C++17 or later
- [SFML 2.5+](https://www.sfml-dev.org/)
- CMake (optional) or any C++ compiler (g++, clang++, MSVC)

---

## Building

### Linux / macOS

```bash
# Install SFML (Ubuntu/Debian)
sudo apt install libsfml-dev

# Install SFML (macOS with Homebrew)
brew install sfml

# Compile
g++ space_invaders.cpp -o space_invaders -lsfml-graphics -lsfml-window -lsfml-system

# Run
./space_invaders
```

### Windows (MinGW)

```bash
g++ space_invaders.cpp -o space_invaders.exe -lsfml-graphics -lsfml-window -lsfml-system
space_invaders.exe
```

### Windows (MSVC)

1. Download SFML from [sfml-dev.org](https://www.sfml-dev.org/download.php)
2. Link against `sfml-graphics.lib`, `sfml-window.lib`, `sfml-system.lib`
3. Copy the SFML `.dll` files next to the executable

---

## Font (Optional)

The game works without a custom font but looks best with a pixel font. Download [Press Start 2P](https://fonts.google.com/specimen/Press+Start+2P) (free, Google Fonts) and place `PressStart2P-Regular.ttf` in the same directory as the binary.

If no font file is found, the game falls back to `DejaVu Sans Mono` (available by default on most Linux systems).

---

## Scoring

| Alien type | Points |
|------------|--------|
| Bottom row (green) | 30 |
| Middle rows (purple) | 20 |
| Top rows (orange) | 10 |
| UFO | 150 |

---

## Project Structure

```
space_invaders/
├── space_invaders.cpp       # Full game source (single file)
├── PressStart2P-Regular.ttf # Optional pixel font
└── README.md
```

---

## License

MIT — free to use, modify, and distribute.

---

## Acknowledgements

Inspired by the original *Space Invaders* (Taito, 1978). Built as a learning project to explore SFML and game loop architecture in C++.
