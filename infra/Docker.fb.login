FROM cshyeon/fb:build AS build

WORKDIR /app
COPY include ./include
COPY lib/include ./lib/include
COPY plugin/linux ./plugin/linux
COPY login ./login

WORKDIR /app/login/build
RUN cmake ..
RUN cmake --build . --config Release --parallel 4

FROM cshyeon/fb:base
WORKDIR /app
COPY --from=build /app/login/build/app .
COPY --from=build /app/login/config/* ./config/

WORKDIR /app
ENTRYPOINT ./app