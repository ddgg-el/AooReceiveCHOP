Aoo Receive CHOP
================

This repository contains the code base to develop a [TouchDesigner](https://derivative.ca/) `CHOP` that receives an audio stream from an Aoo network.

More information about the Aoo project can be found at the [Aoo Homepage](https://aoo.iem.sh/)

### Build

```zsh
$ mkdir build
$ cd build
$ cmake -GXcode .. -DCMAKE_BUILD_TYPE=Debug -DAOO_LOG_LEVEL=Debug
$ cmake --build .
```
The `AooReceiveCHOP.plugin` will be created inside the `plugin` and can be loaded via a `CPlusPlusCHOP` or directly installed in the User `Palette`

### Useful resources

- How to write C++ plugin in TouchDesigner
[https://docs.derivative.ca/Write_a_CPlusPlus_Plugin](https://docs.derivative.ca/Write_a_CPlusPlus_Plugin)


