FROM ubuntu:22.04 AS builder

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    git \
    libpqxx-dev \
    libpq-dev \
    libx11-dev \
    libxrandr-dev \
    libxcursor-dev \
    libxi-dev \
    libudev-dev \
    libgl1-mesa-dev \
    libfreetype6-dev \
    libopenal-dev \
    libflac-dev \
    libvorbis-dev \
    ca-certificates \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app

COPY . .

RUN mkdir -p build && cd build && \
    cmake -DENABLE_AUTH=ON -DCMAKE_BUILD_TYPE=Release .. && \
    make -j$(nproc)

FROM ubuntu:22.04

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y \
    libpqxx-6.4 \
    libpq5 \
    libx11-6 \
    libxrandr2 \
    libxcursor1 \
    libxi6 \
    libudev1 \
    libgl1 \
    libfreetype6 \
    libopenal1 \
    libflac8 \
    libvorbis0a \
    libvorbisenc2 \
    libsfml-dev \
    fonts-dejavu \
    fonts-liberation \
    fonts-freefont-ttf \
    xauth \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app

COPY --from=builder /app/build/nbody_simulation /app/nbody_simulation

ENV DISPLAY=:0

CMD ["./nbody_simulation"]
