"""
ssh_health_check.py — SSH diagnostics for a 5G small cell node.

Supports two modes:
  --dry-run   Run commands locally (no SSH needed; safe for CI and demos).
  (default)   Connect to the target node via Paramiko and run the same
              diagnostic commands remotely.

Usage examples:
  python scripts/ssh_health_check.py --dry-run
  python scripts/ssh_health_check.py --host 192.168.10.21 --user admin
  python scripts/ssh_health_check.py --host 192.168.10.21 --user admin --key ~/.ssh/id_rsa
"""

from __future__ import annotations

import argparse
import subprocess
import sys
from dataclasses import dataclass, field


# ---------------------------------------------------------------------------
# Data structures
# ---------------------------------------------------------------------------

@dataclass
class DiagResult:
    label: str
    command: str
    output: str
    success: bool


# ---------------------------------------------------------------------------
# Diagnostic commands
# ---------------------------------------------------------------------------

DIAG_COMMANDS: list[tuple[str, str]] = [
    ("hostname",  "hostname"),
    ("uptime",    "uptime"),
    ("disk",      "df -h / | tail -1"),
    ("network",   "ip addr show | head -20"),
    ("processes", "ps aux --sort=-%cpu | head -6"),
]


# ---------------------------------------------------------------------------
# Local execution (dry-run mode)
# ---------------------------------------------------------------------------

def run_local(label: str, command: str) -> DiagResult:
    try:
        result = subprocess.run(
            command, shell=True, capture_output=True, text=True, timeout=10
        )
        return DiagResult(
            label=label,
            command=command,
            output=result.stdout.strip() or result.stderr.strip() or "(no output)",
            success=(result.returncode == 0),
        )
    except Exception as exc:
        return DiagResult(label=label, command=command, output=str(exc), success=False)


# ---------------------------------------------------------------------------
# Remote execution via Paramiko
# ---------------------------------------------------------------------------

def run_remote(host: str, user: str, key_path: str | None,
               password: str | None, label: str, command: str) -> DiagResult:
    """Execute *command* on *host* via Paramiko SSH and return the result."""
    try:
        import paramiko  # Local import so dry-run works even without paramiko.
    except ImportError:
        return DiagResult(
            label=label, command=command,
            output="paramiko not installed — run: pip install paramiko",
            success=False,
        )

    client = paramiko.SSHClient()
    client.set_missing_host_key_policy(paramiko.AutoAddPolicy())  # noqa: S507 — lab env

    try:
        connect_kwargs: dict = dict(hostname=host, username=user, timeout=10)
        if key_path:
            connect_kwargs["key_filename"] = key_path
        elif password:
            connect_kwargs["password"] = password
        else:
            connect_kwargs["look_for_keys"] = True

        client.connect(**connect_kwargs)
        _, stdout, stderr = client.exec_command(command, timeout=10)
        out = stdout.read().decode().strip()
        err = stderr.read().decode().strip()
        rc  = stdout.channel.recv_exit_status()

        return DiagResult(
            label=label,
            command=command,
            output=out or err or "(no output)",
            success=(rc == 0),
        )
    except Exception as exc:
        return DiagResult(label=label, command=command, output=str(exc), success=False)
    finally:
        client.close()


# ---------------------------------------------------------------------------
# Report
# ---------------------------------------------------------------------------

def print_report(results: list[DiagResult], host: str) -> None:
    print(f"\n{'='*60}")
    print(f"  5G Small Cell SSH Diagnostics — {host}")
    print(f"{'='*60}\n")
    for r in results:
        status = "OK" if r.success else "FAIL"
        print(f"[{r.label.upper():<12}] [{status}]")
        for line in r.output.splitlines():
            print(f"    {line}")
        print()

    failed = [r for r in results if not r.success]
    if failed:
        print(f"SUMMARY: {len(failed)}/{len(results)} check(s) failed.")
        sys.exit(1)
    else:
        print(f"SUMMARY: All {len(results)} check(s) passed.")


# ---------------------------------------------------------------------------
# Entry point
# ---------------------------------------------------------------------------

def main() -> None:
    parser = argparse.ArgumentParser(
        description="SSH diagnostics for a 5G small cell node."
    )
    parser.add_argument("--host",     default="localhost",
                        help="Target node hostname or IP (default: localhost)")
    parser.add_argument("--user",     default="user",
                        help="SSH username (default: user)")
    parser.add_argument("--key",      default=None,
                        help="Path to private key file")
    parser.add_argument("--password", default=None,
                        help="SSH password (not recommended; use key auth)")
    parser.add_argument("--dry-run",  action="store_true",
                        help="Run diagnostics locally — no SSH connection made")
    args = parser.parse_args()

    results: list[DiagResult] = []
    is_local = args.dry_run or args.host in {"localhost", "127.0.0.1"}

    if is_local:
        print(f"[DRY-RUN] Running diagnostics locally (target would be {args.host})")
        for label, cmd in DIAG_COMMANDS:
            results.append(run_local(label, cmd))
    else:
        print(f"[SSH] Connecting to {args.user}@{args.host} …")
        for label, cmd in DIAG_COMMANDS:
            results.append(
                run_remote(args.host, args.user, args.key, args.password, label, cmd)
            )

    print_report(results, host=args.host if not is_local else "localhost (dry-run)")


if __name__ == "__main__":
    main()
