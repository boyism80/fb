FROM cshyeon/fb:latest
WORKDIR app
COPY . .
RUN chmod 755 ./gateway
ENTRYPOINT ["./gateway"]