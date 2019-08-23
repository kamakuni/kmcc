#include "kmcc.h"

int pos;
Tokens *tokens;
Token *token;
char *user_input;
Var *variables;
char *filename;

int main(int argc, char **argv) {
    if (argc != 2) {
        fprintf(stderr, "引数の個数が正しくありません\n");
        return 1;
    }
    // run testing codes
    if (strcmp(argv[1],"-test") == 0){
        runtest();
        return 0;
    }
    
    pos = 0;
    user_input = argv[1];
    variables = new_var();
    tokens = new_tokens();
    tokenize();
    
    // tokens to syntax tree
    program();
    
    printf(".intel_syntax noprefix\n");
    printf(".global main\n");

    for (int i = 0; code[i]; i++) {
        gen_func(code[i]);
    }

    return 0;
}
