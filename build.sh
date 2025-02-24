#!/bin/bash
set -e

FLAC_DIST=/emsdk/upstream/emscripten/cache/sysroot
FLAC_SHARED_LIB=${FLAC_DIST}/lib/libFLAC.a

emcc ${FLAC_SHARED_LIB} ./encodeFlac.c \
  -s WASM=1 \
  -O3 \
  -s ASSERTIONS=1 \
  -s ALLOW_MEMORY_GROWTH=1 \
  -s EXPORTED_RUNTIME_METHODS='["FS"]' \
  -s FORCE_FILESYSTEM=1 \
  -s USE_OGG=1 \
  -o wasm/encodeFlac.js --bind
