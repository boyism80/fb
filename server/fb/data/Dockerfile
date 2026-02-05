FROM --platform=$BUILDPLATFORM mcr.microsoft.com/dotnet/sdk:8.0 AS data-build
WORKDIR /
COPY resources/table /app/resources/table
COPY tools/data-converter /app/tools/data-converter
WORKDIR /app/tools/data-converter
RUN dotnet publish -c Release /p:DefineConstants=DISABLED_TTY -o bin
WORKDIR /app/tools/data-converter/bin
RUN ./ExcelTableConverter --dir=/app/resources/table --lang="c++|c#" --dsl=/app/resources/table/dsl.json --ns fb.model --additional-headers=model.additional.h
RUN mv output /output

FROM --platform=$BUILDPLATFORM alpine:latest
WORKDIR /fb/data
COPY --from=data-build /output/json/server ./json/server
