#!/bin/bash
set -euo pipefail

OUT_DIR="/build/output"
WORK_DIR="/build/work"
mkdir -p "$OUT_DIR" "$WORK_DIR"
cd "$WORK_DIR"

echo "=== [1/6] Tải libogg (dependency bắt buộc của libvorbis) ==="
if [ ! -d libogg ]; then
  git clone --depth 1 --branch v1.3.5 https://github.com/xiph/ogg.git libogg
fi

echo "=== [2/6] Tải mã nguồn aoTuV beta6.03 (patch trên libvorbis) ==="
if [ ! -d vorbis_aotuv ]; then
  git clone --depth 1 https://github.com/AO-Yumi/vorbis_aotuv.git vorbis_aotuv
fi

echo "=== [3/6] Build libogg -> libogg.a (static, cho Emscripten) ==="
cd "$WORK_DIR/libogg"
./autogen.sh || autoreconf -fi
emconfigure ./configure --disable-shared --enable-static --prefix="$WORK_DIR/deps"
emmake make -j"$(nproc)"
emmake make install

echo "=== [4/6] Build libvorbis (đã patch aoTuV) -> libvorbis.a + libvorbisenc.a ==="
cd "$WORK_DIR/vorbis_aotuv"
# Repo AO-Yumi thường có sẵn cấu trúc autotools của libvorbis đã patch.
# Nếu cấu trúc thư mục khác (ví dụ chỉ chứa file .c/.h rời), xem README.md
# trong repo này để biết cách map vào libvorbis gốc — xem ghi chú NOTES.md.
./autogen.sh || autoreconf -fi
emconfigure ./configure \
  --disable-shared \
  --enable-static \
  --prefix="$WORK_DIR/deps" \
  --with-ogg="$WORK_DIR/deps"
emmake make -j"$(nproc)"
emmake make install

echo "=== [5/6] Compile wrapper C -> WASM (expose hàm encode cho JS) ==="
cd /build
emcc src/aotuv_wrapper.c \
  -I"$WORK_DIR/deps/include" \
  -L"$WORK_DIR/deps/lib" \
  -lvorbisenc -lvorbis -logg \
  -O3 \
  -s WASM=1 \
  -s MODULARIZE=1 \
  -s EXPORT_NAME="AotuvModule" \
  -s EXPORTED_FUNCTIONS="['_aotuv_encode_wav_to_ogg','_malloc','_free']" \
  -s EXPORTED_RUNTIME_METHODS="['ccall','cwrap','HEAPU8']" \
  -s ALLOW_MEMORY_GROWTH=1 \
  -s ENVIRONMENT=web \
  -o "$OUT_DIR/aotuv.js"

echo "=== [6/6] Xong. File nằm ở: ==="
ls -la "$OUT_DIR"
echo "  -> $OUT_DIR/aotuv.js"
echo "  -> $OUT_DIR/aotuv.wasm"
