# Slime Knight

Slime towers have overrun the land. Smash every tower with your hammer, cut down every slime with your sword, and face the Slime King in his keep.

Originally made in VB.NET with FlatRedBall (2013). Ported to C++ with [asw](https://github.com/adsgames/asw).

## Controls

| Action | Keyboard | Gamepad |
| --- | --- | --- |
| Move | WASD / Arrows | Left stick |
| Swing sword (hold to keep swinging) | Space / J | A |
| Dash (no damage while dashing) | Shift / K | B |
| Hammer slam (smashes towers) | E / L | Y |
| Care packages: regen, health, energy, speed | 1 - 4 | D-pad |
| Pause | Esc / P | Start |

## Setup

### CMake

```bash
cmake --preset debug
cmake --build --preset debug
```

### Build Emscripten

```bash
emcmake cmake --preset debug
cmake --build --preset debug
```

The original VB.NET source, scene layouts and layered Paint.NET art are kept in `original/` for reference.
