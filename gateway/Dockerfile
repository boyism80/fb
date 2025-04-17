FROM --platform=$BUILDPLATFORM ghcr.io/boyism80/fb/build AS build

WORKDIR /app
COPY include ./include
COPY gateway ./gateway

WORKDIR /app/gateway/build
RUN cmake ..
RUN cmake --build . --config Release --parallel

FROM --platform=$BUILDPLATFORM ghcr.io/boyism80/fb/build
WORKDIR /app
COPY --from=build /app/gateway/build/app .

WORKDIR /app
CMD ["./app"]