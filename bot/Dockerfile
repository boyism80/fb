FROM --platform=$BUILDPLATFORM ghcr.io/boyism80/fb/build AS build

WORKDIR /app
COPY include ./include
COPY gateway/lib/protocol ./gateway/lib/protocol
COPY login/lib/protocol ./login/lib/protocol
COPY game/lib/protocol ./game/lib/protocol
COPY bot ./bot

WORKDIR /app/bot/build
RUN cmake ..
RUN cmake --build . --config Release --parallel
RUN AVAILABLE_MEM_KB=$(grep MemAvailable /proc/meminfo | awk '{print $2}') && \
    MEM_PER_JOB_KB=$((1500 * 1024)) && \
    MAX_BY_MEM=$((AVAILABLE_MEM_KB / MEM_PER_JOB_KB)) && \
    CPU_CORES=$(nproc) && \
    if [ "$MAX_BY_MEM" -lt "$CPU_CORES" ]; then JOBS="$MAX_BY_MEM"; else JOBS="$CPU_CORES"; fi && \
    if [ "$JOBS" -lt 1 ]; then JOBS=1; fi && \
    echo "Building with ${JOBS} jobs (using Available Memory: ${AVAILABLE_MEM_KB} KB)" && \
    cmake --build . --config Release --parallel "$JOBS"

FROM --platform=$BUILDPLATFORM ghcr.io/boyism80/fb/build
WORKDIR /app
COPY --from=build /app/bot/build/app .
COPY --from=build /fb/data/json/server/ /app/json/

WORKDIR /app
CMD ["./app"]