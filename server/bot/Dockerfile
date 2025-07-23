FROM --platform=$BUILDPLATFORM ghcr.io/boyism80/fb/build AS build

# Build type argument - Release as default for Dockerfile
ARG BUILD_TYPE=Release

WORKDIR /app
COPY include ./include
COPY gateway/lib/protocol ./server/gateway/lib/protocol
COPY login/lib/protocol ./server/login/lib/protocol
COPY game/lib/protocol ./server/game/lib/protocol
COPY bot ./bot

WORKDIR /app/bot/build
RUN cmake -DCMAKE_BUILD_TYPE=${BUILD_TYPE} ..
RUN AVAILABLE_MEM_KB=$(grep MemAvailable /proc/meminfo | awk '{print $2}') && \
    MEM_PER_JOB_KB=$((1500 * 1024)) && \
    MAX_BY_MEM=$((AVAILABLE_MEM_KB / MEM_PER_JOB_KB)) && \
    CPU_CORES=$(nproc) && \
    if [ "$MAX_BY_MEM" -lt "$CPU_CORES" ]; then JOBS="$MAX_BY_MEM"; else JOBS="$CPU_CORES"; fi && \
    if [ "$JOBS" -lt 1 ]; then JOBS=1; fi && \
    echo "Building with ${JOBS} jobs (using Available Memory: ${AVAILABLE_MEM_KB} KB)" && \
    cmake --build . --config ${BUILD_TYPE} --parallel "$JOBS"

FROM --platform=$BUILDPLATFORM ghcr.io/boyism80/fb/build
WORKDIR /app
COPY --from=build /app/bot/build/app .
COPY --from=build /fb/data/json/server/ /app/json/

WORKDIR /app
CMD ["./app"]