#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    void **data;
    int capacity;
    int len;
} Vector;

// Type for tokens
typedef struct {
    int ty; // token type
    int val; // valu for Integer token
    char *input; // token stirng
} Token;


typedef struct {
    Vector *vec;
} Tokens;

typedef struct Node {
    int ty;
    struct Node *lhs;
    struct Node *rhs;
    int val;
    char name;
} Node;

// Values for token types
enum {
    TK_NUM = 256, // token for integer
    TK_IDENT,
    TK_EQ,
    TK_NE,
    TK_LE,
    TK_GE,
    TK_EOF, // token for EOF
};

enum {
    ND_NUM = 256,
    ND_IDENT,
    ND_EQ,
    ND_NE,
    ND_LE,
    ND_GE,
};

Vector *new_vector();
void vec_push(Vector *vec, void *elem);

Token *new_token(int ty, char *input);
Token *new_token_num(int ty, int val, char *input);
Tokens *new_tokens();
void append(Tokens *t, Token *elem);
Token *get(Tokens *t, int i);

void runtest();

Node *new_node(int ty, Node *lhs, Node *rhs);
Node *new_node_num(int val);
Node *new_node_ident(char *name);

int consume(int ty);
void tokenize();

void error(char *fmt, ...);

void program();
Node *add();
Node *equality();
Node *relational();
Node *term();
Node *mul();
Node *unary();

void gen();

extern char *user_input;
extern int pos;
extern Tokens *tokens;
extern Node *code[];
