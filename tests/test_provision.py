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
