ARG BUILD_IMAGE=ghcr.io/boyism80/fb/build
ARG DATA_IMAGE=ghcr.io/boyism80/fb/data:latest

FROM --platform=$BUILDPLATFORM ${BUILD_IMAGE} AS build

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
RUN cmake --build . --config ${BUILD_TYPE} --parallel 12

FROM ${DATA_IMAGE} AS data

FROM --platform=$BUILDPLATFORM ${BUILD_IMAGE}

WORKDIR /app
COPY --from=build /app/game/build/app ./game
COPY --from=build /app/maps/ ./maps/
COPY server/game/scripts/ ./scripts/
COPY --from=data /fb/data/json/server/ ./json/

WORKDIR /app
CMD ["./game"]
