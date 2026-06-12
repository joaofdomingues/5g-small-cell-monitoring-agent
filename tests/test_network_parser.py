import json
from pathlib import Path


def test_config_file_contains_required_fields():
    path = Path("config/agent_config.json")
    data = json.loads(path.read_text(encoding="utf-8"))

    required = [
        "cell_id",
        "technology",
        "rsrp",
        "rsrq",
        "sinr",
        "latency_ms",
        "packet_loss",
        "status",
        "ssh_enabled",
        "min_sinr",
        "min_rsrp",
        "max_packet_loss",
        "max_latency_ms",
        "log_file",
    ]
    for field in required:
        assert field in data


def test_degraded_config_is_useful_for_failure_scenarios():
    path = Path("config/degraded_config.json")
    data = json.loads(path.read_text(encoding="utf-8"))

    assert data["sinr"] < data["min_sinr"]
    assert data["rsrp"] < data["min_rsrp"]
    assert data["packet_loss"] > data["max_packet_loss"]
    assert data["latency_ms"] > data["max_latency_ms"]
    assert data["status"] != "ACTIVE"
    assert data["ssh_enabled"] is False
