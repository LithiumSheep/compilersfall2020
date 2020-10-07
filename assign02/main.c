#include <stdio.h>
#include <stdlib.h>
#include <unistd.h> // for getopt
#include "node.h"
#include "util.h"
#include "grammar_symbols.h"
#include "treeprint.h"
#include "interp.h"

int yyparse(void);

void print_usage(void) {
  err_fatal(
    "Usage: interp [options] <filename>\n"
    "Options:\n"
    "   -p    print parse tree\n"
  );
}

int main(int argc, char **argv) {
  extern FILE *yyin;
  extern struct Node *g_translation_unit;

  int print_parse_tree = 0;
  int opt;

  while ((opt = getopt(argc, argv, "p")) != -1) {
    switch (opt) {
    case 'p':
      print_parse_tree = 1;
      break;

    case '?':
      print_usage();
    }
  }

  if (optind >= argc) {
    print_usage();
  }

  const char *filename = argv[optind];

  yyin = fopen(filename, "r");
  if (!yyin) {
    err_fatal("Could not open input file \"%s\"\n", filename);
  }
  lexer_set_source_file(filename);

  yyparse();

  if (print_parse_tree) {
    treeprint(g_translation_unit, get_grammar_symbol_name);
  } else {
    struct Interp *interp = interp_create(g_translation_unit);
    struct Value val = interp_exec(interp);
    char *result_as_str = val_stringify(val);
    printf("Result: %s\n", result_as_str);
    free(result_as_str);
    interp_destroy(interp);
  }

  return 0;
}
