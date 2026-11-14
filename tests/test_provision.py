"""Generator validation. Synthetic UID strings are TEST_ONLY fixture values."""
import subprocess,sys,tempfile
from pathlib import Path
root=Path(__file__).resolve().parents[1]
script=root/'tools/generate_provision.py'
temp=tempfile.TemporaryDirectory()
out=Path(temp.name)/'provision_data.h'
base=[sys.executable,str(script),'--output',str(out)]
def run(args,ok):
 r=subprocess.run(base+args,capture_output=True,text=True)
 assert (r.returncode==0)==ok,(args,r.stdout,r.stderr)
run(['--uid1','01','--uid2','11223344','--test-only'],False)
run(['--uid1','11223344','--uid2','11223344','--test-only'],False)
run(['--uid1','FACE0001','--uid2','11223344','--test-only'],False)
run(['--uid1','11223344','--uid2','22334455'],False)
run(['--uid1','F0112233','--uid2','F0112244','--test-only'],True)
h=out.read_text()
assert '#define PROVISION_TEST_ONLY 1' in h and h.count('.used=true')==10
print('PASS provision generator: UID length, duplicates, virtual collision, explicit mode, 10 slots, TEST_ONLY write guard')
run(['--keypad-enrollment'],False)
run(['--keypad-enrollment','--uid1','11223344','--confirm-initial-setup'],False)
run(['--uid1','11223344','--confirm-initial-setup'],False)
run(['--keypad-enrollment','--confirm-initial-setup'],True)
h=out.read_text()
assert '#define PROVISION_TEST_ONLY 0' in h
assert '#define PROVISION_KEYPAD_ENROLLMENT 1' in h
assert h.count('.used=true')==8 and h.count('.used=false')==2
assert '#define PROVISION_OCCUPIED_SLOTS 8u' in h
print('PASS keypad provisioning generator: explicit mode, empty 01/02, virtual 03..10, incompatible arguments rejected')
# Compile the generated data itself into the host integration test.
import shutil
cc=shutil.which('gcc')
assert cc,'GCC required for generated-config integration test'
target=Path(temp.name)/'test_enrollment'
subprocess.run([cc,'-std=c11','-Wall','-Wextra','-Werror','-pedantic','-fsanitize=undefined',
 '-I'+str(root/'app/inc'),'-I'+str(root/'hal/inc'),'-I'+str(root/'tests'),'-I'+temp.name,
 str(root/'tests/test_enrollment.c'),str(root/'tests/fake_eeprom.c'),
 *[str(root/'app/src'/n) for n in ['config_store.c','rfid_service.c','access_control.c']],
 '-o',str(target)],check=True)
subprocess.run([str(target)],check=True)
