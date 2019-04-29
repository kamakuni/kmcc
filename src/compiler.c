#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Values for token types
enum {
    TK_NUM = 256, // token for integer
    TK_EOF, // token for EOF
};

// Type for tokens
typedef struct {
    int ty; // token type
    int val; // valu for Integer token
    char *input; // token stirng
} Token;

// array for tokenized result
Token tokens[100];

enum {
    ND_NUM = 256,
};

typedef struct Node {
    int ty;
    struct Node *lhs;
    struct Node *rhs;
    int val;
} Node;

Node *new_node(int ty, Node *lhs, Node *rhs) {
    Node *node = malloc(sizeof(Node));
    node->ty = ty;
    node->lhs = lhs;
    node->rhs = rhs;
    return node;
}

Node *new_node_num(int val) {
    Node *node = malloc(sizeof(Node));
    node->ty = ND_NUM;
    node->val = val;
    return node;
}

int pos = 0;

int consume(int ty) {
    if (tokens[pos].ty != ty)
        return 0;
    pos++;
    return 1;
}

void error(char *fmt, ...);
Node *term();
Node *mul();

Node *add() {
    Node *node = mul();

    for (;;) {
        if (consume('+'))
            node = new_node('+', node, mul());
        else if (consume('-'))
            node = new_node('-', node, mul());
        else
            return node;
    }
}

Node *mul() {
    Node *node = term();

    for (;;) {
        if (consume('*'))
            node = new_node('*', node, term());
        else if (consume('/'))
            node = new_node('/', node, term());
        else
            return node;
        
    }
}

Node *term() {
    if (consume('(')) {
        Node *node = add();
        if (!consume(')')) {
            error("開き括弧に対する閉じ括弧がありません。：%s", tokens[pos].input);
            return node;
        }

        if ( tokens[pos].ty == TK_NUM )
            return new_node_num(tokens[pos++].val);
        
        error("数値でも開き括弧でもないトークンです： %s ", tokens[pos].input);
    }
}

void error(char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, "\n");
    exit(1);
}

void gen(Node *node) {
    if (node->ty == ND_NUM) {
        printf("  push %d\n", node->val);
        return;
    }

    gen(node->lhs);
    gen(node->rhs);

    printf("  pop rdi\n");
    printf("  pop rax\n");

    switch (node->)
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
    }

    printf("  push rax\n");
}

void tokenize(char *p) {
    int i = 0;
    while (*p) {
        if (isspace(*p)) {
            p++;
            continue;
        }
        if ( *p == '+' || *p == '-' ) {
            tokens[i].ty = *p;
            tokens[i].input = p;
            i++;
            p++;
            continue;
        }

        if (isdigit(*p)) {
            tokens[i].ty = TK_NUM;
            tokens[i].input = p;
            tokens[i].val = strtol(p, &p, 10);
            i++;
            continue;
        }

        error("トークナイズできません: %s", p);
        exit(1);
    }

    tokens[i].ty = TK_EOF;
    tokens[i].input = p; 
}

int main(int argc, char **argv) {
    if (argc != 2) {
        fprintf(stderr, "引数の個数が正しくありません\n");
        return 1;
    }
    
    tokenize(argv[1]);

    printf(".intel_syntax noprefix\n");
    printf(".global main\n");
    printf("main:\n");


    if (tokens[0].ty != TK_NUM) 
        error("最初の項が数ではありません");
    printf("  mov rax, %d\n", tokens[0].val);

    int i = 1;
    while(tokens[i].ty != TK_EOF){
        if (tokens[i].ty == '+') {
            i++;
            if (tokens[i].ty != TK_NUM)
                error("予期しないトークンです: %s",tokens[i].input);
            printf("  add rax, %d\n", tokens[i].val);
            i++;
            continue;
        }

        if (tokens[i].ty == '-') {
            i++;
            if (tokens[i].ty != TK_NUM)
                error("予期しないトークンです: %s",tokens[i].input);
            printf("  sub rax, %d\n", tokens[i].val);
            i++;
            continue;
        }

        error("予期しないトークンです：%s", tokens[i].input);
    }

    printf("  ret\n");
    return 0;
}
