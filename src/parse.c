#include "kmcc.h"

// All local variable instances created during parsing are
// accumulated to this list.
static VarList *locals;
// All global variable instances
static VarList *globals;

// Find a variable by name
static Var *find_var(Token *tok) {
  // First look up a variable instance in locals
  for (VarList *vl = locals; vl; vl = vl->next) {
    Var *var = vl->var;
    if (strlen(var->name) == tok->len && !strncmp(tok->str, var->name, tok->len))
      return var;
  }
  // and then look up a variable instance in globals
  for (VarList *vl = globals; vl; vl = vl->next) {
    Var *var = vl->var;
    if (strlen(var->name) == tok->len && !strncmp(tok->str, var->name, tok->len))
      return var;
  }
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
static Node *new_add(Node *lhs, Node *rhs, Token *tok);
static Node *add();
static Node *assign();
static Node *equality();
static Node *relational();
static Node *primary();
static Node *mul();
static Node *unary();      
static void global_var();
static Type *struct_decl();
static Member *struct_member();
static bool is_typename();

// basetype = ("char" | "int" | struct-decl) "*"*
static Type *basetype(void) {
  if (!is_typename())
    error_tok(token, "typename expected");
  Type *ty;
  if (consume("char"))
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

static Type *read_type_suffix(Type *base) {
  if (!consume("["))
    return base;
  int sz = expect_number();
  expect("]");
  base = read_type_suffix(base);
  return array_of(base, sz);
}

// struct-member = basetype ident ("[" num "]")* ";"
static Member *struct_member() {
  Member *mem = calloc(1, sizeof(Member));
  mem->ty = basetype();
  mem->name = expect_ident();
  mem->ty = read_type_suffix(mem->ty);
  expect(";");
  return mem;
}

static VarList *read_func_param() {
  Type *ty = basetype();
  
  char *name = expect_ident();
  ty = read_type_suffix(ty);
  
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
  fn->params = read_func_params();
  expect("{");
  
  Node head = {};
  Node *cur = &head;
  while(!consume("}")){
    cur->next = stmt();
    cur = cur->next;
  }

  fn->node = head.next;
  fn->locals = locals;
  return fn;
}

static void global_var() {
  Type *ty = basetype();
  char *name = expect_ident();
  Token *tok = token;
  ty = read_type_suffix(ty);
  expect(";");

  if (ty->is_complete)
    error_tok(tok, "incomplete type");
  new_gvar(name, ty);
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
   int idx;
};

// Create a node for an array access. For example, if var represents
// a varible x and desg represents indices 3 and 4, this function
// returns a node representing x[3][4]
static Node *new_desg_node2(Var *var, Designator *desg, Token *tok) {
  if (!desg)
    return new_var_node(var, tok);

  Node *node = new_desg_node2(var, desg->next, tok);
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

    if (ty->is_complete) {
      ty->size = tok->cont_len;
      ty->array_len = tok->cont_len;
      ty->is_complete = false;
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

    if (ty->is_complete) {
      ty->size = ty->base->size * i;
      ty->array_len = i;
      ty->is_complete = false;
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
  char *name = expect_ident();
  ty = read_type_suffix(ty);
  Var *var = new_lvar(name, ty);
  
  if (consume(";"))
    return new_node(ND_NULL, tok);

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
  return peek("char") || peek("int") || peek("struct");
}

// struct-decl = "struct" "{" struct-member "}"
static Type *struct_decl() {
  // Read struct members.
  expect("struct");
  expect("{");

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
    mem->offset = offset;
    offset += mem->ty->size;
  }
  ty->size = offset;

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
    Node *node = equality();
    Token *tok;
    if (tok = consume("="))
      node = new_binary(ND_ASSIGN, node, assign(), tok);
    return node;
}

// expr = assign
static Node *expr() {
    return assign();
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
    var->contents = tok->contents;
    var->cont_len = tok->cont_len;
    return new_var_node(var, tok);
  }


  if (tok->kind != TK_NUM)
    error_tok(tok, "expected expression");
  return new_num(expect_number(), tok);
}
