#include "kmcc.h"

int pos;
Tokens *tokens;
Token *token;
Var *variables;
char *filename;
char *user_input;

int main(int argc, char **argv) {
    if (argc != 2) {
        fprintf(stderr, "引数の個数が正しくありません\n");
        return 1;
    }
    
    // run testing codes
    if (strcmp(argv[1],"-test") == 0){
        //runtest();
        return 0;
    }
    
    pos = 0;
    variables = new_var();
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
    /*printf(".intel_syntax noprefix\n");
    printf(".global main\n");

    for (int i = 0; code[i]; i++) {
        gen_func(code[i]);
    }
    */
    return 0;
}
