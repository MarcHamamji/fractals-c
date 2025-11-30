# A GTK fractals explorer in C

## Setup

- Install `gcc` and `make`.

## Usage

```sh
make # Compile the code
make run # Run the compiled main.out
```

## Controls

### General

| Keys | Description |
| - | - |
|`Up`, `Down`, `Left`, `Right` | Move the view 10% towards this direction |
| `+`/`=`, `-` | Zoom in / out the view |
| `q` | Quit the application |
| `j` | Toggle between the different fractals (Mandelbrot, Julia, Newton) |
| `h` | Reset the view, the number of iterations, and the settings of the current fractal |
| `o` | Toggle showing the overlays |
| `i`, `I` | Increment / Decrement the number of iterations

### Julia Fractal

| Keys | Description |
| - | - |
| `w`, `a`, `s`, `d` | Move $z_0$ 0.1 complex units towards this direction |
| `Mouse Drag` | Move $z_0$ using the mouse |


## Development

To get proper autocompletion for `clangd`, you need to generate a `compile_commands.json` file. This can be done using the `bear` tool.
```
bear -- make clean all
```
