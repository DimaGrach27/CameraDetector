FROM debian:bookworm

RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    ninja-build \
    pkg-config \
    make \
    gcc-aarch64-linux-gnu \
    g++-aarch64-linux-gnu \
    binutils-aarch64-linux-gnu \
    file \
    rsync \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /work