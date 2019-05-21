#include "compiler.h"

int pos;
Tokens *tokens;

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
    tokens = new_tokens();
    tokenize(argv[1]);
    
    // tokens to syntax tree
    Node *node = equality();

    printf(".intel_syntax noprefix\n");
    printf(".global main\n");
    printf("main:\n");

    // syntax tree to asm
    gen(node);

    printf("  pop rax\n");
    printf("  ret\n");
    return 0;
}