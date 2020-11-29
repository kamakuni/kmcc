#include "kmcc.h"

// Scope for struct tags
typedef struct TagScope TagScope;
struct TagScope {
  TagScope *next;
  char *name;
  Type *ty;
};

typedef struct {
  VarList *var_scope;
  TagScope *tag_scope;
} Scope;

// All local variable instances created during parsing are
// accumulated to this list.
static VarList *locals;
// Likewise, global variables are accumulated to this list.
static VarList *globals;

// C has two block scopes; one is or variables nd the other is
// for struct tags.
VarList *var_scope;
static TagScope *tag_scope;

// Begin a block scope
static Scope *enter_scope(){
  Scope *sc = calloc(1, sizeof(Scope));
  sc->var_scope = var_scope;
  sc->tag_scope = tag_scope;
  return sc;
}

// End a block scope
static void leave_scope(Scope *sc){
  var_scope = sc->var_scope;
  tag_scope = sc->tag_scope;
}

// Find a variable by name
static Var *find_var(Token *tok) {
  for (VarList *vl = var_scope; vl; vl = vl->next) {
    Var *var = vl->var;
    if (strlen(var->name) == tok->len && !strncmp(tok->str, var->name, tok->len))
      return var;
  }
  return NULL;
}

static TagScope *find_tag(Token *tok) {
  for (TagScope *sc = tag_scope; sc; sc = sc->next)
    if (strlen(sc->name) == tok->len && !strncmp(tok->str, sc->name, tok->len))
      return sc;
  return NULL;
}

Node *new_node(NodeKind kind, Token *tok) {
    Node *node = calloc(1,sizeof(Node));
    node->kind = kind;
    node->tok = tok;
    return node;
}

Node *new_binary(NodeKind kind, Node *lhs, Node *rhs, Token *tok) {
  Node *node = new_node(kind,tok);
    node->lhs = lhs;
    node->rhs = rhs;
    return node;
}

Node *new_num(long val, Token *tok) {
  Node *node = new_node(ND_NUM, tok);
    node->val = val;
    return node;
}

static Node *new_var_node(Var *var, Token *tok) {
  Node *node = new_node(ND_VAR, tok);
  node->var = var;
  return node;
}

static Var *new_var(char *name, Type *ty, bool is_local) {
  Var *var = calloc(1,sizeof(Var));
  var->name = name;
  var->ty = ty;
  var->is_local = is_local;

  VarList *sc = calloc(1, sizeof(VarList));
  sc->var = var;
  sc->next = var_scope;
  var_scope = sc;
  return var;
}

static Var *new_lvar(char *name, Type *ty){
  Var *var = new_var(name, ty, true);
  VarList *vl = calloc(1,sizeof(VarList));
  vl->var = var;
  vl->next = locals;
  locals = vl;
  return var;
}

static Var *new_gvar(char *name, Type *ty) {
  Var *var = new_var(name, ty, false);
  VarList *vl = calloc(1,sizeof(VarList));
  vl->var = var;
  vl->next = globals;
  globals = vl;
  return var;
}

static Node *new_unary(NodeKind kind, Node *expr, Token *tok){
  Node *node = new_node(kind, tok);
  node->lhs = expr;
  return node;
}

Node *code[100];

static char *new_label() {
  static int cnt = 0;
  char buf[20];
  sprintf(buf, ".L.data.%d", cnt++);
  return strndup(buf, 20);
}

static Function *function();
static Node *stmt();
static Node *stmt2();
static Node *expr();
static long eval(Node *node);
static long const_expr();
static Node *new_add(Node *lhs, Node *rhs, Token *tok);
static Node *add();
static Node *assign();
static Node *bitand();
static Node *bitor();
static Node *bitxor();
static Node *conditional();
static Node *equality();
static Node *relational();
static Node *primary();
static Node *mul();
static Node *unary();      
static void global_var();
static Type *struct_decl();
static Member *struct_member();
static bool is_typename();

