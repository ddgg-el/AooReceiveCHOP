Aoo Receive CHOP
================

**IMPORTANT ONLY TESTED ON MACOS**

This repository contains the code base to develop a [TouchDesigner](https://derivative.ca/) `CHOP` that receives an audio stream from an Aoo network.

More information about the Aoo project can be found at the [Aoo Homepage](https://aoo.iem.sh/)

### Build

```zsh
$ mkdir build
$ cd build
$ cmake ..
$ cmake --build .
```
The `AooReceiveCHOP.plugin` will be automatically installed in the TouchDesigner User Library and can only be loaded as a `CustomOperator`.

### Useful resources

- How to write C++ plugin in TouchDesigner
[https://docs.derivative.ca/Write_a_CPlusPlus_Plugin](https://docs.derivative.ca/Write_a_CPlusPlus_Plugin)


