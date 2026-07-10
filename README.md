# LAN Game

Screenshot
---------
![alt text](https://github.com/PowerMaze/Simple-LAN-game/blob/master/Data/shot_1652025_131.png?raw=true)

About
-----

An online network game that I created in my final year of university. The game is not optimised, crashes are possible and uses a very naive client-server implementation. 
The engine uses some third-party libraries to facilitate development.

It also uses resources from another game as a placeholder for testing, and this is about weapons - (Q3ShotGun.mo and q3Rocket.mo, as well as textures) both taken from the game Quake III Arena by Id Software. I do not and will not use them for commercial purposes! Space skybox named "Space Nebula - SkyBox Texture", check for credits https://gamebanana.com/mods/318131. All other resources are made by me.

You may have seen some similarities with other engines, and there are. I took inspiration for ideas from other engines such as Serious Engine and Id Tech 3, and yes, I didn't copy anything from there.

You might also have seen my nickname as Dark-Head - it was my old nickname when I was developing.

This project will most likely be an introduction to how online games and game engines work. I hope it will be useful for someone who wants to get acquainted with how games work.
As this is an alpha version of the game, I plan to improve both the engine and the game in the future.

Projects
--------

In total, we have 6 MS Visual Studio projects, only 3 of which are used for the game. Splitting the game into separate projects will give us the advantage of modularity, as we plan to support game modifications.

* `Editor` In this version, it is most likely a model converter. In the next versions there will be support for the level editor.
* `Engine` Game engine. It is responsible for graphics, networking, level geometry, and other components for the game's functioning.
* `Game` Game module, responsible for the execution of game logic, the game server session and the client. Game entities are also described here.
* `MakeResource` Used to generate textures, models and fonts.
* `Power` Game executable file.
* `Test` It is not used in the game and game engine at all. It is mainly used to test new features.

Building (CMake — Windows / Linux / macOS)
------------------------------------------

The project is cross-platform and builds with CMake on Windows, Linux and macOS.
Platform-specific code (Winsock vs BSD sockets, `LoadLibrary` vs `dlopen`, MSVC-isms)
is isolated in `Source/Engine/Common/Platform.h`; windowing, input and dynamic
library loading go through SDL2 everywhere.

Install the dependencies first (if they are missing, CMake falls back to
building them from source automatically):

```sh
# macOS
brew install cmake ninja sdl2 libsodium

# Debian / Ubuntu
sudo apt install cmake ninja-build libsdl2-dev libsodium-dev libgl1-mesa-dev

# Windows: only CMake + Visual Studio 2022 are required
# (SDL2 and libsodium are fetched and built automatically)
```

Then configure, build and test with the preset for your OS:

```sh
cmake --preset linux    # or: macos, windows
cmake --build --preset linux
ctest --preset linux
```

Binaries land in `build/bin`: the `Power` launcher, the `Engine` and `Game`
shared libraries, plus the `MakeModel` and `Tests` tools. The level `Editor`
is built on Windows only (it uses Win32 file dialogs).

CI builds all three platforms on every push (see `.github/workflows/`).

Building (legacy Visual Studio solution)
----------------------------------------

The original Visual Studio solution is still in `Source/Source.sln`. To use it, install Microsoft Visual Studio 2022 or higher to compile the code, but there is a nuance...

As I said, the engine uses third-party libraries, but due to other licences, they were not included in the project except for the `Bullet Engine`, which is included in the `Engine` project itself, and GLM (OpenGL Mathematics).

To successfully compile and run the game, you need the following libraries:

* `SDL2` Input, window handling, openGL contex.
* `Libsodium` For encrypt/decrypt messages between client and server.

After you have installed the libraries, you need to place the header files and static libraries, as well as the dynamic libraries. There are two folders in the `Sources` folder: `Libs` and `Includes`. 
Place the header files with folders in `Includes`, and static libraries in `Libs`. IMPORTANT: The libsodium library is supplied in both a release and a debug version. 
Therefore, for the debug version, rename the name of the library with the letter “D” (Dlisodium.lib) and the dynamic library (Dlibsodium.dll). 
Add the dynamic libraries to the root folder of the game.

After you have arranged the header files and libraries in the folders, you should get something like this:

```
Source
├──Includes
|  ├──SDL2
|  |  └──headers
|  └──sodium
|     └──headers
└──Libs  
   └──.lib files
```

If everything is fine, you can compile the code and try to play :). Please note that the projects are ALL configured for the x86 platform.

And one more thing - do not use spaces in the names of directories and solution paths

Common problems
---------------

As I said, the game is not perfect, it can crash. Also, if you can't connect to the server on your local network, configure your firewall (Inbound rules) to allow UDP port 5910 for the game.

License
-------

This  game is licensed under the MIT license (see LICENSE file).

Below are the side libraries that are used:

* `SDL2` by Sam Lantinga and SDL Community (zlib license)
* `Libsodium` see credits in official documentation: https://doc.libsodium.org/#thanks (ISC license)
* `Bullet Engine` by Erwin Coumans (zlib license)
* `GLM` by Christophe Riccio (MIT license)
* `stb image` by Sean Barrett (MIT license)
