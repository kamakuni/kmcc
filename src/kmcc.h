#include <ctype.h>
#include <errno.h>
#include <limits.h>
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
    TK_STR, // Strings
    TK_EOF, // End-of-file markers
    TK_SIZEOF, // sizeof
} TokenKind;

typedef struct Token Token;
// Type for tokens
struct Token {
    TokenKind kind; // token kind
    Token *next;
    int len; // token length
    long val; // value for Integer token
    char *str; // token stirng for debugging
    char *contents; // String literal contents including terminating '\0'
    char cont_len; //  String literal length
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
typedef struct Member Member;
typedef struct Initializer Initializer;

// Variable
typedef struct Var Var;
struct Var {
  char *name; // Variable name
  Type *ty; // Type
  bool is_local; // Local or global
  int offset; // Offset from RBP
  // Global variable
  bool is_static;
  Initializer *initializer;
};

typedef struct LVar LVar;
struct LVar {
    LVar *next;
    char *name;
    int len;
  int offset;
};

typedef struct VarList VarList;
struct VarList {
  VarList *next;
  Var *var;
};

typedef struct Node Node;
struct Node {
  int kind;
  Node *next;
  Node *lhs;
  Node *rhs;
  Node *cond; // for ND_IF | ND_FOR | ND_WHILE
  Node *then;
  Node *body; // for ND_IF | ND_FOR | ND_WHILE
  Node *els; // for ND_IF
  Node *init; // for ND_FOR
  Node *inc; // for ND_FOR
  // Struct member access
  Member *member;
  // Function call
  char *funcname;
  Node *args; // for ND_CALL
  Type *ty;
  Token *tok;
  long val; // Integer literal
  Var *var;
};

// Global variable initializer. Global variables can be initialized
// either by a constant expression or a pointer to another global
// variable with an added.
struct Initializer {
  Initializer *next;

  // Constant expression
  int sz;
  long val;

  // Reference to another global variable
  char *label;
  long *addend;
};

typedef enum {
    ND_ADD,
    ND_ASSIGN,
    ND_BITAND,
    ND_BITOR,
    ND_BITXOR,
    ND_BITNOT,
    ND_PRE_INC, // pre ++
    ND_PRE_DEC, // pre --
    ND_POST_INC, // post ++
    ND_POST_DEC, // post --
    ND_ADD_EQ, // +=
    ND_PTR_ADD_EQ, // =+
    ND_SUB_EQ, // -=
    ND_PTR_SUB_EQ, // -=
    ND_MUL_EQ, // *=
    ND_DIV_EQ, // /=
    ND_COMMA,// ,
    ND_SHL, // <<
    ND_SHR, // >>
    ND_MEMBER,
    ND_EXPR_STMT, // Expression statemnt
    ND_STMT_EXPR, // Statement expression
    ND_SUB,
    ND_MUL,
    ND_DIV,
    ND_NOT,
    ND_NUM, // Integer
    ND_CAST, // Type cast
    ND_NULL, // Empty statement
    ND_BLOCK,
    ND_FUNCALL,
    ND_IF,
    ND_WHILE,
    ND_FOR,
    ND_RETURN,
    ND_EQ,
    ND_NE,
    ND_LT,
    ND_LE,
    ND_SHL_EQ, // <<=
    ND_SHR_EQ, // >>=
    ND_LOGAND,
    ND_LOGOR,
    ND_TERNARY,
    ND_ADDR,
    ND_DEREF,
    ND_VAR,
    ND_PTR_DIFF,
    ND_PTR_ADD,
    ND_PTR_SUB,
} NodeKind;

typedef struct Function Function;
struct Function {
  Function *next;
  char *name;
  VarList *params;
  bool is_static;
  Node *node;
  Var *locals;
  int stack_size;
};

typedef struct {
  VarList *globals;
  Function *fns;
} Program;

Program *program(void);

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

Node *new_node(NodeKind kind, Token *tok);
Node *new_binary(NodeKind kind, Node *lhs, Node *rhs, Token *tok);
Node *new_num(long val, Token *tok);
Node *new_node_ident(Type *ty, char *name);
Node *new_node_if(Node *ifCond, Node *ifBody, Node *els);

Var *var_get(Var *var,char *name);
void var_insert_first(Var **var, Type *ty, char *name, int offset);
int var_get_offset(Var *var, char *name);
int var_len(Var *var);

Token *peek(char *s);
Token *consume(char *op);
Token *consume_ident(void);
//int expect(int line, int expected, int actual);
void expect(char *s);
long expect_number();
char *expect_ident();
bool at_eof();
Token *tokenize(char *p);

bool is_alnum(char c);
bool is_alpha(char c);

//char *strndup(char *p,int len);
void error(char *fmt, ...);
void error_tok(Token *tok, char *fmt, ...);
void warn_tok(Token *tok, char *fmt, ...);

typedef enum { 
  TY_VOID,
  TY_BOOL,
  TY_CHAR,
  TY_SHORT,
  TY_INT,
  TY_LONG,
  TY_ENUM,
  TY_PTR,
  TY_ARRAY,
  TY_STRUCT,
  TY_FUNC,
} TypeKind;

struct Type {
  TypeKind kind;
  int size; // sizeof() value
  int align; // alignment
  bool is_incomplete; // incomplete type
  Type *base; // pointer or array
  int array_len; // array
  Member *members; // struct
  Type *return_ty; // function
};

// Struct Member
struct Member {
  Member *next;
  Type *ty;
  Token *tok; // for rror message
  char *name;
  int offset;
};

extern Type *void_type;
extern Type *bool_type;
extern Type *char_type;
extern Type *short_type;
extern Type *int_type;
extern Type *long_type;

bool is_integer(Type *ty);
int align_to(int n, int align);
Type *pointer_to(Type *base);
Type *array_of(Type *base, int size);
Type *struct_type(void);
Type *func_type(Type *return_ty);
Type *enum_type();
Type *struct_type(void);
void add_type(Node *node);

//Node *function();
//Node *stmt();
//Node *add();
//Node *equality();
//Node *relational();
//Node *primary();
//Node *mul();
//Node *unary();
void codegen(Program *prog);
//void gen();

extern char *filename;
extern char *user_input;
extern int pos;
extern Token *token;
extern Tokens *tokens;
extern Node *code[];
extern Var *variables;
