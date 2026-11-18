# ChaosPlotter

An interactive, MPI-parallel renderer for escape-time fractals that draws the
**orbits** (the full iteration paths) of points rather than just coloring each
pixel by escape time — similar in spirit to a Buddhabrot. Worker ranks compute
orbits in parallel and stream them to a rendering rank that draws them live
with OpenGL, so you can pan and zoom while the image keeps accumulating.

## How it works

```
 rank 0 (renderer)                     ranks 1..N-1 (workers)
 ┌───────────────────────┐  ViewParams  ┌──────────────────────────┐
 │ GLFW/OpenGL window    │ ───────────▶ │ sample points c in view  │
 │ keyboard / scroll     │              │ iterate z -> f(z, c)     │
 │ draw orbits (HSV by   │ ◀─────────── │ batch orbits, one MPI    │
 │ iteration, alpha-     │  orbit batch │ message per sweep        │
 │ blended accumulation) │              └──────────────────────────┘
 └───────────────────────┘
```

* **Rank 0** owns the window. Whenever the view changes it sends a
  `ViewParams` struct (bounds + sample spacing) to every worker, and it polls
  for incoming orbit batches with non-blocking `MPI_Iprobe`, so the UI never
  stalls waiting on a worker.
* **Worker ranks** sweep a grid over the current view with jittered sample
  points, each rank covering an interleaved slice of every cell. Each orbit is
  iterated up to `ITERATIONS` steps or until it escapes `BOUND`, then the whole
  sweep is packed into a single `double` buffer and sent in one message.
* Orbits are drawn as translucent line strips colored by iteration (HSV), so
  dense regions build up brightness over time.
* Shutdown is a clean handshake: rank 0 sends `TAG_SHUTDOWN` and waits for each
  worker to acknowledge before `MPI_Finalize`.
* The renderer prints a running **paths-per-second** count, which makes it easy
  to measure scaling as you add ranks.

The map being iterated lives in [`src/function.cpp`](src/function.cpp). The
Mandelbrot set and Burning Ship are included as commented-out alternatives;
the default is a custom cubic-style variant.

## Build

Requires an MPI implementation (OpenMPI or MPICH), GLFW 3, and OpenGL.

```bash
# Ubuntu / Debian
sudo apt install build-essential libopenmpi-dev openmpi-bin libglfw3-dev libgl1-mesa-dev libglu1-mesa-dev

make            # builds build/chaosplotter with mpic++ -O3
```

Compile-time knobs (in [`include/protocol.h`](include/protocol.h)) can be
overridden with `-D`:

| Macro | Default | Meaning |
|---|---|---|
| `ITERATIONS` | 1000 | maximum iterations per orbit |
| `BOUND` | 40000 | escape radius squared |
| `DENSITY` | 40 | sampling density |
| `CULLING` | 1.0 | fraction of each orbit's tail to drop before sending |

```bash
make CXXFLAGS="-std=c++17 -O3 -Iinclude -DITERATIONS=5000"
```

## Run

At least two ranks are needed (one renderer + one or more workers):

```bash
mpirun -np 8 ./build/chaosplotter
```

| Control | Action |
|---|---|
| Arrow keys | pan |
| Mouse wheel | zoom |
| Esc | leave fullscreen |
| Close window | shut down all ranks |

## Project layout

```
include/   protocol.h (MPI tags, shared structs), renderer.h, worker.h, function.h
src/       main.cpp (rank dispatch), renderer.cpp, worker.cpp, function.cpp
Makefile
```
