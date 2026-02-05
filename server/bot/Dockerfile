FROM --platform=$BUILDPLATFORM ghcr.io/boyism80/fb/build AS build

# Build type argument - Release as default for Dockerfile
ARG BUILD_TYPE=Release

WORKDIR /app
COPY include ./include
COPY server/gateway/lib/protocol ./server/gateway/lib/protocol
COPY server/login/lib/protocol ./server/login/lib/protocol
COPY server/game/lib/protocol ./server/game/lib/protocol
COPY server/bot ./bot

WORKDIR /app/bot/build
RUN cmake -DCMAKE_BUILD_TYPE=${BUILD_TYPE} ..
RUN cmake --build . --config ${BUILD_TYPE} --parallel 32

FROM --platform=$BUILDPLATFORM ghcr.io/boyism80/fb/build
WORKDIR /app
COPY --from=build /app/bot/build/app ./bot
COPY --from=ghcr.io/boyism80/fb/data:latest /fb/data/json/server/ /app/json/

WORKDIR /app
CMD ["./bot"]