#include "compiler.h"

int pos;
Tokens *tokens;
char *user_input;

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
    tokens = new_tokens();
    tokenize();
    
    // tokens to syntax tree
    program();

    printf(".intel_syntax noprefix\n");
    printf(".global main\n");
    printf("main:\n");

    printf("  push rbp\n");
    printf("  mov rbp, rsp\n");
    printf("  sub rsp, 208\n");

    // syntax tree to asm
    for (int i = 0; code[i]; i++) {
        gen(code[i]);
        printf("  pop rax\n");
    }

    printf("  mov rsp, rbp\n");
    printf("  pop rbp\n");
    printf("  ret\n");
    return 0;
}