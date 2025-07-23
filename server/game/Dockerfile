FROM --platform=$BUILDPLATFORM ghcr.io/boyism80/fb/build AS build

# Build type argument - Release as default for Dockerfile
ARG BUILD_TYPE=Release

WORKDIR /app
COPY include ./include
COPY game ./server/game
COPY resources/maps/maps.zip ./maps.zip

RUN apt-get install zip unzip -y
RUN mkdir -p maps && unzip -qq maps.zip -d ./maps && rm -f maps.zip

WORKDIR /app/game/build
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
COPY --from=build /app/game/build/app .
COPY --from=build /app/maps/ ./maps/
COPY game/scripts/ ./scripts/
COPY --from=build /fb/data/json/server/ ./json/

WORKDIR /app
CMD ["./app"]
