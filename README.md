# gueepo2D Sample Project

## Building
0. Download and install [https://cmake.org/](CMake) if you don't have it already.
1. Download the repository `git clone --recursive https://github.com/guilhermepo2/gueepo2D-sample.git`
2. Run CMake `cmake -Bbuild .`, or run it with VSCode or a GUI tool. You can download and manually set `SDL2_PATH` on `CMakeLists.txt` on the `src/lib/gueepo2D/gueepo2D/engine/` folder, if that's not defined, CMake will download SDL2 2.0.18 automatically.
3. Open `gueepo2D-sample.sln` on the `build/` folder.

## Building with emscripten

```
cmake -DCMAKE_EXPORT_COMPILE_COMMANDS:BOOL=TRUE -DCMAKE_BUILD_TYPE:STRING=Debug -G "Unix Makefiles" -DCMAKE_TOOLCHAIN_FILE=/opt/homebrew/Cellar/emscripten/3.1.32/libexec/cmake/Modules/Platform/Emscripten.cmake -S/Users/gdeoliveira/workspace/glappy2d -B/Users/gdeoliveira/workspace/glappy2d/www 
```

**Important to note that this is needed on CMakeLists.txt**

```
if(${CMAKE_SYSTEM_NAME} STREQUAL "Emscripten")
    message("emscripten: source dir: " ${CMAKE_SOURCE_DIR})
    set_target_properties(${PROJECT_NAME} PROPERTIES LINK_FLAGS "-s USE_SDL=2 -s USE_WEBGL2=1 -s EXPORTED_FUNCTIONS=['_main','_malloc'] -s EXPORTED_RUNTIME_METHODS=\"['ccall', 'cwrap']\" --preload-file ../src/assets@assets/")
    # target_link_options(${PROJECT_NAME} PRIVATE -sEXPORTED_FUNCTIONS=['_malloc','_free'])
    set(CMAKE_EXECUTABLE_SUFFIX ".html")
endif()
```
