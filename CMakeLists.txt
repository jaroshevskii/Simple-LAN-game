cmake_minimum_required(VERSION 3.21)

project(SimpleLANGame LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_C_STANDARD 11)

# Default to an optimized build when none is requested.
get_property(_is_multi_config GLOBAL PROPERTY GENERATOR_IS_MULTI_CONFIG)
if(NOT _is_multi_config AND NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE RelWithDebInfo CACHE STRING "Build type" FORCE)
  message(STATUS "No CMAKE_BUILD_TYPE set; defaulting to RelWithDebInfo")
endif()

# All binaries (Power, libEngine, libGame, ...) land in one directory so the
# launcher can find the game library and the engine next to itself.
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Look up our own shared libraries next to the executable, so the bin/
# directory stays relocatable (release archives are exactly this directory).
if(APPLE)
  set(CMAKE_BUILD_RPATH "@executable_path")
elseif(UNIX)
  set(CMAKE_BUILD_RPATH "$ORIGIN")
endif()

# Cache compilations when ccache is available (big win for the vendored Bullet).
find_program(CCACHE_PROGRAM ccache)
if(CCACHE_PROGRAM)
  set(CMAKE_C_COMPILER_LAUNCHER "${CCACHE_PROGRAM}")
  set(CMAKE_CXX_COMPILER_LAUNCHER "${CCACHE_PROGRAM}")
  message(STATUS "Using ccache: ${CCACHE_PROGRAM}")
endif()

set(CMAKE_POLICY_VERSION_MINIMUM 3.5)

include(FetchContent)

# --- SDL2 -----------------------------------------------------------------
# System-first (brew/apt/vcpkg); falls back to building from source.
find_package(SDL2 QUIET)
if(SDL2_FOUND)
  message(STATUS "SDL2: using system package")
else()
  message(STATUS "SDL2: building from source")
  FetchContent_Declare(
    SDL2
    GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
    GIT_TAG release-2.30.9
    GIT_SHALLOW TRUE
  )
  set(SDL_TEST OFF CACHE BOOL "" FORCE)
  FetchContent_MakeAvailable(SDL2)
endif()

# --- OpenGL ---------------------------------------------------------------
set(OpenGL_GL_PREFERENCE GLVND)
find_package(OpenGL REQUIRED)

# --- libsodium ------------------------------------------------------------
# No official CMake config upstream: try pkg-config first (brew/apt), then
# build from source via the libsodium-cmake wrapper.
find_package(PkgConfig QUIET)
if(PkgConfig_FOUND)
  pkg_check_modules(SODIUM IMPORTED_TARGET libsodium)
endif()
if(SODIUM_FOUND)
  message(STATUS "libsodium: using system package ${SODIUM_VERSION}")
  set(SODIUM_TARGET PkgConfig::SODIUM)
else()
  message(STATUS "libsodium: building from source")
  set(SODIUM_DISABLE_TESTS ON CACHE BOOL "" FORCE)
  FetchContent_Declare(
    sodium
    GIT_REPOSITORY https://github.com/robinlinden/libsodium-cmake.git
    GIT_TAG 9b2848dfc1b917a9410f0de9d81059b26cbfaa8d  # master 2026-05-29; pulls libsodium as a submodule
  )
  FetchContent_MakeAvailable(sodium)
  set(SODIUM_TARGET sodium)
endif()

# ============================================================================
# Engine — shared library (Engine.dll / libEngine.so / libEngine.dylib)
# Contains the vendored Bullet Physics, renderer, networking and common code.
# ============================================================================

