#include "compiler.h"

int label_count = 0;
char* argregs[] = {"rdi", "rsi", "rdx", "rcx", "r8", "r9"};

void gen_lval(Node *node) {
    if (node->ty != ND_IDENT)
        error("代入の左辺値が変数でありません。");
    
    int offset = var_get_offset(variables, node->name);
    printf("  mov rax, rbp\n");
    printf("  sub rax, %d\n", offset);
    printf("  push rax\n");
}

void gen_block(Node *node){
    for (int i = 0; i < node->stmts->len; i++) {
        gen((Node *)vec_get(node->stmts, i));
        printf("  pop rax\n");
    }
}

void gen_func(Node *node){
    if(node->ty != ND_FUNC)
        error("関数でありません。");
    printf("%s:\n",node->name);
    printf("  push rbp\n");
    printf("  mov rbp, rsp\n");
    int buf = (node->args->len + var_len(variables)) * 8;
    printf("  sub rsp, %d\n", buf);
    for (int i = 0; i < node->args->len; i++) {
    //for (int i = node->args->len; i >= 0 ; i--) {
        int offset = var_get_offset(variables,vec_get(node->args,i));
        if (offset != 0) {
            printf("  mov rax, rbp\n");
            printf("  sub rax, %d\n", offset);
            printf("  mov [rax], %s\n", argregs[i]);
        }
    }
    if(node->block->ty != ND_BLOCK)
        error("ブロックでありません。");
    gen_block(node->block);

    printf("  mov rsp, rbp\n");
    printf("  pop rbp\n");
    printf("  ret\n");
}

void gen(Node *node) {

    if(node->ty == ND_CALL) {
        int len = node->args->len;
        for (int i = 0; i < len; i++) {
            gen((Node *) vec_get(node->args,i));
            printf("  pop rax\n");
            //printf("  mov %s, rax\n", argregs[len - (i + 1)]);
            printf("  mov %s, rax\n", argregs[i]);
            //printf("  mov %s, %d\n", argregs[i], (int) vec_get(node->args,i));
        }

        printf("  call %s\n",node->name);
        printf("  push rax\n");
        return;
    }

    if (node->ty == ND_BLOCK) {
        gen_block(node->block);
        return;
    }

    if (node->ty == ND_NUM) {
        printf("  push %d\n", node->val);
        return;
    }
    
    if (node->ty == ND_IDENT) {
        gen_lval(node);
        printf("  pop rax\n");
        printf("  mov rax, [rax]\n");
        printf("  push rax\n");
        return;
    }
    
    if (node->ty == ND_IF) {
        int label_count_if = label_count;
        label_count += 1;
        gen(node->cond);
        printf("  pop rax\n");
        printf("  cmp rax, 0\n");
        if(node->elseBody == NULL){
            printf("  je  .Lend%d\n", label_count_if);
            if(node->body->ty == ND_BLOCK){
                gen_block(node->body);
            } else {
                gen(node->body);
            }
            printf(".Lend%d:\n", label_count_if);
        } else {
            printf("  je  .Lelse%d\n", label_count_if);
            if(node->body->ty == ND_BLOCK){
                gen_block(node->body);
            } else {
                gen(node->body);
            }
            printf("  jmp  .Lend%d\n", label_count_if);
            printf(".Lelse%d:\n", label_count_if);
            if(node->elseBody->ty == ND_BLOCK){
                gen_block(node->elseBody);
            } else {
                gen(node->elseBody);
            }
            printf(".Lend%d:\n", label_count_if);
        }
        return;
    }

    if (node->ty == ND_WHILE) {
        int label_count_while = label_count;
        label_count += 1;
        printf(".Lbegin%d:\n", label_count_while);
        gen(node->cond);
        printf("  pop rax\n");
        printf("  cmp rax, 0\n");
        printf("  je  .Lend%d\n", label_count_while);
        if(node->body->ty == ND_BLOCK){
            gen_block(node->body);
        } else {
            gen(node->body);
        }
        printf("  jmp  .Lbegin%d\n", label_count_while);
        printf(".Lend%d:\n", label_count_while);
        return;
    }

    if (node->ty == ND_FOR) {
        int label_count_for = label_count;
        label_count += 1;
        if(node->init) {
            gen(node->init);
        }
        printf(".Lbegin%d:\n", label_count_for);
        if(node->cond) {
            gen(node->cond);
            printf("  pop rax\n");
            printf("  cmp rax, 0\n");
            printf("  je  .Lend%d\n", label_count_for);
        }
        gen(node->body);
        if(node->incdec)
          gen(node->incdec);
        printf("  jmp  .Lbegin%d\n", label_count_for);
        printf(".Lend%d:\n", label_count_for);
        return;
    }

    if (node->ty == ND_RETURN) {
        gen(node->lhs);
        printf("  pop rax\n");
        printf("  mov rsp, rbp\n");
        printf("  pop rbp\n");
        printf("  ret\n");
        return;
    }
    
    if (node->ty == '=') {
        gen_lval(node->lhs);
        gen(node->rhs);
        
        printf("  pop rdi\n");
        printf("  pop rax\n");
        printf("  mov [rax], rdi\n");
        printf("  push rdi\n");
        return;
    }
    
    gen(node->lhs);
    gen(node->rhs);
    
    printf("  pop rdi\n");
    printf("  pop rax\n");
    
    switch (node->ty) {
        case '+':
            printf("  add rax, rdi\n");
            break;
        case '-':
            printf("  sub rax, rdi\n");
            break;
        case '*':
            printf("  mul rdi\n");
            break;
        case '/':
            printf("  mov rdx, 0\n");
            printf("  div rdi\n");
            break;
        case '<':
            printf("  cmp rax, rdi\n");
            printf("  setl al\n");
            printf("  movzb rax, al\n");
            break;
        case ND_LE:
            printf("  cmp rax, rdi\n");
            printf("  setle al\n");
            printf("  movzb rax, al\n");
            break;
        case ND_EQ:
            printf("  cmp rax, rdi\n");
            printf("  sete al\n");
            printf("  movzb rax, al\n");
            break;
        case ND_NE:
            printf("  cmp rax, rdi\n");
            printf("  setne al\n");
            printf("  movzb rax, al\n");
    }
    
    printf("  push rax\n");
}
