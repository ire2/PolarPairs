# 🎮 Polar Pairs

**Polar Pairs** is a vertical-scrolling puzzle platformer where two adorable animal characters—**a polar bear** and **a penguin**—must work together to escape an icy labyrinth. Designed for mobile devices using the **CUGL (Cornell University Game Library)** framework, the game challenges players to master character-specific mechanics and navigate treacherous terrain through clever cooperation.

---

## 🧩 Gameplay Overview

- Control two characters with distinct abilities:
  - **Polar Bear**: Can break certain blocks and push heavy objects.
  - **Penguin**: Can slide across ice and fit into small spaces.
- Solve puzzles using **dual-character coordination**.
- Navigate through **modular, procedurally loaded levels** with **vertical scrolling**.
- Reach the goal by switching between characters and leveraging their unique movement types.

---

## 🔧 Features

- 🧠 **Asymmetric Puzzles** – designed around character-specific mechanics.
- 🌨️ **Modular Level Loading** – loads visible level chunks dynamically as the camera scrolls.
- 📱 **Mobile-Ready UX** – optimized UI and touch controls for mobile platforms.
- 🎨 **Hand-Crafted Level Design** – built from `.txt` files with visual tile mapping.
- 📸 **Smooth Camera Tracking** – follows active character with consistent bottom offset for visibility.

---

## 🛠️ Tech Stack

- **C++** with **CUGL (Cornell University Game Library)**
- Scene graph for rendering and UI layout
- `.txt` level parsing with `LevelData` and grid-to-node translation
- Custom timeline and animation handling for interactions (e.g., bounce feedback)

---

## 🚀 Getting Started

### Prerequisites

- CUGL engine set up for mobile deployment
- CMake, Android NDK, or Apple (Best in Apple and Android)
- Git LFS (for asset management, if needed)

### Build Instructions

1. Clone the repository:
   ```bash
   git clone https://github.com/yourusername/polar-pairs.git
    ```
2.    Build using Apple, Android Studio or CMake folders within build folder.
4.    Run on mobile device or emulator.

---

## 🧪 Testing and Debugging
    •    Modular level testing using mock .txt files
    •    Debug toggle overlays for collision tiles
    •    Event-based logging for character actions and tile interactions

---

## 📁 File Structure
```
/assets             # Textures, sounds, fonts
/src
  ├── PolarPairsApp.cpp      # Game entry point
  ├── PolarPairsController   # Game logic and scene handling
  ├── PlaygroundRenderer     # Renders tile grid and characters
  └── LevelData              # Loads and interprets .txt levels
```

---

## 👥 Team
  -    Ignacio Estrada – Lead UX & Game Design, Project Management

---

## ✨ Acknowledgments
  -    Built as part of the Cornell University Game Design capstone.
  -    Special thanks to the course staff and the CUGL development team.

---
