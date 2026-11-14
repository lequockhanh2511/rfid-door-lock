#!/usr/bin/env python3
"""Repeatable driver tests against models; never accesses actual peripherals."""
from pathlib import Path
import os,subprocess,shutil
root=Path(__file__).resolve().parents[1];os.chdir(root)
cc=shutil.which('gcc')
if not cc: raise SystemExit('GCC required')
Path('build/host').mkdir(parents=True,exist_ok=True)
cases={'storage':['eeprom24'],'rc522':['rc522'],'keypad':['keypad'],'display':['ssd1306','lock_output']}
for name,drivers in cases.items():
 target='build/host/test_'+name
 cmd=[cc,'-std=c11','-Wall','-Wextra','-Werror','-pedantic','-fsanitize=undefined',
      '-Iapp/inc','-Ihal/inc','tests/test_'+name+'.c']
 cmd+=['hal/src/'+s+'.c' for s in drivers]
 subprocess.run(cmd+['-o',target],check=True);subprocess.run([str(root/target)],check=True)
for name, sources in {'config':['config_store','rfid_service']}.items():
 target='build/host/test_'+name
 subprocess.run([cc,'-std=c11','-Wall','-Wextra','-Werror','-pedantic','-fsanitize=undefined',
  '-Iapp/inc','-Ihal/inc','-Itests','tests/test_'+name+'.c','tests/fake_eeprom.c',
  *['app/src/'+s+'.c' for s in sources],'-o',target],check=True)
 subprocess.run([str(root/target)],check=True)
print('HOST MOCK TESTS PASS. Electrical/RF/RTOS-on-board tests remain pending.')
