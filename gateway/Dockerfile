FROM debian:bookworm-slim AS build
RUN apt-get update && apt-get install -y --no-install-recommends \
      build-essential cmake git ca-certificates && \
    rm -rf /var/lib/apt/lists/*
WORKDIR /src
COPY . .
RUN cmake -S . -B build -DCMAKE_BUILD_TYPE=Release && \
    cmake --build build -j"$(nproc)"

FROM debian:bookworm-slim
RUN useradd -u 10001 -m gateway
WORKDIR /app
COPY --from=build /src/build/gateway /app/gateway
COPY config.yaml /app/config.yaml
USER gateway
EXPOSE 8080
ENTRYPOINT ["/app/gateway"]
