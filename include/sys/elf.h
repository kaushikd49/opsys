#define __elf_H
#ifndef __elf_H
Elf64_Shdr *get_elf_section_header(Elf64_Ehdr *header);
Elf64_Shdr *get_elf_section_index(Elf64_Shdr *header, int current);
char *get_elf_string_loc(uint64_t base_file, Elf64_Shdr *header, int offset);
Elf64_Shdr *match_section_elf(Elf64_Ehdr *current_elf, char *section);

#endif
