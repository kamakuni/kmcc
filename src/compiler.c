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
    // lhs
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
        }
        return node;
    }

    if ( tokens[pos].ty == TK_NUM )
        return new_node_num(tokens[pos++].val);
    
    error("数値でも開き括弧でもないトークンです： %s ", tokens[pos].input);
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
        if ( *p == '+' || *p == '-' || *p == '*' || *p == '/' || *p == '(' || *p ==')' ) {
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
    // tokens to syntax tree
    Node *node = add();

    printf(".intel_syntax noprefix\n");
    printf(".global main\n");
    printf("main:\n");

    // syntax tree to asm
    gen(node);

    printf("  pop rax\n");
    printf("  ret\n");
    return 0;
}
