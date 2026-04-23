# --- STAGE 1: The Builder ---
FROM ubuntu:24.04 AS builder

ENV DEBIAN_FRONTEND=noninteractive
WORKDIR /build_env

# Install compilers and dev-headers
RUN apt update && \
    apt install -y \
    build-essential cmake clang-18 llvm-18 \
    qt6-base-dev qt6-declarative-dev qt6-charts-dev \
    libqt6charts6-dev libxkbcommon-dev libgl1-mesa-dev \
    libvulkan-dev libboost-all-dev \
    && rm -rf /var/lib/apt/lists/*

ENV CC=clang-18
ENV CXX=clang++-18

# Copy ONLY the CMakeLists.txt first
# This allows Docker to cache the 'environment setup'
COPY CMakeLists.txt ./

COPY . .

# Build everything
RUN cmake -B build -DCMAKE_BUILD_TYPE=Release && \
    cmake --build build --parallel $(nproc)

# We find the binaries and move them to a flat /dist folder.
RUN mkdir /dist && \
    # Search for 'drone_sim' because of the OUTPUT_NAME property in CMake
    find build -type f -name "drone_sim" -exec cp {} /dist/simulator \; && \
    find build -type f -name "run_tests" -exec cp {} /dist/run_tests \; && \
    chmod +x /dist/simulator /dist/run_tests

# --- STAGE 2: The Runtime ---
FROM ubuntu:24.04 AS runtime

RUN apt update && \
    apt install -y \
    qt6-base-dev \
    libqt6charts6 \
    libboost-system-dev \
    libxkbcommon0 \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app

# Now we just copy everything from our clean /dist folder!
COPY --from=builder /dist/ .

# Verify the files are there
RUN ls -la /app

# Default to the simulator for easier SITL testing
CMD ["./simulator"]