// basetype = ("void" | "char" | "int" | struct-decl) "*"*
static Type *basetype(void) {
  if (!is_typename())
    error_tok(token, "typename expected");
  Type *ty;
  if (consume("void"))
    ty = void_type;
  else if (consume("char"))
    ty = char_type;
  else if (consume("int"))
    ty = int_type;
  else
    ty = struct_decl();

  while (consume("*"))
    ty = pointer_to(ty);
  return ty;
}

static is_function() {
  // To keep current token
  Token *tok = token;
  basetype();
  bool is_func = consume_ident() && consume("(");
  token = tok; // args of ) token
  return is_func;
}

// program = (grobal var | function)*
Program *program() {
  Function head = {};
  Function *cur = &head;
  globals = NULL;

  while (!at_eof()) {
    if(is_function()) {
      cur->next = function();
      cur = cur->next;
    } else {
      global_var();
    }
  }

  Program *prog = calloc(1, sizeof(Program));
  prog->globals = globals;
  prog->fns = head.next;
  return prog;
}

// type-suffix = ("[" const_expr? "]" type_suffix)?
static Type *type_suffix(Type *ty) {
  if (!consume("["))
    return ty;
  int sz = 0;
  bool is_incomplete = true;
  if (!consume("]")) {
    sz = const_expr();
    is_incomplete = false;
    expect("]");
  }
  Token *tok = token;
  ty = type_suffix(ty);
  if (ty->is_incomplete)
    error_tok(tok, "incomplete lement type");

  ty = array_of(ty, sz);
  ty->is_incomplete = is_incomplete;
  return ty;
}

static void push_tag_scope(Token *tok, Type *ty) {
  TagScope *sc = calloc(1, sizeof(TagScope));
  sc->next = tag_scope;
  sc->name = strndup(tok->str, tok->len);
  sc->ty = ty;
  tag_scope = sc;
}

// struct-member = basetype ident ("[" num "]")* ";"
static Member *struct_member() {
  Token *tok = token;
  Member *mem = calloc(1, sizeof(Member));
  mem->ty = basetype();
  mem->name = expect_ident();
  mem->ty = type_suffix(mem->ty);
  mem->tok = tok;
  expect(";");
  return mem;
}

static VarList *read_func_param() {
  Type *ty = basetype();
  
  char *name = expect_ident();
  ty = type_suffix(ty);
  
  VarList *vl = calloc(1, sizeof(VarList));
  vl->var = new_lvar(name, ty);
  return vl;
}

static VarList *read_func_params() {
  if (consume(")"))
    return NULL;

  VarList *head = read_func_param();
  VarList *cur = head;

  while (!consume(")")) {
    expect(",");
    cur->next = read_func_param();
    cur = cur->next;
  }

  return head;
}

// function = ident "(" params? ")" "{" stmt* "}"
// params = ident ("," ident)*
static Function *function() {
  locals = NULL;

  Function *fn = calloc(1,sizeof(Function));
  basetype();
  fn->name = expect_ident();
  expect("(");
  Scope *sc = enter_scope();
  fn->params = read_func_params();
  expect("{");
  
  Node head = {};
  Node *cur = &head;
  while(!consume("}")){
    cur->next = stmt();
    cur = cur->next;
  }
  leave_scope(sc);
  
  fn->node = head.next;
  fn->locals = locals;
  return fn;
}

// gvar-initializer2 = assign
static Initializer *new_init_val(Initializer *cur, int sz, int val) {
  Initializer *init = calloc(1, sizeof(Initializer));
  init->sz = sz;
  init->val = val;
  cur->next = init;
  return init;
}

static Initializer *new_init_label(Initializer *cur, char *label) {
  Initializer *init = calloc(1, sizeof(Initializer));
  init->label = label;
  cur->next= init;
  return init;
}

static Initializer *gvar_init_string(char *p, int len) {
  Initializer head = {};
  Initializer *cur = &head;
  for (int i = 0; i < len; i++)
    cur = new_init_val(cur, 1, p[i]);
  return head.next;
}

