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
    int val; // value for Integer token
    char *name; // name for Ident
    char *input; // token stirng for debugging
} Token;


typedef struct {
    Vector *vec;
} Tokens;

typedef struct {
    Vector *keys;
    Vector *vals;
} Map;

typedef struct Var {
    struct Var *next;
    char *name;
    int offset;
} Var;

typedef struct Node {
    int ty;
    struct Node *lhs;
    struct Node *rhs;
    struct Node *cond; // for ND_IF | ND_FOR | ND_WHILE
    struct Node *block; // for ND_FUNC
    struct Node *body; // for ND_IF | ND_FOR | ND_WHILE
    struct Node *elseBody; // for ND_IF
    struct Node *init; // for ND_FOR
    struct Node *incdec; // for ND_FOR
    Vector *stmts; // for ND_BLOCK | ND_FUNC
    Vector *args; // for ND_CALL
    int val;
    char *name;
} Node;

// Values for token types
enum {
    TK_NUM = 256, // token for integer
    TK_IDENT,
    TK_IF,
    TK_INT,
    TK_ELSE,
    TK_WHILE,
    TK_FOR,
    TK_RETURN,
    TK_EQ,
    TK_NE,
    TK_LE,
    TK_GE,
    TK_EOF, // token for EOF
};

enum {
    ND_NUM = 256,
    ND_BLOCK,
    ND_CALL,
    ND_FUNC,
    ND_IDENT,
    ND_IF,
    ND_WHILE,
    ND_FOR,
    ND_RETURN,
    ND_EQ,
    ND_NE,
    ND_LE,
    ND_GE,
    ND_EOF,
    ND_ADDR,
    ND_DEREF,
};

Vector *new_vector();
void vec_push(Vector *vec, void *elem);
void *vec_get(Vector *vec, int i);

Map *new_map();
void map_put(Map *map, char *key, void *val);
Map *map_get(Map *map,char *key);

Token *new_token(int ty, char *input);
Token *new_token_num(int val, char *input);
//Token *new_token_ident(char *name);
Token *new_token_ident(char *input, int len);

Tokens *new_tokens();
void append(Tokens *t, Token *elem);
Token *get(Tokens *t, int i);

void runtest();

Node *new_node(int ty, Node *lhs, Node *rhs);
Node *new_node_num(int val);
Node *new_node_ident(char *name);
Node *new_node_if(Node *ifCond, Node *ifBody, Node *elseBody);

Var *new_var();
void var_insert_first(Var **var, char *name, int offset);
int var_get_offset(Var *var, char *name);
int var_len(Var *var);

int consume(int ty);
void tokenize();

int is_alnum(char c);
void error(char *fmt, ...);

void program();
Node *function();
Node *stmt();
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
extern Var *variables;
