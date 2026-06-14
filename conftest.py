"""
conftest.py — pytest configuration for 5G Small Cell Monitoring Agent.

Adds the project root to sys.path so that test files can import from
`scripts/` and `tests/` without installing the package.

This file is automatically discovered by pytest when it is placed at the
project root.
"""
import sys
from pathlib import Path

# Insert project root at the front of sys.path
sys.path.insert(0, str(Path(__file__).parent))