// gvar-initializer2 = assign
static Initializer *gvar_initializer2(Initializer *cur, Type *ty) {
  Token *tok = token;
  Node *expr = assign();

  if (expr->kind == ND_ADDR) {
    if (expr->lhs->kind != ND_VAR)
      error_tok(tok, "invalid initializer");
    return new_init_label(cur, expr->lhs->var->name);
  }

  if (expr->kind == ND_VAR && expr->var->ty->kind == TY_ARRAY)
    return new_init_label(cur ,expr->var->name);

  return new_init_val(cur, ty->size, eval(expr));
}

static Initializer *gvar_initializer(Type *ty) {
  Initializer head = {};
  gvar_initializer2(&head, ty);
  return head.next;
}

// global-var = basetype declarator type-suffix ("=" gvar-initializer)? ";"
static void global_var() {
  Type *ty = basetype();
  char *name = expect_ident();
  Token *tok = token;
  ty = type_suffix(ty);

  Var *var = new_gvar(name, ty);

  if (!consume("=")) {
    if (ty->is_incomplete)
      error_tok(tok, "incomplete type");
    expect(";");
    return;
  }

  var->initializer = gvar_initializer(ty);
  expect(";");
}

// Some types of list can end with an optional "," followed by "}"
// to allow a trailing comma. This function returns true if it looks
// like we are at the end of such list.
static bool consume_end(void) {
  Token *tok = token;
  if (consume("}") || (consume(",") && consume("}")))
    return true;
  token = tok;
  return false;
}

static bool peek_end(void) {
  Token *tok = token;
  bool ret = consume("}") || (consume(",") && consume("}"));
  token = tok;
  return ret;
}

static void expect_end(void) {
  if (!consume_end())
    expect("}");
}

typedef struct Designator Designator;
struct Designator {
   Designator *next;
   int idx; // array
   Member *mem; // struct
};

// Create a node for an array access. For example, if var represents
// a varible x and desg represents indices 3 and 4, this function
// returns a node representing x[3][4]
static Node *new_desg_node2(Var *var, Designator *desg, Token *tok) {
  if (!desg)
    return new_var_node(var, tok);

  Node *node = new_desg_node2(var, desg->next, tok);
  if (desg->mem){
    node = new_unary(ND_MEMBER, node, desg->mem->tok);
    node->member = desg->mem;
    return node;
  }
  
  node = new_add(node, new_num(desg->idx, tok), tok);
  return new_unary(ND_DEREF, node, tok);
}

static Node *new_desg_node(Var *var, Designator *desg, Node *rhs) {
    Node *lhs = new_desg_node2(var, desg, rhs->tok);
    Node *node = new_binary(ND_ASSIGN, lhs, rhs, rhs->tok);
    return new_unary(ND_EXPR_STMT, node, rhs->tok);
}

static Node *lvar_init_zero(Node *cur, Var *var, Type *ty, Designator *desg) {
  if (ty->kind == TY_ARRAY) {
    for (int i = 0; i < ty->array_len; i++) {
      Designator desg2 = {desg, i++};
      cur = lvar_init_zero(cur,var,ty->base,&desg2);
    }
    return cur;
  }

  cur->next = new_desg_node(var,desg, new_num(0, token));
  return cur->next;
}

