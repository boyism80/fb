FROM cshyeon/fb:build AS build

WORKDIR /app
COPY include ./include
COPY lib/include ./lib/include
COPY game ./game

WORKDIR /app/game/build
RUN cmake ..
RUN cmake --build . --config Release --parallel

FROM cshyeon/fb:build
RUN apt-get install zip unzip -y

WORKDIR /app
COPY --from=build /app/game/build/app .
COPY game/scripts/ /app/scripts/
COPY resources/maps/maps.zip /app/maps/maps.zip
WORKDIR /app/maps
RUN unzip -qq maps.zip
RUN rm -f maps.zip
COPY --from=build /fb/data/json/server/ /app/json/

WORKDIR /app
CMD ./app