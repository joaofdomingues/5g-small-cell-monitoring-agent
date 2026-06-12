import argparse
import subprocess

COMMANDS = {
    "hostname": "hostname",
    "disk": "df -h / | tail -1",
    "network": "ip addr show | head -20",
    "uptime": "uptime"
}


def run_ssh(host: str, user: str, command: str) -> str:
    target = f"{user}@{host}"
    result = subprocess.run(
        ["ssh", target, command],
        capture_output=True,
        text=True,
        timeout=10,
        check=False,
    )
    if result.returncode != 0:
        return result.stderr.strip()
    return result.stdout.strip()


def main() -> None:
    parser = argparse.ArgumentParser(description="SSH diagnostics for a simulated 5G small cell node")
    parser.add_argument("--host", default="localhost")
    parser.add_argument("--user", default="user")
    args = parser.parse_args()

    print("Checking remote small cell node via SSH...")
    for label, command in COMMANDS.items():
        print(f"\n[{label.upper()}]")
        print(run_ssh(args.host, args.user, command))


if __name__ == "__main__":
    main()
