import argparse
import json
import random
from pathlib import Path

DEFAULT_OUTPUT = Path("config/agent_config.json")


def build_metrics(mode: str) -> dict:
    base = {
        "cell_id": "AVEIRO-SC-001" if mode == "healthy" else "AVEIRO-SC-002",
        "technology": "5G",
        "ssh_enabled": mode == "healthy",
        "min_sinr": 10,
        "min_rsrp": -100,
        "max_packet_loss": 1.0,
        "max_latency_ms": 50,
        "log_file": "logs/agent.log",
    }

    if mode == "healthy":
        base.update(
            {
                "rsrp": random.randint(-98, -78),
                "rsrq": random.randint(-13, -8),
                "sinr": random.randint(15, 30),
                "latency_ms": random.randint(8, 35),
                "packet_loss": round(random.uniform(0.0, 0.8), 2),
                "status": "ACTIVE",
            }
        )
    else:
        base.update(
            {
                "rsrp": random.randint(-118, -105),
                "rsrq": random.randint(-20, -15),
                "sinr": random.randint(2, 8),
                "latency_ms": random.randint(65, 120),
                "packet_loss": round(random.uniform(1.3, 4.0), 2),
                "status": random.choice(["DEGRADED", "INACTIVE"]),
            }
        )
    return base


def main() -> None:
    parser = argparse.ArgumentParser(description="Generate simulated 5G small cell metrics")
    parser.add_argument("--mode", choices=["healthy", "degraded"], default="healthy")
    parser.add_argument("--output", default=str(DEFAULT_OUTPUT))
    args = parser.parse_args()

    output = Path(args.output)
    metrics = build_metrics(args.mode)
    output.write_text(json.dumps(metrics, indent=2), encoding="utf-8")

    print(f"Generated {args.mode} simulated telecom metrics at {output}")
    print(json.dumps(metrics, indent=2))


if __name__ == "__main__":
    main()
