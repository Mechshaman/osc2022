SECTIONS
{
  . = 0x80000;
  .text :
  {
    _begin = .;
    KEEP(*(.text.boot)) /* keeps the symbols in the section even if symbols are not referenced */
    *(.text)
  }

  . = ALIGN(4096);
  .data :
  {
    *(.data)
  }

  . = ALIGN(4096);
  .bss (NOLOAD) :
  {
    __bss_start = .;

    *(.bss)

    __bss_end = .;
  }
  _end = .;
}

__bss_size = (__bss_end - __bss_start) >> 3;
__boot_loader = 0x60000;