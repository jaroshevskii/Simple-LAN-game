#!/usr/bin/env bash
# Verifies that the build produced a complete, runnable bin/ directory:
# executables, the Engine/Game shared libraries and every runtime asset the
# engine opens at startup. CI runs this after each build and before packaging
# a release, so a missing file fails the pipeline instead of the game.
set -u

BIN_DIR="${1:-build/bin}"

fail=0
require() {
	if [ ! -f "$BIN_DIR/$1" ]; then
		echo "MISSING: $BIN_DIR/$1"
		fail=1
	fi
}

# --- binaries ---------------------------------------------------------------
case "$(uname -s)" in
	MINGW*|MSYS*|CYGWIN*|Windows_NT)
		require "Power.exe"
		require "Engine.dll"
		require "Game.dll"
		require "SDL2.dll"
		;;
	Darwin)
		require "Power"
		require "libEngine.dylib"
		require "libGame.dylib"
		;;
	*)
		require "Power"
		require "libEngine.so"
		require "libGame.so"
		;;
esac

# --- System/: console config + every shader the engine compiles at startup --
require "System/ConsoleSymbols.cfg"
for pair in TextShaders Model_Test RoomShaders_ SkyBoxShaders; do
	require "System/Shaders/$pair/vertShader.shr"
	require "System/Shaders/$pair/fragShader.shr"
done
require "System/Shaders/SimpleShaderShape/vertShader.shr"
require "System/Shaders/SimpleShaderShape/fragShader.shr"
require "System/Shaders/SimpleShaderShape/vertShaderTexture.shr"
require "System/Shaders/SimpleShaderShape/fragShaderTexture.shr"

# --- Data/: fonts and the multiplayer level loaded by startGame() -----------
require "Data/Font/Console1.fo"
require "Data/Font/Console1.te"
require "Data/Font/Display1.fo"
require "Data/Font/Display1.te"
require "Data/Levels/Multiplayer/DM1.wo"

if [ "$fail" -ne 0 ]; then
	echo "Runtime file check FAILED"
	exit 1
fi

echo "Runtime file check OK: $BIN_DIR is complete"
