# A GTK fractals explorer in C

## Setup

- Install `gcc` and `make`.

## Usage

```sh
make # Compile the code
make run # Run the compiled main.out
```

## Development

To get proper autocompletion for `clangd`, you need to generate a `compile_commands.json` file. This can be done using the `bear` tool.
```
bear -- make clean all
```
