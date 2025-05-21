# Configuration file for the 'lit' test runner.

import lit.formats
from pathlib import Path

config.name = "GBLIB"
config.test_format = lit.formats.ShTest(True)
config.suffixes = [".cpp"]
config.excludes = []

config.test_build = Path(__file__).parent.parent / "build" / "tests"
