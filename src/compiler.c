#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Values for token types
enum {
    TK_NUM = 256, // token for integer
    TK_EQ,
    TK_NE,
    TK_LE,
    TK_GE,
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
    ND_EQ,
    ND_NE,
    ND_LE,
    ND_GE,
};

typedef struct {
    void **data;
    int capacity;
    int len;
} Vector;

Vector *new_vector() {
    Vector *vec = malloc(sizeof(Vector));
    vec->data = malloc(sizeof(void *)* 16);
    vec->capacity = 16;
    vec->len = 0;
    return vec;
}

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
Node *add();
Node *equality();
Node *relational();
Node *term();
Node *mul();
Node *unary();

Node *equality() {
    Node *node = relational();

    for (;;) {
        if (consume(TK_EQ))
            node = new_node(ND_EQ, node, relational());
        else if (consume(TK_NE))
            node = new_node(ND_NE, node, relational());
        else
            return node;
    }
}

Node *relational() {
    // lhs
    Node *node = add();

    for (;;) {
        if (consume('<'))
            node = new_node('<', node, add());
        else if (consume(TK_LE))
            node = new_node(ND_LE, node, add());
        else if (consume('>'))
            node = new_node('<', add(), node);
        else if (consume(TK_GE))
            node = new_node(ND_LE, add(), node);
        else
            return node;
    }
}

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
    Node *node = unary();

    for (;;) {
        if (consume('*'))
            node = new_node('*', node, unary());
        else if (consume('/'))
            node = new_node('/', node, unary());
        else
            return node;
        
    }
}

Node *term() {
    if (consume('(')) {
        Node *node = equality();
        if (!consume(')')) {
            error("開き括弧に対する閉じ括弧がありません。：%s", tokens[pos].input);
        }
        return node;
    }

    if ( tokens[pos].ty == TK_NUM )
        return new_node_num(tokens[pos++].val);
    
    error("数値でも開き括弧でもないトークンです： %s ", tokens[pos].input);
}

Node *unary() {
    if (consume('+')) {
        return term();
    }
    if (consume('-')) {
        // -x => 0-x
        return new_node('-',new_node_num(0), term());
    }
    return term();
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

void tokenize(char *p) {
    int i = 0;
    while (*p) {
        if (isspace(*p)) {
            p++;
            continue;
        }

        if (strncmp(p,"<=",2) == 0){
            tokens[i].ty = TK_LE;
            tokens[i].input = p;
            i++;
            p = p + 2;
            continue;
        }

        if (strncmp(p,">=",2) == 0){
            tokens[i].ty = TK_GE;
            tokens[i].input = p;
            i++;
            p = p + 2;
            continue;
        }

        if ( *p == '+' || *p == '-' || *p == '*' || *p == '/' || *p == '(' || *p ==')' || *p == '<' || *p == '>' ) {
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

        if (strncmp(p,"==",2) == 0){
            tokens[i].ty = TK_EQ;
            tokens[i].input = p;
            i++;
            p = p + 2;
            continue;
        }

        if (strncmp(p,"!=",2) == 0){
            tokens[i].ty = TK_NE;
            tokens[i].input = p;
            i++;
            p = p + 2;
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
