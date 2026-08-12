# CI/CD Admin Handoff — org-pattern prerequisites

The firmware-repo side is complete (org-pattern workflows: `build.yml`, `clang-check.yml`,
`deploy.yml`, `preprocess.yml`). These consume shared infrastructure that only an **org
admin/maintainer** can provision. Until the items below are done, the `build`/`release`
checks stay red (they pull an image that does not exist yet).

## 1. Publish an MSDK toolchain image to the org registry (required)
`ci-workflows` currently publishes only an STM32 image. Add a MAX78000/MSDK image and publish it
so firmware repos can `container:`-pull it. Suggested files in **`ci-workflows`**:

### `docker/max78000/Dockerfile`
```dockerfile
FROM ubuntu:24.04
ARG DEBIAN_FRONTEND=noninteractive

# Base tooling + ccache (ccache is required by the ccache-artifact-location action)
RUN apt-get update && apt-get install -y --no-install-recommends \
        git make wget xz-utils ca-certificates clang-format python3 ccache \
    && ln -sf /usr/bin/python3 /usr/local/bin/python \
    && rm -rf /var/lib/apt/lists/*

# ARM bare-metal toolchain
ARG TOOLCHAIN_VERSION=13.2.Rel1
ARG TOOLCHAIN=arm-gnu-toolchain-${TOOLCHAIN_VERSION}-x86_64-arm-none-eabi
RUN wget -q https://developer.arm.com/-/media/Files/downloads/gnu/${TOOLCHAIN_VERSION}/binrel/${TOOLCHAIN}.tar.xz \
    && mkdir -p /opt && tar xf ${TOOLCHAIN}.tar.xz -C /opt && rm ${TOOLCHAIN}.tar.xz
ENV GCC_ARM_PATH=/opt/${TOOLCHAIN}/bin

# MaximSDK
ARG MSDK_VERSION=v2024_10
RUN git clone --depth 1 --branch ${MSDK_VERSION} https://github.com/analogdevicesinc/msdk.git /opt/MaximSDK
ENV MAXIM_PATH=/opt/MaximSDK

# ccache masquerade (so the shared ccache-artifact-location action accelerates builds)
ARG CACHE_WS=/cache_ws
ENV CACHE_OUTPUT=$CACHE_WS/.ccache
ARG CCACHE_LINKS=$CACHE_WS/ccache-bin
WORKDIR $CCACHE_LINKS
RUN for tool in $(basename -a $GCC_ARM_PATH/*); do ln -s $(which ccache) $tool; done
ENV PATH="$CCACHE_LINKS:$PATH:$GCC_ARM_PATH"
ENV CCACHE_CONFIGPATH=$CACHE_WS/ccache.conf
RUN ccache -o cache_dir=$CACHE_OUTPUT

WORKDIR /work
```

### `.github/workflows/max78000-docker-push.yml` (in ci-workflows)
Mirror the existing `firmware-docker-push.yml`, publishing to
`ghcr.io/naqilogix/max78000-toolchain:v2024_10` on push to the default branch + weekly.

> Confirm the exact registry namespace: existing images use both `ghcr.io/wisear/...` and
> `ghcr.io/naqilogix/...`. The firmware-repo workflows reference `ghcr.io/naqilogix/max78000-toolchain:v2024_10`;
> update the image ref in `build.yml`/`deploy.yml` if the org standardizes on a different namespace/tag.

## 2. Grant this repo access to the shared workflows/image (required)
`build.yml`/`deploy.yml`/`clang-check.yml`/`preprocess.yml` reference `NaqiLogix/ci-workflows/...@master`
and pull `ghcr.io/naqilogix/...`. This works when the firmware repo is **inside the org** (or the
org grants the repo package/actions access). A personal repo cannot read private org packages or call
private org reusable workflows. **Move/transfer `dsp-firmware-naqi` into the NaqiLogix org** (or grant access).

## 3. Branch protection (recommended)
On the default branch, require the `build` and clang `format` checks to pass before merge.

## What is already done on the repo side
- Org-pattern workflows in place (pull central image, reuse ci-workflows actions, semver tags).
- Source formatted to the team `.clang-format` (clang-format 15) so the blocking clang check passes.
- Case-sensitivity and FreeRTOS stack-overflow-hook issues already fixed (build compiles debug + release).
- `.github/labeler.yml` added so PR labeling → changelog categories work.
- The self-contained image lifecycle (`docker/`, `max78000-docker-*.yml`) has been removed — the image now lives in `ci-workflows`.
