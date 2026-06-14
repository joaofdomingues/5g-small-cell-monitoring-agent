# ---------------------------------------------------------------------------
# 5G Small Cell Monitoring Agent — Dockerfile
#
# Multi-stage build:
#   builder  →  compiles C++ with all tests
#   runtime  →  minimal image with only the agent binary + Python scripts
#
# Build:
#   docker build -t 5g-agent .
#
# Run (one-shot):
#   docker run --rm 5g-agent --once
#
# Run (continuous, with SIGTERM support):
#   docker run --rm --init 5g-agent
#   docker stop <container>   ← sends SIGTERM → graceful shutdown
#
# Note: /proc metrics are read from the container's own /proc namespace.
# On Linux hosts this reflects the host kernel; on macOS/Windows (Docker Desktop)
# values will reflect the VM.
# ---------------------------------------------------------------------------

# ── Stage 1: builder ───────────────────────────────────────────────────────
FROM ubuntu:22.04 AS builder

RUN apt-get update && apt-get install -y --no-install-recommends \
        build-essential cmake git ca-certificates \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /build
COPY . .

RUN cmake -B build \
        -DENABLE_GTEST=ON \
        -DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
        -DCMAKE_BUILD_TYPE=Release \
    && cmake --build build --parallel "$(nproc)"

# Run all C++ tests during build — fail the image if any test fails.
RUN cd build && ctest --output-on-failure

# ── Stage 2: runtime ───────────────────────────────────────────────────────
FROM ubuntu:22.04 AS runtime

RUN apt-get update && apt-get install -y --no-install-recommends \
        python3 python3-pip openssh-client \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app

# Copy compiled binary
COPY --from=builder /build/build/5g_agent ./5g_agent

# Copy Python scripts, config and requirements
COPY scripts/  ./scripts/
COPY tests/    ./tests/
COPY config/   ./config/
COPY requirements.txt conftest.py ./

RUN python3 -m pip install --no-cache-dir -r requirements.txt

# Create log directory
RUN mkdir -p logs

# Use --init (tini) in docker run for proper signal forwarding.
ENTRYPOINT ["./5g_agent"]
CMD ["--once", "--config", "config/agent_config.json"]
