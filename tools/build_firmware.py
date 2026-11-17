#!/usr/bin/env python3
"""Build F103C8 firmware from pinned repository dependencies or a supplied CubeF1."""
import argparse
import json
import os
from pathlib import Path
import shutil
import subprocess
from compiler_environment import prepare_compiler

root = Path(__file__).resolve().parents[1]
os.chdir(root)
parser = argparse.ArgumentParser(description=__doc__)
parser.add_argument('--target', choices=['app', 'provision', 'uid'], default='app')
parser.add_argument('--cc', default=os.environ.get('ARM_CC') or shutil.which('arm-none-eabi-gcc'))
parser.add_argument('--cube-root', default=os.environ.get('STM32CUBE_F1'))
parser.add_argument('--freertos-root', default=os.environ.get('FREERTOS_ROOT'))
args = parser.parse_args()
if not args.cc:
    parser.error('Missing arm-none-eabi-gcc. Set ARM_CC or --cc.')
cc = prepare_compiler(args.cc)
extension = '.exe' if cc.lower().endswith('.exe') else ''
stem = cc[:-4] if extension else cc
if not stem.endswith('arm-none-eabi-gcc'):
    parser.error('Expected arm-none-eabi-gcc or arm-none-eabi-gcc.exe')
prefix = stem[:-3]
if args.cube_root:
    cube = Path(args.cube_root)
    hal = cube / 'Drivers/STM32F1xx_HAL_Driver'
    device = cube / 'Drivers/CMSIS/Device/ST/STM32F1xx'
    cmsis = cube / 'Drivers/CMSIS'
    system = device / 'Source/Templates/system_stm32f1xx.c'
    startup = device / 'Source/Templates/gcc/startup_stm32f103xb.s'
    default_rtos = cube / 'Middlewares/Third_Party/FreeRTOS/Source'
else:
    hal = Path('middlewares/hal')
    device = Path('middlewares/device')
    cmsis = Path('middlewares/cmsis')
    system = device / 'system_stm32f1xx.c'
    startup = device / 'startup_stm32f103xb.s'
    default_rtos = Path('middlewares/freertos')
rtos = Path(args.freertos_root) if args.freertos_root else default_rtos
port = rtos / 'portable/GCC/ARM_CM3'
includes = [Path('app/inc'), Path('hal/inc'), Path('bsp/inc'), hal / 'Inc',
            device / 'Include', cmsis / 'Include', rtos / 'include', port,
            Path('build/generated')]
flags = ['-mcpu=cortex-m3', '-mthumb', '-Os', '-g3', '-fstack-usage',
         '-ffunction-sections', '-fdata-sections', '-DSTM32F103xB', '-DUSE_HAL_DRIVER']
for include in includes:
    flags += ['-I', str(include)]
sources = sorted(Path('hal/src').glob('*.c')) + sorted(Path('bsp/src').glob('*.c'))
modules = ['', '_rcc', '_rcc_ex', '_pwr', '_gpio', '_gpio_ex', '_cortex', '_dma',
           '_flash', '_flash_ex', '_spi', '_i2c', '_tim', '_tim_ex']
vendor = [hal / 'Src' / ('stm32f1xx_hal' + module + '.c') for module in modules]
vendor += [system, startup, rtos / 'tasks.c', rtos / 'queue.c', rtos / 'list.c',
           rtos / 'portable/MemMang/heap_4.c', port / 'port.c']
sources += vendor
if args.target == 'app':
    sources += sorted(Path('app/src').glob('*.c'))
elif args.target == 'provision':
    if not Path('build/generated/provision_data.h').is_file():
        parser.error('Run tools/generate_provision.py first.')
    sources += [Path('app/src/config_store.c'), Path('examples/provision/main.c')]
else:
    sources += [Path('examples/uid_reader/main.c')]
missing = [str(source) for source in sources if not source.is_file()]
if missing:
    parser.error('Missing sources:\n' + '\n'.join(missing))
out = Path('build') / args.target
out.mkdir(parents=True, exist_ok=True)
name = {'app': 'rfid_application', 'provision': 'rfid_initial_setup', 'uid': 'rfid_uid_reader'}[args.target]
elf = out / (name + '.elf')
for suffix in ('.elf', '.hex', '.bin', '.map'):
    elf.with_suffix(suffix).unlink(missing_ok=True)
(out / 'memory.json').unlink(missing_ok=True)
objects = []
for index, source in enumerate(sources):
    obj = out / (str(index) + '_' + source.name + '.o')
    objects.append(str(obj))
    extra = ['-std=c11', '-Wall', '-Wextra'] if source.suffix == '.c' else []
    if source not in vendor and source.suffix == '.c':
        extra += ['-Werror', '-Wformat=2']
    subprocess.run([cc, *flags, *extra, '-c', str(source), '-o', str(obj)], check=True)
subprocess.run([cc, *flags, *objects, '-T', 'bsp/stm32f103c8.ld', '-nostartfiles',
                '--specs=nano.specs', '--specs=nosys.specs', '-Wl,--gc-sections',
                '-Wl,-Map=' + str(elf.with_suffix('.map')), '-Wl,--print-memory-usage',
                '-o', str(elf)], check=True)
size_text = subprocess.check_output([prefix + 'size' + extension, str(elf)], text=True)
print(size_text, end='')
text_bytes, data_bytes, bss_bytes = map(int, size_text.splitlines()[-1].split()[:3])
sections = subprocess.check_output([prefix + 'objdump' + extension, '-h', str(elf)], text=True).splitlines()
flash_end, ram_end = 0x08000000, 0x20000000
for index, line in enumerate(sections[:-1]):
    fields = line.split()
    if len(fields) < 7 or not fields[0].isdigit():
        continue
    size, address, load_address = (int(value, 16) for value in fields[2:5])
    attributes = sections[index + 1]
    if 'LOAD' in attributes and 0x08000000 <= load_address < 0x08010000:
        flash_end = max(flash_end, load_address + size)
    if 'ALLOC' in attributes and 0x20000000 <= address < 0x20005000:
        ram_end = max(ram_end, address + size)
memory = {'flash_bytes': flash_end - 0x08000000, 'flash_limit': 62 * 1024,
          'ram_reserved_bytes': ram_end - 0x20000000, 'ram_limit': 20 * 1024,
          'hardware_tested': False}
if memory['flash_bytes'] > memory['flash_limit'] or memory['ram_reserved_bytes'] > memory['ram_limit']:
    raise SystemExit('Memory limits exceeded. HEX/BIN were not generated.')
(out / 'memory.json').write_text(json.dumps(memory, indent=2) + '\n', encoding='utf-8')
for suffix, format_name in [('.hex', 'ihex'), ('.bin', 'binary')]:
    subprocess.run([prefix + 'objcopy' + extension, '-O', format_name,
                    str(elf), str(elf.with_suffix(suffix))], check=True)
print('BUILD PASS:', args.target, '- hardware execution NOT TESTED.')