file(GLOB_RECURSE BULLET_SOURCES
  Source/Engine/BulletCollision/*.cpp
  Source/Engine/BulletDynamics/*.cpp
  Source/Engine/LinearMath/*.cpp
)

set(ENGINE_SOURCES
  Source/Engine/Common/Console.cpp
  Source/Engine/Common/ConsoleDataBase.cpp
  Source/Engine/Common/CRC.cpp
  Source/Engine/Common/FileIO.cpp
  Source/Engine/Common/FileName.cpp
  Source/Engine/Common/FontData.cpp
  Source/Engine/Common/Memory.cpp
  Source/Engine/Common/PrintMessages.cpp
  Source/Engine/Common/Timer.cpp
  Source/Engine/DataBase/DataBase.cpp
  Source/Engine/EMain.cpp
  Source/Engine/Entities/CollisionDispatcher.cpp
  Source/Engine/Entities/EntityMain.cpp
  Source/Engine/Entities/PhysicsEntity.cpp
  Source/Engine/Graphics/Draw.cpp
  Source/Engine/Graphics/glad.c
  Source/Engine/Graphics/Render.cpp
  Source/Engine/Graphics/RenderScene.cpp
  Source/Engine/Graphics/Shader.cpp
  Source/Engine/Graphics/SkyBox.cpp
  Source/Engine/Graphics/stb_image.cpp
  Source/Engine/Graphics/Texture.cpp
  Source/Engine/Graphics/View.cpp
  Source/Engine/Level/BaseWorld.cpp
  Source/Engine/Level/Brush.cpp
  Source/Engine/Level/BrushIO.cpp
  Source/Engine/Level/Polygon.cpp
  Source/Engine/Level/RoomBase.cpp
  Source/Engine/Level/WorldVertex.cpp
  Source/Engine/Math/FrustumCulling.cpp
  Source/Engine/Math/Functions.cpp
  # Math/Matrix4.c + Math/Vector.c are excluded (dead code, GLM is used instead),
  # same as in the original Visual Studio project.
  Source/Engine/Model/Import3D.cpp
  Source/Engine/Model/ImportOBJ.cpp
  Source/Engine/Model/Model.cpp
  Source/Engine/Model/ModelIO.cpp
  Source/Engine/Model/ModelMain.cpp
  Source/Engine/Model/Object.cpp
  Source/Engine/NET/Client.cpp
  Source/Engine/NET/GetLastErrors.cpp
  Source/Engine/NET/NetworkChat.cpp
  Source/Engine/NET/NetworkMessage.cpp
  Source/Engine/NET/Protection/SecureData.cpp
  Source/Engine/NET/Relationships.cpp
  Source/Engine/NET/Server.cpp
  Source/Engine/NET/Socket.cpp
  Source/Engine/RenderWorld/RenderEntities.cpp
  Source/Engine/RenderWorld/RenderWorld.cpp
  Source/Engine/stdAfx.cpp
)

add_library(Engine SHARED ${ENGINE_SOURCES} ${BULLET_SOURCES})

target_include_directories(Engine
  PUBLIC
    Source          # <Engine/...> style includes
    Source/Includes # vendored GLM
    Source/Engine   # EMain.h pulls "Common/..." and vendored Bullet headers
)

target_compile_definitions(Engine
  PRIVATE ENGINE_EXPORTS USE_ENGINE _CRT_SECURE_NO_WARNINGS
  PUBLIC  SDL2_IS_ONE USE_SDL
)

target_link_libraries(Engine
  PUBLIC  SDL2::SDL2
  PRIVATE OpenGL::GL ${SODIUM_TARGET}
)

if(WIN32)
  target_link_libraries(Engine PRIVATE ws2_32 iphlpapi)
endif()

# Non-MSVC compilers: the legacy code trips a lot of benign warnings; silence
# the noisy ones. (MSVC builds keep their defaults, same as the .sln did.)
set(POWER_QUIET_WARNINGS "")
if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  set(POWER_QUIET_WARNINGS -Wno-writable-strings -Wno-format-security)
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  set(POWER_QUIET_WARNINGS -Wno-write-strings -Wno-format-security)
endif()
target_compile_options(Engine PRIVATE ${POWER_QUIET_WARNINGS})

# ============================================================================
# Game — the game module loaded by Power at run time (SDL_LoadObject)
# ============================================================================

set(GAME_SOURCES
  Source/Game/Entities/Armor.cpp
  Source/Game/Entities/Bullet.cpp
  Source/Game/Entities/Explosion.cpp
  Source/Game/Entities/Health.cpp
  Source/Game/Entities/Light.cpp
  Source/Game/Entities/PhysicsEntity.cpp
  Source/Game/Entities/PlayerSpawn.cpp
  Source/Game/Entities/Projectile.cpp
  Source/Game/Entities/PushableModel.cpp
  Source/Game/Entities/TextAlert.cpp
  Source/Game/Entities/Weapon.cpp
  Source/Game/Entities/ZoneHurt.cpp
  Source/Game/GCamera.cpp
  Source/Game/GClient.cpp
  Source/Game/GConsole.cpp
  Source/Game/GEntities.cpp
  Source/Game/GHUD.cpp
  Source/Game/GMain.cpp
  Source/Game/GPlayer.cpp
  Source/Game/GPlayerClient.cpp
  Source/Game/GPlayerShared.cpp
  Source/Game/GServer.cpp
  Source/Game/GSession.cpp
  Source/Game/GWeapon.cpp
  Source/Game/stdAfx.cpp
  Source/Game/WeaponPlacement.cpp
)

add_library(Game SHARED ${GAME_SOURCES})
target_include_directories(Game PRIVATE Source/Game)
target_compile_definitions(Game PRIVATE GAME_EXPORTS USE_ENGINE _CRT_SECURE_NO_WARNINGS)
target_link_libraries(Game PRIVATE Engine)
target_compile_options(Game PRIVATE ${POWER_QUIET_WARNINGS})

# ============================================================================
# Power — the game launcher executable
# ============================================================================

add_executable(Power
  Source/Power/Menu.cpp
  Source/Power/MenuButton.cpp
  Source/Power/stdAfx.cpp
  Source/Power/WinMain.cpp
)
target_include_directories(Power PRIVATE Source/Power Source/Game)
target_compile_definitions(Power PRIVATE USE_ENGINE _CRT_SECURE_NO_WARNINGS)
target_link_libraries(Power PRIVATE Engine OpenGL::GL)
if(WIN32)
  target_link_libraries(Power PRIVATE SDL2::SDL2main)
endif()
# Power loads libGame at run time, so make sure it is built alongside
add_dependencies(Power Game)

# The engine resolves every data path relative to the executable directory,
# so mirror the runtime assets next to the binary:
#   System/ - console config + shaders, Data/ - fonts, textures, levels
add_custom_command(TARGET Power POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_CURRENT_SOURCE_DIR}/Source/Power/System
    $<TARGET_FILE_DIR:Power>/System
  COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_CURRENT_SOURCE_DIR}/Data
    $<TARGET_FILE_DIR:Power>/Data)

# Windows: place the dependency DLLs (SDL2.dll etc.) next to Power.exe, so
# the bin/ directory runs and packages as-is.
if(WIN32)
  add_custom_command(TARGET Power POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
      $<TARGET_RUNTIME_DLLS:Power> $<TARGET_FILE_DIR:Power>
    COMMAND_EXPAND_LISTS)
endif()
target_compile_options(Power PRIVATE ${POWER_QUIET_WARNINGS})

# ============================================================================
# Tools
# ============================================================================

add_executable(MakeModel Source/MakeModel/MakeModel.cpp)
target_compile_definitions(MakeModel PRIVATE USE_ENGINE NO_SDL _CRT_SECURE_NO_WARNINGS)
target_link_libraries(MakeModel PRIVATE Engine)

# The level editor uses Win32 file dialogs (OPENFILENAME/_bstr_t), so it is
# built on Windows only.
if(WIN32)
  add_executable(Editor
    Source/Editor/EditorCamera.cpp
    Source/Editor/EditorDoc.cpp
    Source/Editor/EditorMain.cpp
    Source/Editor/EditorModel.cpp
    Source/Editor/stdAfx.cpp
  )
  target_include_directories(Editor PRIVATE Source/Editor)
  target_compile_definitions(Editor PRIVATE USE_ENGINE _CRT_SECURE_NO_WARNINGS)
  target_link_libraries(Editor PRIVATE Engine OpenGL::GL SDL2::SDL2main)
endif()

# ============================================================================
# Tests
# ============================================================================

enable_testing()

add_executable(Tests
  Source/Test/FileName.cpp
  Source/Test/Main.cpp
)
target_compile_definitions(Tests PRIVATE _CRT_SECURE_NO_WARNINGS)

add_test(NAME Tests COMMAND Tests WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/Source/Test)
