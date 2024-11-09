FROM cshyeon/fb:build AS build

WORKDIR /app
COPY include ./include
COPY lib/include ./lib/include
COPY login ./login

WORKDIR /app/login/build
RUN cmake ..
RUN cmake --build . --config Release --parallel

FROM cshyeon/fb:build
WORKDIR /app
COPY --from=build /app/login/build/app .

WORKDIR /app
ENTRYPOINT ./app