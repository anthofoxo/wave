# Wave

## Cloning
git clone --recurse-submodules https://github.com/anthofoxo/wave.git

## Building
### Premake
This project uses premake as it's build system.
To build this project run [premake5](https://premake.github.io/).
A prewritten script for visual studio 2019 is located in the scripts/ directory.
After premake is ran for the project, all the neccesary project files should now exist.
### Incbin ([Only applies to MSVC](https://github.com/graphitemaster/incbin#msvc))
We use a library called [incbin](https://github.com/graphitemaster/incbin) to include font files, audio, etc embedded in the binary. Making it easier to distribute.
In your IDE just build the incbin project to generate the executable file.
All binaries will be located in bin/.
After this is ran, the entire solution should be able to be built without a problem.
