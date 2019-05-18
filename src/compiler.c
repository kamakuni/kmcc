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

void vec_push(Vector *vec, void *elem) {
    if (vec->capacity == vec->len) {
        vec->capacity *= 2;
        vec->data = realloc(vec->data, sizeof(void *) * vec->capacity );
    }
    vec->data[vec->len++] = elem;
}

// Type for tokens
typedef struct {
    int ty; // token type
    int val; // valu for Integer token
    char *input; // token stirng
} Token;

Token *new_token(int ty, char *input) {
    Token *t = malloc(sizeof(Token));
    t->ty = ty;
    t->input = input;
    return t;
}

Token *new_token_num(int ty, int val, char *input) {
    Token *t = malloc(sizeof(Token));
    t->ty = ty;
    t->val = val;
    t->input = input;
    return t;
}

typedef struct {
    Vector *vec;
} Tokens;

Tokens *new_tokens(){
    Tokens *t = malloc(sizeof(Tokens));
    //t->i = 0;
    t->vec = new_vector();
    return t;
}

Tokens *tokens;

void append(Tokens *t, Token *elem) {
    vec_push(t->vec,(void *) elem);
}

Token *get(Tokens *t, int i) {
    return (Token *) t->vec->data[i];
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
    if (get(tokens, pos)->ty != ty)
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
            error("開き括弧に対する閉じ括弧がありません。：%s", get(tokens,pos)->input);
        }
        return node;
    }

    if ( get(tokens,pos)->ty == TK_NUM )
        return new_node_num(get(tokens,pos++)->val);
    
    error("数値でも開き括弧でもないトークンです： %s ", get(tokens,pos)->input);
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

    while (*p) {
        if (isspace(*p)) {
            p++;
            continue;
        }

        if (strncmp(p,"<=",2) == 0){
            append(tokens, new_token(TK_LE,p));
            p = p + 2;
            continue;
        }

        if (strncmp(p,">=",2) == 0){
            append(tokens, new_token(TK_GE,p));
            p = p + 2;
            continue;
        }

        if ( *p == '+' || *p == '-' || *p == '*' || *p == '/' || *p == '(' || *p ==')' || *p == '<' || *p == '>' ) {
            append(tokens, new_token(*p,p));
            p++;
            continue;
        }

        if (isdigit(*p)) {
            append(tokens, new_token_num(TK_NUM, strtol(p, &p, 10), p));
            continue;
        }

        if (strncmp(p,"==",2) == 0){
            append(tokens, new_token(TK_EQ,p));
            p = p + 2;
            continue;
        }

        if (strncmp(p,"!=",2) == 0){
            append(tokens, new_token(TK_NE,p));
            p = p + 2;
            continue;
        }

        error("トークナイズできません: %s", p);
        exit(1);
    }

    append(tokens, new_token(TK_EOF,p));
}

int expect(int line, int expected, int actual) {
    if (expected == actual) 
        return 0;
    fprintf(stderr,"%d: %d expected, but got %d\n",line, expected, actual);
    exit(1);
}

void runtest();

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

void runtest(){
    Vector *vec = new_vector();
    expect(__LINE__, 0, vec->len);

    for (int i = 0; i < 100; i++)
        vec_push(vec,(void *)i);

    expect(__LINE__, 100, vec->len);
    expect(__LINE__, 0, (long)vec->data[0]);
    expect(__LINE__, 50, (long)vec->data[50]);
    expect(__LINE__, 99, (long)vec->data[99]);

    Tokens *tokens = new_tokens();
    append(tokens, new_token_num(1,2,"3"));
    Token *token = get(tokens,0);

    expect(__LINE__, 1, token->ty);
    expect(__LINE__, 2, token->val);
    expect(__LINE__, 0, strcmp("3", token->input));

    append(tokens, new_token_num(2,3,"4"));

    token = get(tokens,1);

    expect(__LINE__, 2, token->ty);
    expect(__LINE__, 3, token->val);
    expect(__LINE__, 0, strcmp("4", token->input));

    printf("OK\n");
}