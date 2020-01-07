#include "kmcc.h"

Tokens *tokens;
Token *token;
char *filename;
char *user_input;

int main(int argc, char **argv) {

  if (argc != 2)
    error("%s: invalid number of arguments", argv[0]);
    
  // run testing codes
  if (strcmp(argv[1],"-test") == 0){
    //runtest();
    return 0;
  }

  // Tokenize and parse
    user_input = argv[1];
    token = tokenize(user_input);
    
    // tokens to syntax tree
    Function *prog = program();

    for (Function *fn = prog; fn; fn = fn->next) {
      int offset = 0;
      for (Var *var = prog->locals; var; var = var->next) {
	offset += 8;
	var->offset = offset;
      }
      fn->stack_size = offset;
    }

    codegen(prog);
    return 0;
}
