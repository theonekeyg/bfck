#include "bfck.h"

__attribute__((format(printf, 2, 3)))
_Noreturn void bfck_panic(struct bfck_tokenizer *tokenizer, const char *fmt, ...) {
  va_list ap;
  char err_msg[500];
  va_start(ap, fmt);
  vsnprintf(err_msg, 500, fmt, ap);
  fprintf(stderr, "Error: `%s` at [%ld;%ld]\n", err_msg, tokenizer->nline, tokenizer->nrow);
  exit(1);
}
