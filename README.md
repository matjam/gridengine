# GridEngine

This project is what I'm building as a basis for my own roguelike. It is currently incomplete, and therefore I don't recommend you use it as the basis of your roguelike, just yet - unless you want to contribute to the project. I'm mostly putting it up here so other people who want to do something similar to me, have an existing pattern.

I've been working on this in one shape or form for a while now, I felt it was time to just throw it all together on github to see if it's useful for anyone else. But primarily, it's only useful for me, probably.

The end desire is that in the process of writing my game, that I will develop a framework that other people can use to make roguelikes, and can extend to make roguelike development in C++ easier.

# Features

- ConsoleScreen: fixed width PCF unicode font support for bitmap character mode rendered console screen. Tested with 8x8 and 8x16 fonts - but you'll need to probably do some modifications to make it work properly.

  The screen is drawn using sf::VertexBuffer for speed; probably can be optimized further.

- Logging: for logging things with spdlog to the console for now. Intend to implement a file logger at some point.
- FileCache: integrated in memory simple file cache - ask it for a file and it will read it once and cache in memory.
- Stats: class for tracking the time things take in your code, so you can then display stats on it in game. Useful for finding the hotspots in your code. Just decorate the things you think are expensive with `Stats::being("name_of_stat")` and `Stats::end("name_of_stat")` and then you can fetch the average time taken (over a default 30 samples) with `Stats::getAverageTime("name_of_stat")`.
- ScriptEngine: A wrapper around Lua which right now just bootstraps Lua. At a future point I'll be adding functions to it to allow you to access game structures such as the ConsoleScreen data from within lua.
- State and StateStack: classes for managing a stack of event handlers. When you bring up the main menu, for example, you can push a new State to the StateStack and it will receive all the input events, and when you exit the main menu, it can go back to the main state.
- Engine: core of the system; manages the main window, and handles the main event/draw loop.
- It builds on Linux, Windows and MacOS X - and even makes a Mac .app bundle with configuration to include the data files into the bundle. Cmake is cool, yo.

# Todo

- Configuration files in Lua to configure things like the font and screen geometry
- basic in-game menuing API
- utilities in the ConsoleScreen to aid drawing of various text elements, menus, tables, etc.
- a game (LOL)
- a framework for
  - map generation algorithms
  - Line of sight calculations
  - pathfinding
  - some kind of command system for the run loop

# Notes

This is in flux .. I don't expect that the API will remain stable for a while. When it reaches a level of what I consider complete, I will bless it with a 1.0.0 version number. But, for now, it will live here and I'll add to it as I build out my game.

# Building

I use CMake to try to simplify things; the project should build on Windows, Linux and MacOS X.

Generally speaking, you can build with

```
git submodule update --init --recursive
mkdir build
cd build
cmake ..
cmake --build .
```

## Requirements

You need to install vcpkg somewhere; if you want this cmake build to just work, then you should install it in `C:\Dev\vcpkg` on Windows and `/usr/local/vcpk` on Linux and MacOS X.

### vcpkg packages

You need at least the following:

- lua (used by sol2 pulled in from submodules)
- sfml
- spdlog
- freetype
- fmt
- boost - this might be optional at this moment
- probably opengl stuff

It might also whinge when you run cmake that it doesn't have something - add it and let me know.

### Windows

I have only tested with Visual Studio Community 2019 for the compiler. You will need to install that, and then when you run cmake, specify that as the compiler. If you use Visual Studio Code, you should use the kit "Visual Studio Community 2019 Release - x86_amd64". I've not tested with anything else.

### Mac OS X

Obviously, you'll need X Code and all it provides, and you'll need to install the cli tools kit with it.

### Linux

Tested with clang10. You will need some system packages for the vcpkgs to install; they will complain when you try to add them, then you add them.

## Running

You need to start the game in the root of the project as it expects to find files in `data/`. Once running you can hit the following keys

- F1 - brings up the debug menu with some stats
- F2 - shows a map of all the characters loaded from the font. Yep, all of them.
- F3 - smashes the screen with random colors/characters.
- F4 - does an old style loading screen.

# Developing with it

I've tried to separate things out so you can implement stuff in the main source folder. What I suggest you do though is just make this project an external git submodule of your project, then you can separate your code out and use what I provide as a skeleton.

As I work with the project more I will add more information, so really - you're on your own for now.

# Support

Not at this time. You can feel free to chat to me about this, and if you want to make any small PRs I'm happy to integrate if they make sense to me at the time. But this thing is nowhere near complete.

# Thanks

Thanks to Viznu - http://pelulamu.net/unscii/ - for the fabulous unscii font.

# Contact

You can usually find me in #roguelikedev on https://discord.gg/tgUhtQC

# License

Everything in this repo is licensed under the [MIT License](LICENSE).
