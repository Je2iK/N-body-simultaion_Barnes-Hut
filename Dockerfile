# Build stage
FROM ubuntu:22.04 AS builder

# Set non-interactive mode for apt
ENV DEBIAN_FRONTEND=noninteractive

# Install build dependencies
RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    git \
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

# Set working directory
WORKDIR /app

# Copy source files
COPY . .

# Build the application
RUN mkdir -p build && cd build && \
    cmake -DENABLE_AUTH=OFF -DCMAKE_BUILD_TYPE=Release .. && \
    make -j$(nproc)

# Runtime stage
FROM ubuntu:22.04

# Set non-interactive mode for apt
ENV DEBIAN_FRONTEND=noninteractive

# Install runtime dependencies
RUN apt-get update && apt-get install -y \
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
    libsfml-dev \
    fonts-dejavu \
    fonts-liberation \
    fonts-freefont-ttf \
    xauth \
    && rm -rf /var/lib/apt/lists/*

# Set working directory
WORKDIR /app

# Copy built executable from builder
COPY --from=builder /app/build/nbody_simulation /app/nbody_simulation

# Set display for X11
ENV DISPLAY=:0

# Run the application
CMD ["./nbody_simulation"]
