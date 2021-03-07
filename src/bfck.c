#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include "bfck.h"

_Noreturn static void usage() {
  printf("Usage: bfck [options] [file | -c cmd]\n\n");
  printf(" Just another brainf*ck interpreter\n");
  exit(0);
}

static char *read_file(const char *fname) {
  int fd = open(fname, O_RDONLY);
  if (fd == -1) {
    fprintf(stderr, "Error: %s\n", strerror(errno));
    return NULL;
  }
  struct stat st;
  if (!fstat(fd, &st)) {
    /*
    if ((st.st_mode & S_IFMT) != S_IFREG) {
      fprintf(stderr, "Error: `%s` is not a regular file\n", fname);
      return NULL;
    }
    */
    if (!(st.st_mode & S_IFREG)) {
      fprintf(stderr, "Error: `%s` is not a regular file\n", fname);
      return NULL;
    }
    size_t buf_size = st.st_size * sizeof(char);
    char *buf = malloc(buf_size+1);
    read(fd, buf, buf_size);
    buf[buf_size] = '\0';
    return buf;
  } else {
    fprintf(stderr, "Error: %s\n", strerror(errno));
    return NULL;
  }
}

int bfck_init(struct bfck_inter *inter) {
  memset(inter->mem, 0, MEMORY_SIZE);
  inter->mem_idx = 0;
  inter->depth = 0;
  if (!(inter->tokenizer = malloc(sizeof(*inter->tokenizer)))) {
    return 1;
  }
  inter->tokenizer->buf = 0;
  inter->tokenizer->ip = 0;
  return 0;
}

void bfck_set_buffer(struct bfck_tokenizer *tokenizer, char *buf) {
  tokenizer->nrow = 0;
  tokenizer->nline = 0;
  tokenizer->buf = buf;
  tokenizer->ip = tokenizer->buf;
}

BFCK_INST bfck_next(struct bfck_tokenizer *tokenizer) {
  int inst = INST_NOP;
  switch (*tokenizer->ip) {
    case '+':
      inst = INST_BYTE_INC;
      break;
    case '-':
      inst = INST_BYTE_DEC;
      break;
    case '>':
      inst = INST_DP_INC;
      break;
    case '<':
      inst = INST_DP_DEC;
      break;
    case '.':
      inst = INST_PUTCHAR;
      break;
    case ',':
      inst = INST_GETCHAR;
      break;
    case '[':
      inst = INST_LOOP_START;
      break;
    case ']':
      inst = INST_LOOP_END;
      break;
    case '\0':
      return INST_EXIT;
  }
  if (*tokenizer->ip == '\n') {
    tokenizer->nrow = 0;
    ++tokenizer->nline;
  } else {
    ++tokenizer->nrow;
  }
  ++tokenizer->ip;
  return inst;
}

void bfck_execute(struct bfck_inter *inter) {
  int inst;
  while ((inst = bfck_next(inter->tokenizer)) != INST_EXIT) {
    switch (inst) {
      case INST_DP_INC:
        ++inter->mem_idx;
        break;
      case INST_DP_DEC:
        --inter->mem_idx;
        break;
      case INST_BYTE_INC:
        inter->mem[inter->mem_idx] += 1;
        break;
      case INST_BYTE_DEC:
        inter->mem[inter->mem_idx] -= 1;
        break;
      case INST_PUTCHAR:
        putchar(inter->mem[inter->mem_idx]);
        break;
      case INST_GETCHAR:
        inter->mem[inter->mem_idx] = getchar();
        break;
      case INST_LOOP_START:
        if (inter->depth >= MAX_CALLSTACK) {
          bfck_panic(inter->tokenizer, "Exceeded maximum size of the callstack");
          return;
        }
        inter->callstack[inter->depth++] = inter->tokenizer->ip;
        break;
      case INST_LOOP_END:
        if (inter->depth < 1) {
          bfck_panic(inter->tokenizer, "Loop-closing bracket outside of the loop");
        }
        if (inter->mem[inter->mem_idx]) {
          inter->tokenizer->ip = inter->callstack[inter->depth-1];
        } else {
          --inter->depth;
        }
        break;
    }
  }
}

void bfck_free(struct bfck_inter *inter) {
  if (inter->tokenizer) {
    if (inter->tokenizer->buf) {
      free(inter->tokenizer->buf);
    }
    free(inter->tokenizer);
  }
}

static void shift_arg(int *argc, char **argv) {
  if (*argc < 1) {
    return;
  }
  memmove(argv, &argv[1], (*argc - 1) * sizeof(*argv));
  *argc -= 1;
}

static void parse_arg(int *argc, char **argv, struct bfck_inter *inter) {
  if (!strcmp(*argv, "--help")) {
    usage();
  } else if (!strcmp(*argv, "-c")) {
    if (*argc < 2) {
      fprintf(stderr, "Error: -c option requires an argument\n");
      usage();
      exit(1);
    }
    // If buffer already filled, just skip -c argument
    if (inter->tokenizer->buf) {
      shift_arg(argc, argv);
      shift_arg(argc, argv);
    } else {
      shift_arg(argc, argv);
      bfck_set_buffer(inter->tokenizer, strdup(*argv));
      shift_arg(argc, argv);
    }
  } else {
    // If buffer already filled with -c option, use that,
    // instead of refilling from file
    if (inter->tokenizer->buf) {
      shift_arg(argc, argv);
    } else {
      char *buf;
      if ((buf = read_file(*argv))) {
        bfck_set_buffer(inter->tokenizer, read_file(*argv));
        shift_arg(argc, argv);
      } else {
        exit(1);
      }
    }
  }
}

static void parse_args(int argc, char **argv, struct bfck_inter *inter) {
  shift_arg(&argc, argv);

  while (argc > 0) {
    parse_arg(&argc, argv, inter);
  }
}

int main(int argc, char **argv) {
  if (argc < 2) {
    usage();
  }
  struct bfck_inter bfck;
  if (bfck_init(&bfck)) {
    fprintf(stderr, "Error: %s\n", strerror(ENOMEM));
    return 1;
  }

  parse_args(argc, argv, &bfck);

  bfck_execute(&bfck);
  bfck_free(&bfck);
  return 0;
}
