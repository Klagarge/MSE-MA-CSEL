# https://github.com/devcontainers/images/tree/main/src/cpp
ARG VARIANT=ubuntu-24.04
FROM mcr.microsoft.com/devcontainers/cpp:${VARIANT}

RUN apt-get update && \
    export DEBIAN_FRONTEND=noninteractive && \
    apt-get -y install --no-install-recommends \
    autoconf \
    automake \
    bc \
    bison \
    clang-format \
    cpio \
    device-tree-compiler \
    file \
    flex \
    libfl-dev \
    libglib2.0-dev \
    libssl-dev \
    libubootenv-tool \
    libyaml-dev \
    mercurial \
    python3-dev \
    rsync \
    swig \
    u-boot-tools \
    wget \
    bear

RUN curl --proto '=https' --tlsv1.2 -sSf https://just.systems/install.sh | bash -s -- --to /usr/local/bin

COPY scripts/* /usr/local/bin/
RUN chmod +x /usr/local/bin/*
