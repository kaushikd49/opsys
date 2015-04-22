#ifndef __SBUNIX_H
#define __SBUNIX_H

#include <sys/defs.h>

void printf(const char *fmt, ...);
int write_char_to_vid_mem(char c, uint64_t pos);
#endif
