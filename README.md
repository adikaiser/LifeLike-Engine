# LifeLike Engine
Lifelike Engine is a software that can simulate various biological components and their interactions.<br>
I developed it as my Matura Thesis in the last two years of high school and apart from `third_party` everything was written by me or taken from a tutorial.<br>

## Installation
### Dependencies
LifeLike Engine has a few dependencies that need to be separately installed in order for it to work
- [Vulkan SDK](https://vulkan.lunarg.com/)
- A [C++ compiler](#compilers) (Only if you want to compile the code yourself)
- [CMake](https://cmake.org/download/) (Only if you want to compile the code yourself)

There are two ways of getting LifeLike Engine to work on your computer

### Option 1: Download a precompiled executable
Download the latest [release](https://github.com/j-goth/LifeLike-Engine/releases) folder for your operating system and unzip it. The executable is inside that folder with the name `LifeLike-Engine` 

### Option 2: Compile the code manually
Download or clone the entire repository (including third party dependencies) via Terminal:

```terminal
git clone --recursive https://github.com/j-goth/LifeLike-Engine.git
```

or install manually:

1. Download this repository (green button -> Download ZIP) and unzip it.
2. Follow the links in [third_party](https://github.com/j-goth/LifeLike-Engine/tree/main/third_party) and separately download each Repository.
3. Unzip all of them into `third_party` (e.g. `LifeLike-Engine/third_party/SDL`)

Now create a directory called `build` in `LifeLike-Engine` and execute the following terminal commands

```terminal
# Create 'build' if not already existent and move there
mkdir <LifeLike Engine directory>/build

# Compile
cmake -S <LifeLike Engine directory> -B <LifeLike Engine directory>/build
cmake --build <LifeLike Engine directory>/build --config Relese
``` 
If everything worked there should be an executable with the name `LifeLike-Engine` in `build` (or a subdirectory depending on the compiler)


**NOTE:**
The default build type is Release. In case you want to use Debug mode (outputs some status messages to make it easier to find errors) you need to change `cmake -S <> -B <>` to:
```terminal
cmake -S <LifeLike Engine directory> -B <LifeLike Engine directory>/build -DCMAKE_BUILD_TYPE=Debug
```

### Compilers
LifeLike Engine was tested with a few compilers

**Linux:**
- [GCC](https://gcc.gnu.org/) (Works fine)

**Windows**
- [MSVC](https://learn.microsoft.com/en-us/cpp/overview/acquire-msvc?view=msvc-170) (Compiles with warnings)
- [MinGW](https://www.mingw-w64.org/) (Works fine)

Other compilers might or might not work but the compilation process should be the same for all of them