// lvar-initializer2 = assign
//                   | "{" lvar-initializer2 ("," lvar-initializer2)* ","? "}"
//
// An initializer for a local variable is expanded to multiple
// assignments. For example, this function creates the following
// nodes for x[2][3] = {{1,2,3},{4,5,6}}.
//
//   x[0][0]=1;
//   x[0][1]=2;
//   x[0][2]=3;
//   x[1][0]=4;
//   x[1][1]=5;
//   x[1][2]=6;
//
//  Struct members are initialized in declaration order. For example,
//  `struct { int a; int b; } x = {1 , 2}` sets x.a to 1 and x.b to 2.
//
//  There are a few pecial rules for ambiguas initializers are
//  shorthand notations:
//
//  - If an initializer list is shorter than an array, excess array 
//    elements are initialized with 0.
//
//  - A char array can be initialized by a string literal. For example,
//    `char x[4] = "foo"` is equivalent to `char x[4] = {'f','o','o',
//    '\0'}`.
//
//  - If lhs is an imcomplete array. its size is set to the number of
//    items on the rhs. For example, `x` in `int x[]={1, 2, 3}` will have
//    type `int[3]` because the rhs initializer has three items.
static Node *lvar_initializer2(Node *cur, Var *var, Type *ty, Designator *desg) {
  if (ty->kind == TY_ARRAY && ty->base->kind == TY_CHAR &&
    token->kind == TK_STR) {
    // Initialize a char array with a string literal.
    Token *tok = token;
    token = token->next;

    if (ty->is_incomplete) {
      ty->size = tok->cont_len;
      ty->array_len = tok->cont_len;
      ty->is_incomplete = false;
    }

    int len = (ty->array_len < tok->cont_len)
      ? ty->array_len : tok->cont_len;

    for (int i = 0; i < len; i++) {
      Designator desg2 = {desg, i};
      Node *rhs = new_num(tok->contents[i], tok);
      cur->next = new_desg_node(var, &desg2, rhs);
      cur = cur->next;
    }

    for (int i = len; i < ty->array_len; i++) {
      Designator desg2 = {desg, i};
      cur = lvar_init_zero(cur, var, ty->base, &desg2);
    }
    return cur;
  }
  
  if (ty->kind == TY_ARRAY) {
    expect("{");
    int i = 0;

    if (!peek("}")) {
      do {
        Designator desg2 = {desg, i++};
        cur = lvar_initializer2(cur, var, ty->base, &desg2);
      } while (!peek_end() && consume(","));
    }
    expect_end();
    
    // Set excess array elements to zero.
    while (i < ty->array_len) {
      Designator desg2 = {desg, i++};
      cur = lvar_init_zero(cur, var, ty->base, &desg2);
    }

    if (ty->is_incomplete) {
      ty->size = ty->base->size * i;
      ty->array_len = i;
      ty->is_incomplete = false;
    }
    return cur;
  }
  if (ty->kind == TY_STRUCT) {
    expect("{");
    Member *mem = ty->members;

    if (!peek("}")) {
      do {
        Designator desg2 = {desg, 0, mem};
        cur = lvar_initializer2(cur, var, mem->ty, &desg2);
        mem = mem->next;
      } while(!peek_end() && consume(","));
    }
    expect_end();

    // Set excess struct elements to zero.
    for (; mem; mem = mem->next) {
      Designator desg2 ={desg, 0, mem};
      cur = lvar_init_zero(cur ,var, mem->ty, &desg2);
    }
    return cur;
  }
  cur->next = new_desg_node(var, desg, assign());
  return cur->next;
}

static Node *lvar_initializer(Var *var, Token *tok) {
  Node head = {};
  lvar_initializer2(&head, var, var->ty, NULL);

  Node *node = new_node(ND_BLOCK,tok);
  node->body = head.next;
  return node;
}

// declaration = basetype declarator type-suffix ("=" lvar-initializer)? ";"
//             | basetype ;
static Node *declaration(){
  Token *tok = token;
  Type *ty = basetype();
  if (consume(";"))
    return new_node(ND_NULL, tok);

  char *name = expect_ident();
  ty = type_suffix(ty);

  if (ty->kind == TY_VOID)
    error_tok(tok, "variable declared void");

  Var *var = new_lvar(name, ty);
  if (consume(";")) {
    if (ty->is_incomplete)
      error_tok(tok, "incomplete type");
    return new_node(ND_NULL,tok);
  }

  expect("=");
  Node *node = lvar_initializer(var, tok);
  expect(";");
  return node;
}

static Node *read_expr_stmt(){
  Token *tok = token;
  return new_unary(ND_EXPR_STMT, expr(), tok);
}

static bool is_typename(void) {
  return peek("void") || peek("char") || peek("int") || peek("struct");
}

