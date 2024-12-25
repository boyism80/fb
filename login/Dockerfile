FROM ghcr.io/boyism80/fb/build AS build

WORKDIR /app
COPY include ./include
COPY lib/include ./lib/include
COPY login ./login

WORKDIR /app/login/build
RUN cmake ..
RUN cmake --build . --config Dockerfile --parallel

FROM ghcr.io/boyism80/fb/build
WORKDIR /app
COPY --from=build /app/login/build/app .
COPY --from=build /fb/data/json/server/ /app/json/

WORKDIR /app
CMD ./app