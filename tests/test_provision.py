"""Validate the only supported setup mode and generated configuration contract."""
import subprocess
import sys
import tempfile
from pathlib import Path

root = Path(__file__).resolve().parents[1]
with tempfile.TemporaryDirectory() as directory:
    output = Path(directory) / 'provision_data.h'
    base = [sys.executable, '-X', 'utf8', str(root / 'tools/generate_provision.py'), '--output', str(output)]
    for arguments, expected in [
        ([], False),
        (['--uid1', '11223344', '--uid2', '22334455', '--test-only'], False),
        (['--test-only', '--confirm-initial-setup'], False),
        (['--test-only'], True),
        (['--confirm-initial-setup'], True),
    ]:
        result = subprocess.run(base + arguments, capture_output=True, text=True, encoding='utf-8')
        assert (result.returncode == 0) == expected, result.stderr
        if expected:
            header = output.read_text(encoding='utf-8')
            assert '#define PROVISION_OCCUPIED_SLOTS 8u' in header
            assert '#include "initial_config.h"' in header
            assert '#define provision_config app_initial_config' in header
            assert '#define PROVISION_TEST_ONLY ' + str(int('--test-only' in arguments)) in header
print('PASS provision generator: single eight-virtual-card policy, no physical UID input, TEST_ONLY and explicit setup modes')
