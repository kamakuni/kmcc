//int main() { enum { zero, one, two}; return zero; }
//int g1; int *g1_ptr() {return &g1;} int main() { return *g1_ptr(); }
//int main() {return -1;}
//int main() { int i=2; i+=5; return i;}
//int g1 = 1;
//int main() {
//    return 0;
//}
//int main() { struct t {char a[2];}; { struct t {char a[4];}; } struct t y; return sizeof(y); }
//int main() {char g = 'c'; return g; };
//typedef struct FILE FILE;
/*
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

typedef struct Type Type;
typedef struct Member Member;

struct Type {
  TypeKind kind;
  int size; // sizeof() value
  int align; // alignment
  _Bool is_incomplete; // incomplete type
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

Type *void_type = &(Type){ TY_VOID, 1, 1 };
*/
typedef enum {
  TYPEDEF = 1 << 0,
} StorageClass;

int basetype(StorageClass *sclass) {
  if (*sclass & (*sclass - 1))
    return 1;
  return 0;
}

int main() {
  /*int type = ND_ADD;
  switch(type) {
    case ND_ADD:
      return 0;
    default:
      return 1;
  }*/
  StorageClass sclass;
  return basetype(&sclass);
}
