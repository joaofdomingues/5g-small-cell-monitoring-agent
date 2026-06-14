"""
simulate_5g_metrics.py — Generate simulated 5G small cell metrics.

Writes a JSON config file that the C++ agent reads. Supports healthy,
degraded, and random modes. All public functions are fully type-annotated.

Usage:
  python scripts/simulate_5g_metrics.py --mode healthy
  python scripts/simulate_5g_metrics.py --mode degraded --output config/degraded_config.json
  python scripts/simulate_5g_metrics.py --mode random
"""

from __future__ import annotations

import argparse
import json
import random
from dataclasses import asdict, dataclass
from pathlib import Path
from typing import Literal

Mode = Literal["healthy", "degraded", "random"]
DEFAULT_OUTPUT = Path("config/agent_config.json")


# ---------------------------------------------------------------------------
# Data model
# ---------------------------------------------------------------------------

@dataclass
class RadioMetrics:
    """Radio-layer metrics for one 5G small cell."""
    rsrp:         int     # Reference Signal Received Power   (dBm)
    rsrq:         int     # Reference Signal Received Quality  (dB)
    sinr:         int     # Signal-to-Interference-plus-Noise  (dB)
    latency_ms:   int     # Round-trip latency                 (ms)
    packet_loss:  float   # Packet loss rate                   (%)
    status:       str     # "ACTIVE" | "DEGRADED" | "INACTIVE"


@dataclass
class AgentConfigPayload:
    """Full JSON payload written to agent_config.json."""
    cell_id:          str
    technology:       str
    rsrp:             int
    rsrq:             int
    sinr:             int
    latency_ms:       int
    packet_loss:      float
    status:           str
    ssh_enabled:      bool
    min_sinr:         int   = 10
    min_rsrp:         int   = -100
    max_packet_loss:  float = 1.0
    max_latency_ms:   int   = 50
    log_file:         str   = "logs/agent.log"
    poll_interval_sec: int  = 5
    daemon_mode:      bool  = False


# ---------------------------------------------------------------------------
# Metric generators
# ---------------------------------------------------------------------------

def _healthy_radio() -> RadioMetrics:
    return RadioMetrics(
        rsrp        = random.randint(-98, -78),
        rsrq        = random.randint(-13, -8),
        sinr        = random.randint(15, 30),
        latency_ms  = random.randint(8, 35),
        packet_loss = round(random.uniform(0.0, 0.8), 2),
        status      = "ACTIVE",
    )


def _degraded_radio() -> RadioMetrics:
    return RadioMetrics(
        rsrp        = random.randint(-118, -105),
        rsrq        = random.randint(-20, -15),
        sinr        = random.randint(2, 8),
        latency_ms  = random.randint(65, 120),
        packet_loss = round(random.uniform(1.3, 4.0), 2),
        status      = random.choice(["DEGRADED", "INACTIVE"]),
    )


def build_config(mode: Mode) -> AgentConfigPayload:
    """Build a full agent config for the given mode."""
    effective_mode: Literal["healthy", "degraded"] = (
        random.choice(["healthy", "degraded"]) if mode == "random" else mode
    )
    radio = _healthy_radio() if effective_mode == "healthy" else _degraded_radio()

    return AgentConfigPayload(
        cell_id     = "AVEIRO-SC-001" if effective_mode == "healthy" else "AVEIRO-SC-002",
        technology  = "5G NR",
        rsrp        = radio.rsrp,
        rsrq        = radio.rsrq,
        sinr        = radio.sinr,
        latency_ms  = radio.latency_ms,
        packet_loss = radio.packet_loss,
        status      = radio.status,
        ssh_enabled = effective_mode == "healthy",
    )


def write_config(config: AgentConfigPayload, output: Path) -> None:
    """Serialise config to JSON and write to *output*."""
    output.parent.mkdir(parents=True, exist_ok=True)
    output.write_text(json.dumps(asdict(config), indent=2), encoding="utf-8")


# ---------------------------------------------------------------------------
# Entry point
# ---------------------------------------------------------------------------

def main() -> None:
    parser = argparse.ArgumentParser(
        description="Generate simulated 5G small cell metrics for the C++ agent."
    )
    parser.add_argument(
        "--mode",
        choices=["healthy", "degraded", "random"],
        default="healthy",
        help="Metric profile to generate (default: healthy)",
    )
    parser.add_argument(
        "--output",
        default=str(DEFAULT_OUTPUT),
        help=f"Output JSON path (default: {DEFAULT_OUTPUT})",
    )
    args = parser.parse_args()

    config = build_config(args.mode)
    output = Path(args.output)
    write_config(config, output)

    print(f"[simulate_5g_metrics] Generated '{args.mode}' config → {output}")
    print(json.dumps(asdict(config), indent=2))


if __name__ == "__main__":
    main()
