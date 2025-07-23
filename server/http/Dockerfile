FROM --platform=$BUILDPLATFORM ghcr.io/boyism80/fb/build AS data-build

FROM --platform=$BUILDPLATFORM mcr.microsoft.com/dotnet/sdk:8.0 AS build
ARG SERVICE
ARG BUILD_TYPE=Release
ENV SERVICE=$SERVICE

WORKDIR /app
COPY http ./server/http
COPY ${SERVICE} ./server/${SERVICE}
RUN dotnet publish -c ${BUILD_TYPE} -o /app/dist/${SERVICE} ./${SERVICE}

FROM --platform=$BUILDPLATFORM mcr.microsoft.com/dotnet/aspnet:8.0
ARG SERVICE
ENV SERVICE=$SERVICE
COPY --from=build /app/dist/${SERVICE} /app
COPY --from=data-build /fb/data/json/server/ /app/json/
WORKDIR /app
ENTRYPOINT ["sh", "-c", "dotnet ${SERVICE}.dll"]
