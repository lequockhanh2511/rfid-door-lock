#!/usr/bin/env python3
"""Compile public contracts independently and together; no hardware execution."""
from pathlib import Path
import shutil
import os
import subprocess
import tempfile
from compiler_environment import prepare_compiler

root = Path(__file__).resolve().parents[1]
cc = os.environ.get('CC') or shutil.which('gcc')
if not cc:
    raise SystemExit('GCC missing: no compile validation performed.')
cc = prepare_compiler(cc)
headers = sorted((root / 'app/inc').glob('*.h')) + sorted((root / 'hal/inc').glob('*.h'))
flags = ['-std=c11', '-Wall', '-Wextra', '-Werror', '-pedantic',
         '-I', str(root / 'app/inc'), '-I', str(root / 'hal/inc'), '-fsyntax-only']
with tempfile.TemporaryDirectory(prefix='rfid-m1-') as temp:
    source = Path(temp) / 'contract_check.c'
    for header in headers:
        source.write_text('#include "' + header.name + '"\nextern int contract_probe;\n', encoding='utf-8')
        subprocess.run([cc, *flags, str(source)], check=True)
        print('PASS standalone header:', header.relative_to(root))
    source.write_text(''.join('#include "' + h.name + '"\n' for h in headers) + '''
_Static_assert(APP_CARD_CAPACITY <= 16u, "occupied_mask capacity");
_Static_assert(APP_UI_QUEUE_DEPTH == 1u, "overwrite mailbox must have one slot");
_Static_assert(sizeof(APP_DEMO_USER_PIN) == APP_PIN_DIGITS + 1u, "user PIN size");
_Static_assert(sizeof(APP_DEMO_ADMIN_PIN) == APP_PIN_DIGITS + 1u, "admin PIN size");
_Static_assert(APP_CONTROL_PRIORITY > APP_RFID_PRIORITY, "control priority");
_Static_assert(APP_RFID_PRIORITY > APP_UI_PRIORITY, "UI priority");
_Static_assert(APP_UNLOCK_MS < 0x80000000u, "wrap-safe time horizon");
/* Compile call sites to check that interfaces compose; intentionally no link. */
void contract_usage(void) {
    app_command_t command = {0};
    app_event_t event = {0};
    app_ui_snapshot_t view = {0};
    command.request_id = 1u;
    command.kind = APP_CMD_VERIFY_PIN;
    command.data.credential.role = APP_PIN_ADMIN;
    (void)app_ipc_send_command(&command, 0u);
    (void)app_ipc_receive_event(&event, 0u);
    (void)app_ipc_publish_ui(&view);
    lock_output_set(false);
}
''', encoding='utf-8')
    subprocess.run([cc, *flags, str(source)], check=True)
    print('PASS combined contracts and configuration constraints')
print('Header compile checks only; this command does not link firmware or test hardware.')
