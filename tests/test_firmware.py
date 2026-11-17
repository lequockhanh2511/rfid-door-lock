"""Validate generated ELF/HEX/BIN consistency and reserved configuration boundaries."""
import json
from pathlib import Path
import struct

root = Path(__file__).resolve().parents[1]
targets = {'app': 'rfid_application', 'uid': 'rfid_uid_reader',
           'provision': 'rfid_initial_setup'}
for target, name in targets.items():
    folder = root / 'build' / target
    elf = (folder / (name + '.elf')).read_bytes()
    assert elf[:7] == b'\x7fELF\x01\x01\x01', target
    assert struct.unpack_from('<H', elf, 18)[0] == 40, 'ELF must target ARM'
    binary = (folder / (name + '.bin')).read_bytes()
    stack, reset = struct.unpack_from('<II', binary)
    assert stack == 0x20005000, (target, hex(stack))
    assert reset & 1 and 0x08000000 <= reset < 0x0800F800, (target, hex(reset))
    base = 0
    image = {}
    ended = False
    for line in (folder / (name + '.hex')).read_text().splitlines():
        assert not ended and line.startswith(':'), target
        record = bytes.fromhex(line[1:])
        assert len(record) == record[0] + 5 and sum(record) % 256 == 0, target
        count, offset, kind = record[0], int.from_bytes(record[1:3], 'big'), record[3]
        payload = record[4:4 + count]
        if kind == 0:
            for index, value in enumerate(payload):
                address = base + offset + index
                assert 0x08000000 <= address < 0x0800F800, 'HEX overlaps configuration pages'
                assert address not in image, 'Overlapping HEX records'
                image[address] = value
        elif kind == 4:
            assert count == 2
            base = int.from_bytes(payload, 'big') << 16
        elif kind == 1:
            assert count == 0
            ended = True
        else:
            assert kind == 5 and count == 4, 'Unexpected HEX record type'
    assert ended and min(image) == 0x08000000
    assert max(image) - 0x08000000 + 1 == len(binary)
    assert all(binary[address - 0x08000000] == value for address, value in image.items())
    memory = json.loads((folder / 'memory.json').read_text())
    assert memory['flash_bytes'] == len(binary) <= 62 * 1024
    assert memory['ram_reserved_bytes'] <= 20 * 1024
    print('PASS firmware', target, '- ARM ELF, vector, HEX checksum, BIN consistency, Flash/RAM limits')
