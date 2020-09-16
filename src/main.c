#include "kmcc.h"

Tokens *tokens;
Token *token;
char *filename;
char *user_input;

int main(int argc, char **argv) {

  if (argc != 2)
    error("%s: invalid number of arguments", argv[0]);
    
  // Run testing codes.
  if (strcmp(argv[1],"-test") == 0){
    //runtest();
    return 0;
  }

  // Tokenize and parse.
  user_input = argv[1];
  token = tokenize(user_input);
  Program *prog = program();

  // Assign offsets to local variables.
  for (Function *fn = prog->fns; fn; fn = fn->next) {
    int offset = 0;
    for (VarList *vl = fn->locals; vl; vl = vl->next) {
	Var *var = vl->var;
	offset += var->ty->size;
	var->offset = offset;
    }
    fn->stack_size = offset;
  }

  // Traverse the AST to emit assembly.
  codegen(prog);
  
  return 0;
}
