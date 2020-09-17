#include <stdio.h>
#include <unistd.h>
#include "util.h"
#include "lexer.h"
#include "parser.h"
#include "interp.h"

enum {
  INTERPRET,
  PRINT_TOKENS,
  PRINT_PARSE_TREE,
};

int main(int argc, char **argv) {
  int mode = INTERPRET, opt;

  while ((opt = getopt(argc, argv, "lp")) != -1) {
    switch (opt) {
    case 'l':
      mode = PRINT_TOKENS;
      break;
    case 'p':
      mode = PRINT_PARSE_TREE;
      break;
    default:
      err_fatal("Unknown command-line option '%c'\n", opt);
    }
  }

  FILE *in;
  const char *filename;
  if (optind < argc) {
    // there was a command line argument after the option(s)
    filename = argv[optind];
    in = fopen(filename, "r");
    if (!in) {
      err_fatal("Could not open input file '%s'\n", filename);
    }
  } else {
    // no filename specified, so read from stdin
    filename = "<stdin>";
    in = stdin;
  }

  struct Lexer *lexer = lexer_create(in, filename);
  if (mode == PRINT_TOKENS) {
    struct Node *tok;
    while ((tok = lexer_next(lexer)) != NULL) {
      printf("%d:%s\n", node_get_tag(tok), node_get_str(tok));
    }
    return 0;
  }

  struct Parser *parser = parser_create(lexer);
  struct Node *root = parser_parse(parser);

  if (mode == PRINT_PARSE_TREE) {
    // just print a parse tree
    printf("Parse tree:\n");
    parser_print_parse_tree(root);
  } else {
    // evaluate and print result

    struct Interpreter *interp = interp_create(root);
    long result = interp_exec(interp);
    printf("Result: %ld\n", result);

  }

  parser_destroy(parser);

  return 0;
}
