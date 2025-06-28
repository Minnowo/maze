
A maze gamme / solver.

# How to play

## CLI Arguments
```
-w <width>
-h <height>
-l <0-100>    0 = most walls, 100 = no walls
-a <0|1>      0 = DFS, 1 = Floodfill
```

## Controls

- hold `space` to run the solving algorithm
- press `r` to generate a map
- use `up` `left` `right` `down` keys to move around

# Building

You need CMake and a C++ compiler
```sh
make build

# and run
./build/maze -w 30 -h 30 -l 5
```