// struct-decl = "struct" ident
//             | "struct" ident? "{" struct-member "}"
static Type *struct_decl() {
  // Read struct members.
  expect("struct");

  // Read a struct tag.
  Token *tag = consume_dent();
  if (tag && !peek("{")) {
    TagScope *sc = find_tag(tag);
    if(!sc)
      error_tok(tag, "unknown struct type");
    return sc->ty;
  }

  expect("{");

  // Read struct members.
  Member head = {};
  Member *cur = &head;

  while (!consume("}")) {
    cur->next = struct_member();
    cur = cur->next;
  }

  Type *ty = calloc(1, sizeof(Type));
  ty->kind = TY_STRUCT;
  ty->members = head.next;

  // Assign offsets within the struc to members.
  int offset = 0;
  for (Member *mem = ty->members; mem; mem = mem->next) {
    if (mem->ty->is_incomplete)
      error_tok(mem->tok, "incomplete struct member");

    mem->offset = offset;
    offset += mem->ty->size;
  }
  ty->size = offset;
  
  // Register the struct type if a name was given.
  if (tag)
    push_tag_scope(tag, ty);
  return ty;
}

static Member *find_member(Type *ty, char *name) {
  for (Member *mem = ty->members; mem; mem = mem->next)
    if(!strcmp(mem->name, name))
      return mem;
    return NULL;
}

static Node *struct_ref(Node *lhs) {
  add_type(lhs);
  if (lhs->ty->kind != TY_STRUCT)
    error_tok(lhs->tok, "not a struct");

  Token *tok = token;
  Member *mem = find_member(lhs->ty, expect_ident());
  if (!mem)
    error_tok(tok, "no such member");
  
  Node *node = new_unary(ND_MEMBER, lhs, tok);
  node->member = mem;
  return node;
}

static Node *assign() {
    Node *node = conditional();
    Token *tok;
    if (tok = consume("="))
      node = new_binary(ND_ASSIGN, node, assign(), tok);
    return node;
}

// bitor = bitxor ("|" bitxor)*
static Node *bitor() {
  Node *node = bitxor();
  Token *tok;
  while (tok = consume("|"))
    node = new_binary(ND_BITOR, node, bitxor(), tok);
  return node;
}

// bitxor = bitand ("^" bitand)*
static Node *bitxor() {
  Node *node = bitand();
  Token *tok;
  while (tok = consume("^"))
    node = new_binary(ND_BITXOR, node, bitand(), tok);
  return node;
}

// bitxor = equality ("&" equality)*
static Node *bitand() {
  Node *node = equality();
  Token *tok;
  while (tok = consume("&"))
    node = new_binary(ND_BITAND, node, equality(), tok);
  return node;
}

// expr = assign ("," assign)*
static Node *expr() {
    Node *node = assign();
    Token *tok;
    while (tok = consume(",")) {
      node = new_unary(ND_EXPR_STMT, node, node->tok);
      node = new_binary(ND_COMMA, node, assign(), tok);
    }
    return node;
}

static Node *stmt() {
  Node *node = stmt2();
  add_type(node);
  return node;
}

// stmt2 = "return" expr ";"
//       | "if" "(" expr ")" stmt ("else" stmt)?
//       | "while" "(" expr ")" stmt
//       | "for" "(" expr? ";" expr? ";" expr? ")" stmt  
//       | "{" stmt* "}"
//       | expr ";"
static Node *stmt2() {
  Token *tok;
  if (tok = consume("return")) {
    Node *node = new_unary(ND_RETURN, expr(), tok);
    expect(";");
    return node;
  }

  if (tok = consume("if")) {
    Node *node = new_node(ND_IF,tok);
    expect("(");
    node->cond = expr();
    expect(")");
    node->then = stmt();
    if(consume("else"))
      node->els = stmt();
    return node;
  }

  if(tok = consume("while")) {
    Node *node = new_node(ND_WHILE, tok);
    expect("(");
    node->cond = expr();
    expect(")");
    node->then = stmt();
    return node;
  }

  if (tok = consume("for")) {
    Node *node = new_node(ND_FOR, tok);
    expect("(");
    if (!consume(";")) {
      node->init = read_expr_stmt();
      expect(";");
    }
    if (!consume(";")) {
      node->cond = expr();
      expect(";");
    }
    if (!consume(")")) {
      node->inc = read_expr_stmt();
      expect(")");
    }
    node->then = stmt();
    return node;
  }

  if (tok = consume("{")) {
    Node head = {};
    Node *cur = &head;

    while(!consume("}")){
      cur->next = stmt();
      cur = cur->next;
    }

    Node *node = new_node(ND_BLOCK, tok);
    node->body = head.next;
    return node;
  }

  if (is_typename())
    return declaration();
  
  Node *node = read_expr_stmt();
  expect(";");
  return node;
}

