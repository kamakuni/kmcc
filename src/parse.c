#include "kmcc.h"

// All local variable instances created during parsing are
// accumulated to this list.
static VarList *locals;

static Type *basetype(void) {
  expect("int");
  Type *ty = int_type;
  while (consume("*"))
    ty = pointer_to(ty);
  return ty;
}

// Find a local variable by name
static Var *find_var(Token *tok) {
  for (VarList *vl = locals; vl; vl = vl->next) {
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

Node *new_node_block(Vector *stmts) {
    Node *node = new_node(ND_BLOCK);
    node->stmts = stmts;
    return node;
}

Node *new_node_call(char *name, Node *args) {
    Node *node = new_node(ND_CALL);
    node->name = name;
    node->args = args;
    return node;
}

Node *new_node_function(char *name, Vector *args, Node *block) {
    Node *node = new_node(ND_FUNC);
    node->name = name;
    node->args = args;
    node->block = block;
    return node;
}

Node *new_num(long val, Token *tok) {
  Node *node = new_node(ND_NUM, tok);
    node->val = val;
    return node;
}

Node *new_node_ident(Type *ty, char *name) {
    Node *node = new_node(ND_IDENT);
    node->ty = ty;
    node->name = strndup(name, strlen(name));
    var_append(variables, ty, name);
    return node;
}

Node *new_node_for(Node *init, Node *cond, Node * inc, Node *body) {
    Node *node = new_node(ND_FOR);
    node->init = init;
    node->cond = cond;
    node->inc = inc;
    node->body = body;
    return node;
}

Node *new_node_if(Node *cond, Node *body, Node *els) {
    Node *node = new_node(ND_IF);
    node->cond = cond;
    node->body = body;
    node->els = els;
    return node;
}

Node *new_node_while(Node *cond, Node *body) {
    Node *node = new_node(ND_WHILE);
    node->cond = cond;
    node->body = body;
    return node;
}

static Node *new_var_node(Var *var, Token *tok) {
  Node *node = new_node(ND_VAR, tok);
  node->var = var;
  return node;
}

static Var *new_lvar(char *name){
  Var *var = calloc(1,sizeof(Var));
  var->name = name;

  VarList *vl = calloc(1 sizeof(VarList));
  vl->var = var;
  vl->next = locals;
  locals = vl;
  return var;
}

static Node *new_unary(NodeKind kind, Node *expr, Token *tok){
  Node *node = new_node(kind, tok);
  node->lhs = expr;
  return node;
}

Node *code[100];

static Function *function();
static Node *stmt();
static Node *add();
static Node *equality();
static Node *relational();
static Node *primary();
static Node *mul();
static Node *unary();      

static Node *assign() {
    Node *node = equality();
    Token *tok;
    if (tok = consume("="))
      node = new_binary(ND_ASSIGN, node, assign(), tok);
    return node;
}

static Node *expr() {
    return assign();
}

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

static Function *function() {
  locals = NULL;

  expect("int");
  while (consume("*")) {
    // do something
    //if (!consume("*"))
    //    error_at(token->str, "'*'ではないではないトークンです");
  }    
  
  char *name = expect_ident();
  expect("(");
  expect(")");
  expect("{");

  Node head = {};
  Node *cur = &head;

  while(!consume("}")){
    cur->next = stmt();
    cur = cur->next;
  }

  Function *fn = calloc(1, sizeof(Function));
  fn->name = name;
  fn->node = head.next;
  fn->locals = locals;
  return fn;
}

static Node *read_expr_stmt(){
  Token *tok = token;
  return new_unary(ND_EXPR_STMT, expr(), tok);
}

static Node *stmt2();

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
    Node *node = ew_uary(ND_RETURN, expr(), tok);
    expect(";");
    return node;
  }

  if (tok = consume("if")) {
    Node *node = new_node(ND_IF,tok);
    expect("(");
    node->code = expr();
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

  Node *node = read_expr_stmt();
  expect(";");
  return node;
  /*  
    if (consume("int")) {
        Type *ty = malloc(sizeof(Type));
        ty->ty = INT;
        while (token->kind != TK_IDENT) {
            if (!consume("*"))
                error_at(token->str, "'*'ではないではないトークンです");
            Type *next = ty;
            ty = malloc(sizeof(Type));
            ty->ty = PTR;
            ty->ptr_to = next;
        }
        Token *ident = consume_ident();
        if (ident != NULL) {
	  Var *var = new_lvar(strndup(ident->str, ident->len));
	  node = new_var_node(var);
	  expect(";");
          return node;
        } else {
            error_at(token->str, "識別子ではないトークンです");
        }
    }
    if (consume("{")) {
        Vector *stmts = new_vector();
        while(!consume("}")) {
            vec_push(stmts, stmt());
        }
        return new_node_block(stmts);
    }
    if (consume("if")) {
	expect("(");
	Node *cond = expr();
	expect(")");
        Node *body = stmt();
        if(consume("else")) {
            Node *els = stmt();
            return new_node_if(cond, body, els);
        }
        return new_node_if(cond, body, NULL);
    }
    if (consume("while")) {
	expect("(");
	Node *cond = expr();
	expect(")");
        Node *body = stmt();
        return new_node_while(cond, body);
    }
    if (consume("for")) {
	expect("(");
        Node *init = NULL;
        if(!consume(";")) {
            init = expr();
	    expect(";");
        }
        Node *cond = NULL;
        if(!consume(";")) {
            cond = expr();
            expect(";");
        }
        Node *inc = NULL;
        if(!consume(")")) {
            inc = expr();
	    expect(")");
        }
        Node *body = stmt();
        return new_node_for(init, cond, inc, body);
    }
    if (consume("return")) {
        node = new_binary(ND_RETURN, expr(), NULL);
    }
    node = read_expr_stmt();
    expect(";");
    return node;
  */
}

// program = function*
Function *program() {
  Function head = {};
  Function *cur = &head;

  while (!at_eof()) {
    cur->next = function();
    cur = cur->next;
  }
  return head.next;
}

static VarList *read_func_params() {
  if (consume(")"))
    return NULL;

  VarList *head = calloc(1, sizeof(VarList));
  head->var = new_lvar(expect_ident());
  VarList *cur = head;

  while (!consume(")")) {
    expect(",");
    cur->next = calloc(1, sizeof(VarList));
    cur->next->var = new_lvar(expect_ident());
    cur = cur->next;
  }

  return head;
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

  if (is_integr(lhs->ty) && is_integer(rhs->ty))
    return new_binary(ND_SUB, lhs, rhs, tok);
  if (lhs->ty->base && is_integer(rhs->ty))
    return new_binary(ND_PTR_SUB, lhs, rhs, tok);
  if (lhs->ty->base && rhs->ty->base)
    return new_binary(ND_PTR_DIFF, lhs, rhs,tok);
  error_tok(tok,"invalid operands");
}

static Node *add() {
    // lhs
    Node *node = mul();
    Token *tok;
    for (;;) {
        if (tok = consume("+"))
	  node = new_binary(ND_ADD, node, mul(), tok);
        else if (tok = consume("-"))
	  node = new_binary(ND_SUB, node, mul(), tok);
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

static Node *primary() {
    if (consume("(")) {
        Node *node = equality();
	expect(")");
        return node;
    }
    
    if (token->kind == TK_NUM ) {
        return new_num(expect_number());
    }
    Token *ident = consume_ident();
    if (ident != NULL) {
        char *name = strndup(ident->str,ident->len);
        if (!consume("(")) {
	  Var *var = find_var(ident);
	  if(!var)
	    var = new_lvar(strndup(ident->str, ident->len));
	  return new_var_node(var);
        }
	/* Vector *args = new_vector();
        while (!consume(")")) {
            Node *node = add();
            vec_push(args, (void *) node);
            consume(",");
	    }*/
	Node *args = func_args();
        return new_node_call(name, args);
    }
    
    error("数値でも開き括弧でもないトークンです： %s ", token->str);
    return NULL;
}

static Node *unary() {
  Token *tok;
  if (consume("+")) {
    return unary();
  }
  if (tok = consume("-")) {
    // -x => 0-x
    return new_binary(ND_SUB,new_num(0, tok), unary() tok);
  }
  if (tok = consume("*")) {
    return new_unary(ND_DEREF, unary(), tok);
  }
  if (tok = consume("&")) {
    return new_unary(ND_ADDR, unary(), tok);
  }
  return primary();
}
