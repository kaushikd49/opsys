#ifndef _DEFS_H
#define _DEFS_H

#define NULL ((void*)0)

typedef unsigned long uint64_t;
typedef          long   int64_t;
typedef unsigned int   uint32_t;
typedef          int    int32_t;
typedef unsigned short uint16_t;
typedef          short  int16_t;

typedef unsigned long Elf64_Addr;
typedef unsigned long Elf64_Off;
typedef unsigned short Elf64_Half;
typedef unsigned int Elf64_Word;
typedef int Elf64_Sword;
typedef unsigned long Elf64_Xword;
typedef long Elf64_Sxord;
typedef struct {
	unsigned char e_ident[16];
	Elf64_Half e_type;
	Elf64_Half e_machine;
	Elf64_Word e_version;
	Elf64_Addr e_entry;
	Elf64_Off e_phoff;
	Elf64_Off e_shoff;
	Elf64_Word e_flags;
	Elf64_Half e_ehsize;
	Elf64_Half e_phentsize;
	Elf64_Half e_phnum;
	Elf64_Half e_shentsize;
	Elf64_Half e_shnum;
	Elf64_Half e_shstrndx;
} Elf64_Ehdr;
typedef struct {
	Elf64_Word sh_name;
	Elf64_Word sh_type;
	Elf64_Xword sh_flags;
	Elf64_Addr sh_addr;
	Elf64_Off sh_offset;
	Elf64_Xword sh_size;
	Elf64_Word sh_link;
	Elf64_Word sh_info;
	Elf64_Xword sh_addralign;
	Elf64_Xword sh_entsize;
}Elf64_Shdr;
struct idtD {
	uint16_t offset_1;
	uint16_t segment_selector;
	char zero;
	char type;
	uint16_t offset2;
	uint32_t offset3;
	uint32_t zero2;
}__attribute__((packed));
struct idtD idt_tab[255];
struct lidtr_t { //initializing ldtr register
	uint16_t size;
	uint64_t base;
}__attribute__((packed));
#define align(size, alignment) (size + (alignment - 1)) & ~(alignment - 1)
#define TARFS_ALIGNMENT 512

typedef int32_t pid_t;
typedef int64_t ssize_t;
typedef uint64_t size_t;
typedef uint64_t off_t; //todo: was clashing with the system library  include/sys/tarfs.h
typedef int64_t time_t;

#endif