// equality = relational ("==" relational | "!=" relational)*
static Node *equality() {
    Node *node = relational();
    Token *tok;
    
    for (;;) {
      if (tok = consume("=="))
	      node = new_binary(ND_EQ, node, relational(), tok);
      else if (tok = consume("!="))
	      node = new_binary(ND_NE, node, relational(), tok);
      else
        return node;
    }
}

// relational = add ("<" add | "<=" add | ">" add | ">=" add)*
static Node *relational() {
    // lhs
    Node *node = add();
    Token *tok;
    
    for (;;) {
      if (tok = consume("<"))
        node = new_binary(ND_LT, node, add(), tok);
      else if (tok = consume("<="))
        node = new_binary(ND_LE, node, add(), tok);
      else if (tok = consume(">"))
        node = new_binary(ND_LT, add(), node, tok);
      else if (tok = consume(">="))
        node = new_binary(ND_LE, add(), node, tok);
      else
        return node;
    }
}

static Node *conditional() {
  Node *node = bitor();
  Token *tok = consume("?");
  if (!tok)
    return node;

  Node *ternary = new_node(ND_TERNARY, tok);
  ternary->cond = node;
  ternary->then = expr();
  expect(":");
  ternary->els = conditional();
  return ternary;
}

static Node *new_add(Node *lhs, Node *rhs, Token *tok) {
  add_type(lhs);
  add_type(rhs);

  if (is_integer(lhs->ty) && is_integer(rhs->ty))
    return new_binary(ND_ADD, lhs, rhs, tok);
  if (lhs->ty->base && is_integer(rhs->ty))
    return new_binary(ND_PTR_ADD, lhs, rhs, tok);
  if (is_integer(lhs->ty) && rhs->ty->base)
    return new_binary(ND_PTR_ADD, rhs, lhs, tok);
  error_tok(tok, "invalid operands");
}

static Node *new_sub(Node *lhs, Node *rhs, Token *tok) {
  add_type(lhs);
  add_type(rhs);

  if (is_integer(lhs->ty) && is_integer(rhs->ty))
    return new_binary(ND_SUB, lhs, rhs, tok);
  if (lhs->ty->base && is_integer(rhs->ty))
    return new_binary(ND_PTR_SUB, lhs, rhs, tok);
  if (lhs->ty->base && rhs->ty->base)
    return new_binary(ND_PTR_DIFF, lhs, rhs,tok);
  error_tok(tok,"invalid operands");
}

// add = mul ("+" mul | "-" mul)*
static Node *add() {
    // lhs
    Node *node = mul();
    Token *tok;
    for (;;) {
      if (tok = consume("+"))
    	  node = new_add(node, mul(), tok);
      else if (tok = consume("-"))
    	  node = new_sub(node, mul(), tok);
      else
        return node;
    }
}

// mul = unary ("*" unary | "/" unary)*
static Node *mul() {
    Node *node = unary();
    Token *tok;
    for (;;) {
      if (tok = consume("*"))
    	  node = new_binary(ND_MUL, node, unary(), tok);
      else if (tok = consume("/"))
	      node = new_binary(ND_DIV, node, unary(), tok);
      else
        return node;
        
    }
}

// postfix = primary ("[" expr "]" | "." ident)*
static Node *postfix() {
  Node * node = primary();
  Token *tok;

  for (;;) {
    if (tok = consume("[")) {
      // x[y] is short for *(x + y)
      // x+y
      Node *exp = new_add(node, expr(), tok);
      expect("]");
      // *(x+y)
      node = new_unary(ND_DEREF, exp, tok);
      continue;
    }

    if (tok = consume(".")) {
      node = struct_ref(node);
      continue;
    }

    if (tok = consume("->")) {
      // x->y is short for (*x).y
      node = new_unary(ND_DEREF, node, tok);
      node = struct_ref(node);
      continue;
    }

    return node;
  }

}

