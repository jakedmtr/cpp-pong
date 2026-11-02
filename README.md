# Pong — C++20 & SDL3

A local implementation of the classic Pong game, built in **C++20** with **SDL3** and **SDL3_ttf**.

This is a hands-on programming project focused on real-time application structure, keyboard input, movement, collision response, and game-state management. The emphasis is on understanding and implementing the underlying logic rather than using a game engine.

## Features

- **Delta-time-based movement** for the paddles and ball
- **Ball physics and collisions**, including angled rebounds from paddles
- **Score tracking**, ball resets after points, and match-winning conditions
- **Pause menu** with Resume and Quit options
- **Game Over screen** showing the winner, with Play Again and Quit options
- **On-screen text** rendered using SDL3_ttf

## Controls

| Action | Key |
| --- | --- |
| Player 1: move up / down | `W` / `S` |
| Player 2: move up / down | `↑` / `↓` |
| Pause / resume | `P` |
| Navigate menus | `↑` / `↓` |
| Select a menu option | `Enter` |
| Restart after game over | `R` |

## Built With

- **C++20** — game logic and application code
- **SDL3** — window creation, rendering, input, and timing
- **SDL3_ttf** — font loading and text rendering

## Build and Run (Linux)

### Requirements

- A C++20-capable compiler (such as GCC)
- SDL3 and SDL3_ttf, including their development files
- `pkg-config`
- The font asset at `assets/score.ttf`

From the repository root, compile with:

```bash
g++ -std=c++20 -Wall -Wextra main.cpp -o pong \
  $(pkg-config --cflags --libs sdl3 sdl3-ttf)
```

Then run:

```bash
./pong
```

Run the executable **from the repository root**, because the game loads its font using a relative path (`assets/score.ttf`).

## Project Structure

```text
.
├── assets/
│   └── score.ttf      # Font used for scores and menus
├── main.cpp           # Game logic, input, rendering, and state handling
└── README.md
```

## What I Practised

This project was an exercise in fundamental C++ and software-engineering concepts:

- Structuring a real-time **game loop** around input, updates, and rendering
- Using **delta time** for frame-rate-independent movement
- Detecting and resolving **wall and paddle collisions**
- Managing **game states**, such as playing, paused, and game over
- Handling **SDL resource lifetimes** and cleanup
- Building and debugging a native C++ application on Linux

## Scope

This project is intentionally small: a complete, local two-player Pong game rather than a general-purpose game engine. It does **not** include an AI opponent or online multiplayer.