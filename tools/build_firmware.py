#!/usr/bin/env python3
"""Build diagnostic, application or deliberate provision firmware. No downloads."""
from pathlib import Path
import os, shutil, subprocess, argparse
root=Path(__file__).resolve().parents[1]
os.chdir(root)
cc=os.environ.get('ARM_CC') or shutil.which('arm-none-eabi-gcc')
if not cc: raise SystemExit('Missing arm-none-eabi-gcc. Set ARM_CC to its absolute path.')
cc=str(Path(cc).resolve()); prefix=cc[:-3] if cc.endswith('gcc') else ''
if not prefix: raise SystemExit('ARM_CC must point to arm-none-eabi-gcc')
parser=argparse.ArgumentParser(description=__doc__)
parser.add_argument('--target',choices=['m2','app','provision'],default='app')
args=parser.parse_args()
if args.target=='provision' and not (root/'build/generated/provision_data.h').exists():
 raise SystemExit('Run tools/generate_provision.py with your two real card UIDs first.')
out=root/'build'/args.target;out.mkdir(parents=True,exist_ok=True)
incs=['app/inc','hal/inc','bsp/inc','middlewares/hal/Inc','middlewares/device/Include',
      'middlewares/cmsis/Include','middlewares/freertos/include','middlewares/freertos/portable/GCC/ARM_CM4F','build/generated']
flags=['-mcpu=cortex-m4','-mthumb','-mfpu=fpv4-sp-d16','-mfloat-abi=hard',
       '-Os','-g3','-fstack-usage','-ffunction-sections','-fdata-sections','-DSTM32F411xE','-DUSE_HAL_DRIVER']
for p in incs: flags+=['-I',p]
sources=list(Path('hal/src').glob('*.c'))+list(Path('bsp/src').glob('*.c'))
sources += [p for p in Path('middlewares/hal/Src').glob('*.c') if p.name != 'stm32f4xx_hal_flash_ex.c']
sources += [Path(p) for p in ['middlewares/device/system_stm32f4xx.c','middlewares/device/startup_stm32f411xe.s',
  'middlewares/freertos/tasks.c','middlewares/freertos/queue.c','middlewares/freertos/list.c',
  'middlewares/freertos/portable/MemMang/heap_4.c','middlewares/freertos/portable/GCC/ARM_CM4F/port.c',
  'examples/m2_diagnostics/main.c']]
if args.target=='app':
 sources=sources[:-1]+list(Path('app/src').glob('*.c'))
elif args.target=='provision':
 sources=sources[:-1]+[Path('app/src/config_store.c'),Path('examples/provision/main.c')]
name={'m2':'rfid_m2_diagnostics','app':'rfid_application','provision':'rfid_initial_setup'}[args.target]
objects=[]
for src in sources:
 obj=out/(str(src).replace('/','_')+'.o');objects.append(str(obj))
 extra=['-std=c11','-Wall','-Wextra'] if src.suffix=='.c' else []
 if not str(src).startswith('middlewares/') and src.suffix=='.c':extra+=['-Werror','-Wformat=2']
 subprocess.run([cc,*flags,*extra,'-c',str(src),'-o',str(obj)],check=True)
elf=out/(name+'.elf')
subprocess.run([cc,*flags,*objects,'-T','bsp/stm32f411re.ld','-nostartfiles','--specs=nano.specs','--specs=nosys.specs',
 '-Wl,--gc-sections','-Wl,-Map='+str(out/(name+'.map')),'-Wl,--print-memory-usage',
 '-Wl,--no-warn-rwx-segments','-o',str(elf)],check=True)
for ext,fmt in [('hex','ihex'),('bin','binary')]:
 subprocess.run([prefix+'objcopy','-O',fmt,str(elf),str(elf.with_suffix('.'+ext))],check=True)
subprocess.run([prefix+'size',str(elf)],check=True)
print('BUILD PASS: ARM target='+args.target+'. Hardware execution: NOT TESTED.')
