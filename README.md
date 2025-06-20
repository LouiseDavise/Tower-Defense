# 🛡️ Tower Defense Game

A feature-rich, visually styled **Tower Defense Game** built with Allegro 5. It includes enemy wave systems, turret building, player authentication (new & returning), score tracking, and a stage-based progression system.

---

## 🎮 Features

### 🔐 Player System
- **New Player Entry**: Users enter their name and are assigned a unique UID.
- **Returning Players**: Re-enter your UID to resume and view history.
- **Persistent Data**: Player names and UIDs stored in `Data/player_uid.txt`.

### 🏰 Core Gameplay
- **Tower Defense Mechanics**: Build turrets, manage resources, and survive waves of enemies.
- **Enemy Variety**: Includes normal soldiers, tanks, super tanks, extreme tanks (with shields), and fast-moving planes.
- **Strategic Depth**: Use tools like mines and shovels, and manage grid-based placement logic.
- **Stages**: Choose between multiple maps to increase difficulty.

### 📈 Leaderboard & History
- **Global Leaderboard**: Shows top scores from all players (`Data/scoreboard.txt`).
- **Welcome Scene**: Returning players can view their personal past performances.
- **Score Metrics**: Based on enemies killed, time survived, money earned, and remaining lives.

### ⚙️ Settings
- **Volume Controls**: Adjust BGM and SFX volumes with sliders.

---

## 🕹️ Controls

| Key      | Function                        |
|----------|---------------------------------|
| `Q`, `W` | Place different turret types    |
| `0–9`    | Set game speed multiplier       |
| `TAB`    | Toggle debug mode               |
| Mouse    | Interact with UI and place items|

---

## 📁 Project Structure

├── Enemy/ # Enemy type implementations (Soldier, Tank, Plane, etc.)
├── Turret/ # Turret classes (Machine Gun, Laser, Bom, etc.)
├── UI/ # UI components (buttons, labels, effects)
├── Scene/ # Game scenes (Play, Start, Auth, Win, Lose, etc.)
├── Data/ # Saved player data and leaderboard
├── Resource/ # Maps and enemy wave data
└── player_data.h # Global player data (UID, name)

---

## 🛠️ Technologies

- **Language**: C++
- **Graphics/Audio Library**: [Allegro 5](https://liballeg.org/)
- **File I/O**: Used for saving leaderboard and player info
- **Standard Library**: STL containers, strings, file streams, etc.

---

## 🧠 Notable Code Concepts

- **Pathfinding**: BFS-based for enemy path calculations
- **Dynamic UI**: Paginated leaderboard and history views
- **Object-Oriented Design**: Modular and extensible scene and entity system
- **Gameplay Logic**: Includes explosion effects, turret previews, resource deduction, etc.

---

## 🧪 Build Instructions

1. Make sure **Allegro 5** is installed and linked.
2. Clone the repository:
   ```bash
   git clone https://github.com/yourusername/tower-defense-game.git
   cd tower-defense-game
