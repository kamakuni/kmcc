#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

typedef struct Vector Vector;
struct Vector{
    void **data;
    int capacity;
    int len;
};

// Values for token types
typedef enum {
    TK_RESERVED, // Keywords or punctuators
    TK_NUM, // Integer literals
    TK_IDENT, // Identifiers
    TK_EOF, // End-of-file markers
} TokenKind;

typedef struct Token Token;
// Type for tokens
struct Token {
    TokenKind kind; // token kind
    Token *next;
    int len; // token length
    long val; // value for Integer token
    char *name; // name for Ident
    char *str; // token stirng for debugging
};

typedef struct Tokens Tokens;
struct Tokens {
    Vector *vec;
};

typedef struct Map Map;
struct Map {
    Vector *keys;
    Vector *vals;
};

typedef struct Type Type;
struct Type {
    enum { INT, PTR } ty;
    Type *ptr_to;
};

typedef struct Var Var;
struct Var {
    Var *next;
    char *name;
    Type *ty;
    int offset;
};

typedef struct Node Node;
struct Node {
    int kind;
    Node *lhs;
    Node *rhs;
    Node *cond; // for ND_IF | ND_FOR | ND_WHILE
    Node *block; // for ND_FUNC
    Node *body; // for ND_IF | ND_FOR | ND_WHILE
    Node *elseBody; // for ND_IF
    Node *init; // for ND_FOR
    Node *incdec; // for ND_FOR
    Vector *stmts; // for ND_BLOCK | ND_FUNC
    Vector *args; // for ND_CALL
    Type *ty;
    int val;
    char *name;
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
    ND_LT,
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

Tokens *new_tokens();
void append(Tokens *t, Token *elem);
Token *get(Tokens *t, int i);

void runtest();

Node *new_node(int kind, Node *lhs, Node *rhs);
Node *new_node_num(long val);
Node *new_node_ident(Type *ty, char *name);
Node *new_node_if(Node *ifCond, Node *ifBody, Node *elseBody);

Var *new_var();
Var *var_get(Var *var,char *name);
void var_insert_first(Var **var, Type *ty, char *name, int offset);
int var_get_offset(Var *var, char *name);
int var_len(Var *var);

Token *peek(char *s);
bool consume(char *s);
Token *consume_ident(void);
//int expect(int line, int expected, int actual);
void expect(char *op);
long expect_number();
char *expect_ident();
bool at_eof();
Token *tokenize(char *p);

int is_alnum(char c);
char *strndup(char *p,int len);
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

extern char *filename;
extern char *user_input;
extern int pos;
extern Token *token;
extern Tokens *tokens;
extern Node *code[];
extern Var *variables;