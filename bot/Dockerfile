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

FROM --platform=$BUILDPLATFORM ghcr.io/boyism80/fb/build
WORKDIR /app
COPY --from=build /app/bot/build/app .
COPY --from=build /fb/data/json/server/ /app/json/

WORKDIR /app
CMD ["./app"]