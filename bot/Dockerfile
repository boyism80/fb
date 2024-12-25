FROM ghcr.io/boyism80/fb/build AS build

WORKDIR /app
COPY include ./include
COPY lib/include ./lib/include
COPY bot ./bot

WORKDIR /app/bot/build
RUN cmake ..
RUN cmake --build . --config Release --parallel

FROM ghcr.io/boyism80/fb/build
WORKDIR /app
COPY --from=build /app/bot/build/app .
COPY --from=build /fb/data/json/server/ /app/json/

WORKDIR /app
CMD ./app