FROM ubuntu:24.04

ARG DEBIAN_FRONTEND=noninteractive

# Base build tooling (python3 is used by some MSDK build steps; alias it to `python`)
RUN apt-get update && apt-get install -y --no-install-recommends \
        git make wget xz-utils ca-certificates clang-format python3 \
    && ln -sf /usr/bin/python3 /usr/local/bin/python \
    && rm -rf /var/lib/apt/lists/*

# --- ARM bare-metal toolchain (arm-none-eabi) ---
ARG TOOLCHAIN_VERSION=13.2.Rel1
ARG TOOLCHAIN=arm-gnu-toolchain-${TOOLCHAIN_VERSION}-x86_64-arm-none-eabi
RUN wget -q https://developer.arm.com/-/media/Files/downloads/gnu/${TOOLCHAIN_VERSION}/binrel/${TOOLCHAIN}.tar.xz \
    && mkdir -p /opt && tar xf ${TOOLCHAIN}.tar.xz -C /opt \
    && rm ${TOOLCHAIN}.tar.xz
ENV PATH="/opt/${TOOLCHAIN}/bin:${PATH}"

# --- MaximSDK (MSDK): Libraries/, CMSIS, BSPs, build .mk files ---
ARG MSDK_VERSION=v2024_10
RUN git clone --depth 1 --branch ${MSDK_VERSION} \
        https://github.com/analogdevicesinc/msdk.git /opt/MaximSDK
ENV MAXIM_PATH=/opt/MaximSDK

WORKDIR /work
