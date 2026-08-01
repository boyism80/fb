ARG BUILD_IMAGE=ghcr.io/boyism80/fb/build
ARG DATA_IMAGE=ghcr.io/boyism80/fb/data:latest

FROM --platform=$BUILDPLATFORM ${BUILD_IMAGE} AS build

# Build type argument - Release as default for Dockerfile
ARG BUILD_TYPE=Release

WORKDIR /app
COPY include ./include
COPY server/login ./login

WORKDIR /app/login/build
RUN cmake -DCMAKE_BUILD_TYPE=${BUILD_TYPE} ..
RUN cmake --build . --config ${BUILD_TYPE} --parallel 12

FROM ${DATA_IMAGE} AS data

FROM --platform=$BUILDPLATFORM ${BUILD_IMAGE}
WORKDIR /app
COPY --from=build /app/login/build/app ./login
COPY --from=data /fb/data/json/server/ /app/json/

WORKDIR /app
CMD ["./login"]
