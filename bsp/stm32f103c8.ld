/* STM32F103C8T6: 62 KiB firmware + 2 KiB reserved configuration; 20 KiB RAM. */
ENTRY(Reset_Handler)
MEMORY { FLASH (rx) : ORIGIN = 0x08000000, LENGTH = 62K
         RAM (xrw) : ORIGIN = 0x20000000, LENGTH = 20K }
_estack = ORIGIN(RAM) + LENGTH(RAM);
_Min_Stack_Size = 0x400;
SECTIONS {
 .isr_vector : { . = ALIGN(4); KEEP(*(.isr_vector)) . = ALIGN(4); } > FLASH
 .text : { . = ALIGN(4); *(.text .text.*) *(.rodata .rodata.*) *(.glue_7*) KEEP(*(.init)) KEEP(*(.fini)) . = ALIGN(4); _etext = .; } > FLASH
 .ARM.extab : { *(.ARM.extab* .gnu.linkonce.armextab.*) } > FLASH
 .ARM.exidx : { __exidx_start = .; *(.ARM.exidx* .gnu.linkonce.armexidx.*) __exidx_end = .; } > FLASH
 .preinit_array : { PROVIDE_HIDDEN(__preinit_array_start = .); KEEP(*(.preinit_array*)) PROVIDE_HIDDEN(__preinit_array_end = .); } > FLASH
 .init_array : { PROVIDE_HIDDEN(__init_array_start = .); KEEP(*(SORT(.init_array.*))) KEEP(*(.init_array*)) PROVIDE_HIDDEN(__init_array_end = .); } > FLASH
 .fini_array : { PROVIDE_HIDDEN(__fini_array_start = .); KEEP(*(SORT(.fini_array.*))) KEEP(*(.fini_array*)) PROVIDE_HIDDEN(__fini_array_end = .); } > FLASH
 .data : { . = ALIGN(4); _sdata = .; *(.data .data.*) . = ALIGN(4); _edata = .; } > RAM AT> FLASH
 _sidata = LOADADDR(.data);
 .bss (NOLOAD) : { . = ALIGN(4); _sbss = .; __bss_start__ = .; *(.bss .bss.*) *(COMMON) . = ALIGN(4); _ebss = .; __bss_end__ = .; } > RAM
 ._user_heap_stack (NOLOAD) : { . = ALIGN(8); PROVIDE(end = .); PROVIDE(_end = .); . = . + _Min_Stack_Size; . = ALIGN(8); } > RAM
 /DISCARD/ : { *(.note.GNU-stack) }
}
ASSERT(LOADADDR(.data) + SIZEOF(.data) <= 0x0800F800, "Firmware overlaps Flash configuration pages")
