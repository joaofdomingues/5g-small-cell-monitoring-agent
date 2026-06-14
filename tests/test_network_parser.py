"""
Tests for simulate_5g_metrics.py — config generation logic.
"""

from __future__ import annotations

import json
import tempfile
from pathlib import Path

import pytest

from scripts.simulate_5g_metrics import (
    AgentConfigPayload,
    Mode,
    build_config,
    write_config,
)

REQUIRED_FIELDS = [
    "cell_id", "technology", "rsrp", "rsrq", "sinr",
    "latency_ms", "packet_loss", "status", "ssh_enabled",
    "min_sinr", "min_rsrp", "max_packet_loss", "max_latency_ms",
    "log_file", "poll_interval_sec", "daemon_mode",
]


# ---------------------------------------------------------------------------
# build_config — healthy
# ---------------------------------------------------------------------------

class TestHealthyConfig:
    def test_status_is_active(self) -> None:
        assert build_config("healthy").status == "ACTIVE"

    def test_ssh_enabled(self) -> None:
        assert build_config("healthy").ssh_enabled is True

    def test_sinr_above_threshold(self) -> None:
        cfg = build_config("healthy")
        assert cfg.sinr >= cfg.min_sinr

    def test_rsrp_above_threshold(self) -> None:
        cfg = build_config("healthy")
        assert cfg.rsrp >= cfg.min_rsrp

    def test_packet_loss_below_threshold(self) -> None:
        cfg = build_config("healthy")
        assert cfg.packet_loss <= cfg.max_packet_loss

    def test_latency_below_threshold(self) -> None:
        cfg = build_config("healthy")
        assert cfg.latency_ms <= cfg.max_latency_ms


# ---------------------------------------------------------------------------
# build_config — degraded
# ---------------------------------------------------------------------------

class TestDegradedConfig:
    def test_status_is_not_active(self) -> None:
        assert build_config("degraded").status != "ACTIVE"

    def test_ssh_disabled(self) -> None:
        assert build_config("degraded").ssh_enabled is False

    def test_sinr_below_threshold(self) -> None:
        cfg = build_config("degraded")
        assert cfg.sinr < cfg.min_sinr

    def test_rsrp_below_threshold(self) -> None:
        cfg = build_config("degraded")
        assert cfg.rsrp < cfg.min_rsrp

    def test_packet_loss_above_threshold(self) -> None:
        cfg = build_config("degraded")
        assert cfg.packet_loss > cfg.max_packet_loss

    def test_latency_above_threshold(self) -> None:
        cfg = build_config("degraded")
        assert cfg.latency_ms > cfg.max_latency_ms


# ---------------------------------------------------------------------------
# build_config — random
# ---------------------------------------------------------------------------

class TestRandomConfig:
    def test_random_produces_valid_config(self) -> None:
        cfg = build_config("random")
        assert cfg.status in {"ACTIVE", "DEGRADED", "INACTIVE"}

    def test_random_has_all_required_fields(self) -> None:
        import dataclasses
        cfg = build_config("random")
        keys = {f.name for f in dataclasses.fields(cfg)}
        assert set(REQUIRED_FIELDS).issubset(keys)


# ---------------------------------------------------------------------------
# write_config
# ---------------------------------------------------------------------------

class TestWriteConfig:
    def test_writes_valid_json(self) -> None:
        with tempfile.TemporaryDirectory() as tmp:
            out = Path(tmp) / "test_config.json"
            cfg = build_config("healthy")
            write_config(cfg, out)
            data = json.loads(out.read_text())
            for field in REQUIRED_FIELDS:
                assert field in data, f"Missing field: {field}"

    def test_creates_parent_dirs(self) -> None:
        with tempfile.TemporaryDirectory() as tmp:
            out = Path(tmp) / "nested" / "dir" / "config.json"
            write_config(build_config("healthy"), out)
            assert out.exists()

    def test_json_is_pretty_printed(self) -> None:
        with tempfile.TemporaryDirectory() as tmp:
            out = Path(tmp) / "config.json"
            write_config(build_config("healthy"), out)
            content = out.read_text()
            assert "\n" in content  # pretty-printed, not one-liner
