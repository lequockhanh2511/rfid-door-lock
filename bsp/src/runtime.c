/* Startup comes from ST, not hosted crt0. No C++ constructors/destructors here.
 * newlib __libc_init_array invokes _init; no board initialization belongs here.
 */
void _init(void) {}
void _fini(void) {}
