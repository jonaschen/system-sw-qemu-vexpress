MEMORY
{
	KERNEL_IMAGE 	(RWX) : ORIGIN = 0x60000000, LENGTH = 0x10000000
	TTBDIR 	(RWX) : ORIGIN = 0x6F000000, LENGTH = 0x1000000
	RAM2 	(RX) : ORIGIN = 0x70000000, LENGTH = 0x10000000

	KERNEL_VADDR (RWX) : ORIGIN = 0x80000000, LENGTH = 0x80000000
}
PROVIDE(KERNEL_OFFSET = 0x80000000);

PROVIDE(normal_memory_base = 0x60000000);
PROVIDE(normal_memory_size = 0x20000000);
PROVIDE(kernel_image_base = ORIGIN(KERNEL_IMAGE));
PROVIDE(kernel_image_size = LENGTH(KERNEL_IMAGE));
PROVIDE(peripheral_device_base = 0x10000000);
PROVIDE(peripheral_device_size = 0x10000000);

PROVIDE(PHYS_OFFSET = ORIGIN(RAM2));
PROVIDE(PHYS_LENGTH = LENGTH(RAM2));
PROVIDE(__vector_base = KERNEL_OFFSET);

/* 1MB Sections: 4096 X 4-byte entries for 4G virtual space */
PROVIDE(ttb_address = ORIGIN(TTBDIR));
PROVIDE(ttb1_address = ORIGIN(TTBDIR) + 0x800000);
PROVIDE(TTB_DIR_SIZE = 0x4000);

SECTIONS
{
	.text :
	{
		_stext = .;
		KEEP(*(.vectors))
		*(.text)
		*(.text.*)
		*(.rodata)
		*(.rodata.*)

		. = ALIGN(4);
		_etext = .;
	} > KERNEL_VADDR AT > KERNEL_IMAGE

	.data :
	{
		. = ALIGN(4);
		_sdata = .;
		*(.data)
		*(.data*)

		. = ALIGN(4);
		_edata = .;
	} > KERNEL_VADDR AT > KERNEL_IMAGE

	.bss :
	{
		. = ALIGN(4);
		_sbss = .;
		*(.bss)

		. = ALIGN(4);
		_ebss = .;
	} > KERNEL_VADDR AT > KERNEL_IMAGE

	_estack = KERNEL_OFFSET + 0x1000000; /* Currently 16MB Boundary */
}

/* Note. */
/* KEEP: LD keeps the symbols in the section even if symbols are not referenced. */

