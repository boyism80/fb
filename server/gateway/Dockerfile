ARG BUILD_IMAGE=ghcr.io/boyism80/fb/build

FROM --platform=$BUILDPLATFORM ${BUILD_IMAGE} AS build

# Build type argument - Release as default for Dockerfile
ARG BUILD_TYPE=Release

WORKDIR /app
COPY include ./include
COPY server/gateway ./gateway

WORKDIR /app/gateway/build
RUN cmake -DCMAKE_BUILD_TYPE=${BUILD_TYPE} ..
RUN cmake --build . --config ${BUILD_TYPE} --parallel 12

FROM --platform=$BUILDPLATFORM ${BUILD_IMAGE}
WORKDIR /app
COPY --from=build /app/gateway/build/app ./gateway

WORKDIR /app
CMD ["./gateway"]