// unary = ("+" | "-" | "*" | "&")? unary
//       | postfix
static Node *unary() {
  Token *tok;
  
  if (consume("+")) {
    return unary();
  }
  if (tok = consume("-")) {
    // -x => 0-x
    return new_binary(ND_SUB, new_num(0, tok), unary(), tok);
  }
  if (tok = consume("&")) {
    return new_unary(ND_ADDR, unary(), tok);
  }
  if (tok = consume("*")) {
    return new_unary(ND_DEREF, unary(), tok);
  }
  return postfix();
}

// func-args = "(" (assign ("," assign)*)? ")"
static Node *func_args() {
  if (consume(")"))
    return NULL;

  Node *head = assign();
  Node *cur = head;
  while (consume(",")) {
    cur->next = assign();
    cur = cur->next;
  }
  expect(")");
  return head;
}

// primary =  "(" expr ")" | "sizeof" unary | ident func-args? | str | num
// args = "(" ident ("," ident)* ")"
static Node *primary() {
  if (consume("(")) {
    Node *node = expr();
    expect(")");
    return node;
  }

  Token *tok;
  if (tok = consume("sizeof")){
    Node *node = unary();
    add_type(node);
    if (node->ty->is_incomplete)
      error_tok(node->tok, "incomplete type");
    return new_num(node->ty->size,tok);
    //if(node->ty->kind == TY_INT) {
    //  return new_num(4,tok);
    //} else if (node->ty->kind == TY_PTR) {
    //  return new_num(8,tok);
    //}
  }
  
  if (tok = consume_ident()) {
    
    if (consume("(")) {
      // Function call
      // ident(x,y)
      Node *node = new_node(ND_FUNCALL, tok);
      node->funcname = strndup(tok->str, tok->len);
      node->args = func_args();
      return node;
    }

    // Variable
    // indent without (
    Var *var = find_var(tok);
    if(!var)
      error_tok(tok, "undefined variable");
    return new_var_node(var, tok);
  }

  tok = token;
  if (tok->kind == TK_STR) {
    token = token->next;

    Type *ty = array_of(char_type, tok->cont_len);
    Var *var = new_gvar(new_label(), ty);
    var->initializer = gvar_init_string(tok->contents, tok->cont_len);
    return new_var_node(var, tok);
  }


  if (tok->kind != TK_NUM)
    error_tok(tok, "expected expression");
  return new_num(expect_number(), tok);
}

// Evaluate a given node as a content expression.
static long eval(Node *node) {
  switch (node->kind){
  case ND_ADD:
    return eval(node->lhs) + eval(node->rhs);
  case ND_SUB:
    return eval(node->lhs) - eval(node->rhs);
  case ND_MUL:
    return eval(node->lhs) * eval(node->rhs);
  case ND_DIV:
    return eval(node->lhs) / eval(node->rhs);
  case ND_BITAND:
    return eval(node->lhs) & eval(node->rhs);
  case ND_BITOR:
    return eval(node->lhs) | eval(node->rhs);
  case ND_BITXOR:
    return eval(node->lhs) | eval(node->rhs);
  case ND_SHL:
    return eval(node->lhs) << eval(node->rhs);
  case ND_SHR:
    return eval(node->lhs) >> eval(node->rhs);
  case ND_EQ:
    return eval(node->lhs) == eval(node->rhs);
  case ND_NE:
    return eval(node->lhs) != eval(node->rhs);
  case ND_LT:
    return eval(node->lhs) < eval(node->rhs);
  case ND_LE:
    return eval(node->lhs) <= eval(node->rhs);
  case ND_TERNARY:
    return eval(node->cond) ? eval(node->then) : eval(node->els);
  case ND_COMMA:
    return eval(node->lhs);
  case ND_NOT:
    return !eval(node->lhs);
  case ND_LOGAND:
    return eval(node->lhs) && eval(node->rhs);
  case ND_LOGOR:
    return eval(node->lhs) || eval(node->rhs);
  case ND_NUM:
    return node->val;
  }

  error_tok(node->tok, "not a constant expression");
}

static long const_expr() {
  return eval(conditional());
}