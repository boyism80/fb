ARG DATA_IMAGE=ghcr.io/boyism80/fb/data:latest

FROM --platform=$BUILDPLATFORM mcr.microsoft.com/dotnet/sdk:8.0 AS build
ARG BUILD_TYPE=Release

WORKDIR /app
COPY server/http ./http
COPY server/matchmaking ./matchmaking
RUN dotnet publish -c ${BUILD_TYPE} -o /app/dist/matchmaking ./matchmaking

FROM ${DATA_IMAGE} AS data

FROM --platform=$BUILDPLATFORM mcr.microsoft.com/dotnet/aspnet:8.0
COPY --from=build /app/dist/matchmaking /app
COPY --from=data /fb/data/json/server/ /app/json/
WORKDIR /app
ENTRYPOINT ["dotnet", "matchmaking.dll"]
