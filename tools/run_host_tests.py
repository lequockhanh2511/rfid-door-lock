#!/usr/bin/env python3
"""Compile and execute host models. No hardware is accessed."""
import argparse
import os
from pathlib import Path
import shutil
import subprocess
import sys
from compiler_environment import prepare_compiler

root = Path(__file__).resolve().parents[1]
os.chdir(root)
parser = argparse.ArgumentParser(description=__doc__)
parser.add_argument('--cc', default=os.environ.get('CC') or shutil.which('gcc'))
parser.add_argument('--sanitize', action='store_true')
args = parser.parse_args()
if not args.cc:
    parser.error('Host GCC missing. Set CC or --cc. C tests have not run.')
args.cc = prepare_compiler(args.cc)
flags = ['-std=c11', '-Wall', '-Wextra', '-Werror', '-pedantic',
         '-Iapp/inc', '-Ihal/inc', '-Itests', '-Ibuild/host/generated']
if args.sanitize:
    flags.append('-fsanitize=undefined')
out = root / 'build/host'
out.mkdir(parents=True, exist_ok=True)
subprocess.run([sys.executable, '-X', 'utf8', 'tools/generate_provision.py',
                '--test-only',
                '--output', str(out / 'generated/provision_data.h')], check=True)
cases = {
    'rc522': ['hal/src/rc522.c'],
    'keypad': ['hal/src/keypad.c'],
    'display': ['hal/src/ssd1306.c', 'hal/src/lock_output.c'],
    'config': ['app/src/config_store.c', 'app/src/rfid_service.c', 'tests/fake_flash.c'],
    'ipc': ['app/src/app_ipc.c'],
}
for name in ('control', 'outline', 'enrollment'):
    cases[name] = cases['config'] + ['app/src/access_control.c']
for name, sources in cases.items():
    target = out / ('test_' + name + ('.exe' if os.name == 'nt' else ''))
    includes = ['-Itests/mocks'] if name == 'ipc' else []
    subprocess.run([args.cc, *flags, *includes, 'tests/test_' + name + '.c',
                    *sources, '-o', str(target.relative_to(root))], check=True)
    subprocess.run([str(target)], check=True)
subprocess.run([sys.executable, '-X', 'utf8', 'tests/test_provision.py'], check=True)
print('PASS: 8 C host suites and provision generator. Hardware remains untested.')
print('Undefined-behavior sanitizer enabled:', args.sanitize)
