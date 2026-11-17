#!/usr/bin/env python3
"""Generate the single supported eight-virtual-card setup configuration."""
import argparse
from pathlib import Path

def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--output', type=Path, default=Path(__file__).resolve().parents[1] / 'build/generated/provision_data.h')
    mode = parser.add_mutually_exclusive_group(required=True)
    mode.add_argument('--confirm-initial-setup', action='store_true')
    mode.add_argument('--test-only', action='store_true')
    args = parser.parse_args()
    lines = [
        '/* Slots 01/02 empty; eight synthetic UIDs in slots 03..10. */',
        '#ifndef PROVISION_DATA_H',
        '#define PROVISION_DATA_H',
        '#include "initial_config.h"',
        '#define PROVISION_TEST_ONLY ' + str(int(args.test_only)),
        '#define PROVISION_OCCUPIED_SLOTS 8u',
        '#define provision_config app_initial_config',
        '#endif',
        '',
    ]
    args.output.parent.mkdir(parents=True, exist_ok=True)
    args.output.write_text('\n'.join(lines), encoding='utf-8')
    print('Generated eight-virtual-card setup; TEST_ONLY:', args.test_only)

if __name__ == '__main__':
    main()
