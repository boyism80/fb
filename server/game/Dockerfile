FROM --platform=$BUILDPLATFORM ghcr.io/boyism80/fb/build AS build

# Build type argument - Release as default for Dockerfile
ARG BUILD_TYPE=Release

WORKDIR /app
COPY include ./include
COPY server/game ./game
COPY resources/maps/maps.zip ./maps.zip

RUN apt-get install zip unzip -y
RUN mkdir -p maps && unzip -qq maps.zip -d ./maps && rm -f maps.zip

WORKDIR /app/game/build
RUN cmake -DCMAKE_BUILD_TYPE=${BUILD_TYPE} ..
RUN cmake --build . --config ${BUILD_TYPE} --parallel 32

FROM --platform=$BUILDPLATFORM ghcr.io/boyism80/fb/build

WORKDIR /app
COPY --from=build /app/game/build/app ./game
COPY --from=build /app/maps/ ./maps/
COPY server/game/scripts/ ./scripts/
COPY --from=ghcr.io/boyism80/fb/data:latest /fb/data/json/server/ ./json/

WORKDIR /app
CMD ["./game"]
