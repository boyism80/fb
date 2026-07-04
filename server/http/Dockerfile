ARG DATA_IMAGE=ghcr.io/boyism80/fb/data:latest

FROM --platform=$BUILDPLATFORM mcr.microsoft.com/dotnet/sdk:8.0 AS build
ARG SERVICE
ARG BUILD_TYPE=Release
ENV SERVICE=$SERVICE

WORKDIR /app
COPY server/http ./http
COPY server/${SERVICE} ./${SERVICE}
RUN dotnet publish -c ${BUILD_TYPE} -o /app/dist/${SERVICE} ./${SERVICE}

FROM ${DATA_IMAGE} AS data

FROM --platform=$BUILDPLATFORM mcr.microsoft.com/dotnet/aspnet:8.0
ARG SERVICE
ENV SERVICE=$SERVICE
COPY --from=build /app/dist/${SERVICE} /app
COPY --from=data /fb/data/json/server/ /app/json/
COPY infra/db/migrations /app/migrations
WORKDIR /app
ENTRYPOINT ["sh", "-c", "dotnet ${SERVICE}.dll"]
