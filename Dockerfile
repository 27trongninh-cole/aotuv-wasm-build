FROM emscripten/emsdk:3.1.51

WORKDIR /build

# Công cụ phụ trợ cần để tải + giải nén source aoTuV/libogg
RUN apt-get update && apt-get install -y --no-install-recommends \
    wget \
    git \
    p7zip-full \
    bzip2 \
    autoconf \
    automake \
    libtool \
    pkg-config \
    && rm -rf /var/lib/apt/lists/*

COPY scripts/ /build/scripts/
COPY src/ /build/src/

RUN chmod +x /build/scripts/*.sh

# Toàn bộ quá trình build chạy trong 1 script duy nhất, xem scripts/build_all.sh
CMD ["/build/scripts/build_all.sh"]
