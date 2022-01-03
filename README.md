# please
made to be simple, easy to build, and mainly to learn.

## License
Everyone is free to use my code to do whatever they want.
The license note in the file LICENSE is valid just for the stuff in the `src` folder.
Everything in the `src` folder is either mine, or is available in the public domain.

The code inside the `include` folder may be redistributed under it's own license (generally found at the top of the file or in a `LICENSE` file).

## Build
Simplest way: If you are on Windows, simply run `build-debug-cl.bat` through VS's Developer Command Prompt x64 and it should compile normally. If you are on Linux, then run `build-debug-gcc.sh` instead. Both of these scripts have only 1 command. You can read it to understand what is needed to compile.

#### build.c
[nobuild](https://github.com/tsoding/nobuild): Works on Windows and Linux.
Simply do
```bash
cc build.c -o build
./build
```
and it should build with the `cc` compiler of your choosing. It can be `gcc`, `clang`, `cl`, or `tcc`.

#### Makefile
You may also have noticed the `Makefile` file. If you want to use it, make sure you have Clang installed on your machine.
Here are some variables you can set (all of them are optional):
* `CUSTOMC` and `CUSTOMLD`: Custom flags for compiling and linking respectively;
* `DEBUG`: Debug flags. It will be passed both for compilation and linking stages;
* `BUILDDIR`: The build directory where the object files and executable are going to be placed at. Defaults to `build`;
* `PLATFORM`: The platform you want to build to. It's worth noting that you should setup your own environment for cross-compilation;

If `CUSTOMC` and `DEBUG` are both not specified, `CUSTOMC` will default to optimization flags.

#### Config macros
Some macros you can `-D` or `-U` to enable/disable stuff. Check out the `src/internal_config.h` file. It has the default options and conditions for each of them.
* `INTERNAL_ENABLE_OPENGL`: Enables the OpenGL 3.3 backend. For now it won't compile with this disabled;
* `INTERNAL_ENABLE_D3D11`: Enables the ~~not working~~ Direct3D 11 backend;
* `INTERNAL_ENABLE_DISCORD_SDK`: Enables the Discord SDK support. When it's enabled, the application will look for a `discord_game_sdk` shared library in the same folder.

## APIs
* Graphics: OpenGL 3.3;
* Gamepad Input: DirectInput, XInput;
* Audio: WASAPI, ALSA;
* System: Win32, Linux (X11);

## External
Dependencies:
* [stb_image](https://github.com/nothings/stb/blob/master/stb_image.h) (src/ext/stb_image.h)
* [stb_truetype](https://github.com/nothings/stb/blob/master/stb_truetype.h) (src/ext/stb_truetype.h)
* [stb_vorbis](https://github.com/nothings/stb/blob/master/stb_vorbis.c) (src/ext/stb_vorbis.h)
* [CGLM](https://github.com/recp/cglm) (include/cglm)

Others:
* [GLFW](https://github.com/glfw/glfw): Some of the code in this repository was borrowed and modified from GLFW (see file `include/guid_utils.h`). Though GLFW in general is a really nice reference.
* [SDL_GameControllerDB](https://github.com/gabomdq/SDL_GameControllerDB): It was used to generate the `include/internal_gamepad_map_database.inc` file.