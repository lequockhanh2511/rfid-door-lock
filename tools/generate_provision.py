#!/usr/bin/env python3
"""Generate deliberate initial setup data. Does not access hardware."""
import argparse,re
from pathlib import Path

def uid(text):
    text=text.replace(':','').replace('-','').replace(' ','')
    if len(text) not in (8,14,20) or not re.fullmatch('[0-9A-Fa-f]+',text):
        raise argparse.ArgumentTypeError('UID must have 4, 7 or 10 bytes of hex')
    return bytes.fromhex(text)

def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--output',type=Path,default=Path(__file__).resolve().parents[1]/'build/generated/provision_data.h')
    parser.add_argument('--uid1',type=uid,required=True,help='UID read from physical card 1')
    parser.add_argument('--uid2',type=uid,required=True,help='UID read from physical card 2')
    mode=parser.add_mutually_exclusive_group(required=True)
    mode.add_argument('--confirm-initial-setup',action='store_true',help='Authorize generating a one-time EEPROM setup image')
    mode.add_argument('--test-only',action='store_true',help='Generate a compile-check image that cannot write EEPROM')
    a=parser.parse_args()
    cards=[a.uid1,a.uid2]+[bytes((0xfa,0xce,0,i)) for i in range(1,9)]
    if len(set(cards))!=10:parser.error('Duplicate UID or collision with virtual cards FA:CE:00:01..08')
    lines=['/* Generated locally; slots 01/02 supplied by operator, 03..10 virtual. */',
           '#ifndef PROVISION_DATA_H','#define PROVISION_DATA_H','#include "config_store.h"',
           '#define PROVISION_TEST_ONLY '+str(int(a.test_only)),
           'static const app_config_t provision_config = { .slots = {']
    for card in cards:
        lines.append('    {.used=true,.uid={.length='+str(len(card))+',.bytes={'+','.join(f'0x{x:02x}' for x in card)+'}}},')
    lines+=['}, .user_pin={.digits=APP_DEMO_USER_PIN}, .admin_pin={.digits=APP_DEMO_ADMIN_PIN} };','#endif','']
    out=a.output
    out.parent.mkdir(parents=True,exist_ok=True);out.write_text('\n'.join(lines))
    print('Generated',out,'TEST_ONLY - EEPROM writes disabled' if a.test_only else 'INITIAL SETUP - blank/invalid configuration only')
if __name__=='__main__':main()
