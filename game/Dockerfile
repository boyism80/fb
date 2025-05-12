FROM --platform=$BUILDPLATFORM ghcr.io/boyism80/fb/build AS build

WORKDIR /app
COPY include ./include
COPY game ./game
COPY resources/maps/maps.zip ./maps.zip

RUN apt-get install zip unzip -y
RUN mkdir -p maps && unzip -qq maps.zip -d ./maps && rm -f maps.zip

WORKDIR /app/game/build
RUN cmake -DCMAKE_BUILD_TYPE=Release ..
RUN cmake --build . --parallel

FROM --platform=$BUILDPLATFORM ghcr.io/boyism80/fb/build

WORKDIR /app
COPY --from=build /app/game/build/app ./game-server
COPY --from=build /app/maps/ ./maps/
COPY game/scripts/ ./scripts/
COPY --from=build /fb/data/json/server/ ./json/

WORKDIR /app
CMD ["./game-server"]
