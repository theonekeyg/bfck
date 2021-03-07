#ifndef _BFCK_H
#define _BFCK_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdarg.h>
#include <errno.h>

#define MEMORY_SIZE 30000
#define MAX_CALLSTACK 30

typedef enum {
  INST_NOP = 0,
  INST_DP_INC,
  INST_DP_DEC,
  INST_BYTE_INC,
  INST_BYTE_DEC,
  INST_PUTCHAR,
  INST_GETCHAR,
  INST_LOOP_START,
  INST_LOOP_END,
  INST_EXIT
} BFCK_INST;

struct bfck_tokenizer {
  char *filename;
  char *buf; // program buffer
  char *ip;  // instruction pointer
  size_t nline;
  size_t nrow;
};

struct bfck_inter {
  struct bfck_tokenizer *tokenizer;
  uint8_t mem[MEMORY_SIZE]; // 3kb memory space
  int mem_idx;              // current index to memory
  int depth;                // current depth of the call stack ([])
  char *callstack[MAX_CALLSTACK];
};

_Noreturn void bfck_panic(struct bfck_tokenizer *tokenizer, const char *fmt, ...);
int bfck_init(struct bfck_inter *inter);
void bfck_set_buffer(struct bfck_tokenizer *tokenizer, char *buf);
BFCK_INST bfck_next(struct bfck_tokenizer *tokenizer);
void bfck_execute(struct bfck_inter *inter);
void bfck_free(struct bfck_inter *inter);

#endif // _BFCK_H
