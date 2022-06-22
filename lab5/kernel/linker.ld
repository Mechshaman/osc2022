SECTIONS
{
  . = 0x80000;
  .text :
  {
    KEEP(*(.text.boot)) /* keeps the symbols in the section even if symbols are not referenced */
    *(.text)
  }
  .data :
  {
    *(.data)
  }
  . = ALIGN(16); 
  .bss :
  {
    __bss_start = .;

    *(.bss)

    __bss_end = .;
  }
  . = ALIGN(16); 
  _heap_start = .;
  _end = .;
}

__bss_size = (__bss_end - __bss_start) >> 3;