FROM ubuntu:24.04

RUN apt-get update && apt-get install -y cmake g++ python3 python3-pip && rm -rf /var/lib/apt/lists/*

WORKDIR /app
COPY . .

RUN mkdir -p build && cd build && cmake .. && make

CMD ["./build/5g_agent", "config/agent_config.json"]
