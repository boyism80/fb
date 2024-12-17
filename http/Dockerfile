FROM ghcr.io/boyism80/fb/build AS data-build

FROM mcr.microsoft.com/dotnet/sdk:8.0 AS build
ARG SERVICE
ENV SERVICE=$SERVICE

WORKDIR /app
COPY http ./http
COPY ${SERVICE} ./${SERVICE}
RUN dotnet publish -c Release -o /app/dist/${SERVICE} ./${SERVICE}

FROM mcr.microsoft.com/dotnet/aspnet:8.0
ARG SERVICE
ENV SERVICE=$SERVICE
COPY --from=build /app/dist/${SERVICE} /app
COPY --from=data-build /fb/data/json/server/ /app/json/
WORKDIR /app
ENTRYPOINT dotnet ${SERVICE}.dll