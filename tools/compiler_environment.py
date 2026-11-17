"""Process-local GCC setup, including portable Windows paths with Unicode parents."""
import ctypes
import os
from pathlib import Path
import subprocess


def prepare_compiler(executable):
    compiler = os.path.abspath(executable)
    if os.name == 'nt':
        buffer = ctypes.create_unicode_buffer(32768)
        if ctypes.windll.kernel32.GetShortPathNameW(compiler, buffer, len(buffer)):
            compiler = str(Path(buffer.value).parent / Path(compiler).name)
    binary = os.path.dirname(compiler)
    os.environ['PATH'] = binary + os.pathsep + os.environ.get('PATH', '')
    if os.name == 'nt':
        # GCC may rediscover its long executable path internally. Keep its
        # subordinate tools and libraries on the same compatible short prefix.
        prefix = Path(binary).parent
        machine = subprocess.check_output([compiler, '-dumpmachine'], text=True).strip()
        version = subprocess.check_output([compiler, '-dumpfullversion'], text=True).strip()
        gcc_root = prefix / 'lib/gcc'
        if (gcc_root / machine / version).is_dir():
            os.environ['GCC_EXEC_PREFIX'] = gcc_root.as_posix() + '/'
            os.environ['COMPILER_PATH'] = os.pathsep.join([
                str(prefix / 'libexec/gcc' / machine / version), binary])
            os.environ['LIBRARY_PATH'] = os.pathsep.join([
                str(gcc_root / machine / version), str(prefix / 'lib')])
    return compiler
