#include "kmcc.h"
Var *locals;

static Type *basetype(void) {
  expect("int");
  Type *ty = int_type;
  while (consume("*"))
    ty = pointer_to(ty);
  return ty;
}

static Var *find_var(Token *tok) {
  for (Var *var = locals; var; var = var->next)
    if (tok->len == tok->len && !memcmp(tok->str, var->name, tok->len))
     return var;
  return NULL;
}

Node *new_node(NodeKind kind) {
    Node *node = calloc(1,sizeof(Node));
    node->kind = kind;
    return node;
}

Node *new_binary(NodeKind kind, Node *lhs, Node *rhs) {
    Node *node = new_node(kind);
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

Node *new_node_num(long val) {
    Node *node = new_node(ND_NUM);
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

static Node *new_var_node(Var *var) {
  Node *node = new_node(ND_VAR);
  node->var = var;
  return node;
}

static Var *new_lvar(char *name){
  Var *var = calloc(1,sizeof(Var));
  var->next = locals;
  var->name = name;
  locals = var;
  return var;
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
    if (consume("="))
        node = new_binary(ND_ASSIGN, node, assign());
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

/*
static Node *function() {
    Node *node;
    expect("int");
    while (consume("*")) {
        // do something
        //if (!consume("*"))
        //    error_at(token->str, "'*'ではないではないトークンです");
    }
    Token *ident = consume_ident();
    if (ident == NULL)
        error_at(token->str, "関数名ではないではないトークンです");
    expect("(");
    Vector *args = new_vector();
    while (!consume(")")) {
        expect("int");
        Type *ty = malloc(sizeof(Type));
        ty->ty = INT;
        Token *var = consume_ident();
        if (var != NULL) {
            var_append(variables, ty, strndup(var->str,var->len));
            vec_push(args, (void *) strndup(var->str,var->len));
        } else {
            while (!consume_ident()) {
 	        expect("*");
                Type *next = ty;
                ty = malloc(sizeof(Type));
                ty->ty = PTR;
                ty->ptr_to = next;
            }
        }
        consume(",");
    }
    expect("{");
    Vector *stmts = new_vector();
    while(!consume("}")) {
        vec_push(stmts, stmt());
    }
    Node *block = new_node_block(stmts);
    return new_node_function(strndup(ident->str,ident->len), args, block);
    }*/

static Node stmt() {
  Node *node = stmt2();
  add_type(node);
  return node;
}

static Node *stmt2() {
    Node *node;

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
    } else {
        node = expr();
    }
    expect(";");
    return node;
}

Function *program() {
  Function head = {};
  Function *cur = &head;

  while (!at_eof()) {
    cur->next = function();
    cur = cur->next;
  }
  return head.next;
}

static Node *equality() {
    Node *node = relational();
    
    for (;;) {
        if (consume("=="))
	  node = new_binary(ND_EQ, node, relational());
        else if (consume("!="))
	  node = new_binary(ND_NE, node, relational());
        else
          return node;
    }
}

static Node *relational() {
    // lhs
    Node *node = add();
    
    for (;;) {
        if (consume("<"))
            node = new_binary(ND_LT, node, add());
        else if (consume("<="))
            node = new_binary(ND_LE, node, add());
        else if (consume(">"))
            node = new_binary(ND_LT, add(), node);
        else if (consume(">="))
            node = new_binary(ND_LE, add(), node);
        else
            return node;
    }
}

static Node *add() {
    // lhs
    Node *node = mul();
    
    for (;;) {
        if (consume("+"))
            node = new_binary(ND_ADD, node, mul());
        else if (consume("-"))
            node = new_binary(ND_SUB, node, mul());
        else
            return node;
    }
}

static Node *mul() {
    Node *node = unary();
    
    for (;;) {
        if (consume("*"))
            node = new_binary(ND_MUL, node, unary());
        else if (consume("/"))
            node = new_binary(ND_DIV, node, unary());
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
        return new_node_num(expect_number());
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
    if (consume("+")) {
        return unary();
    }
    if (consume("-")) {
        // -x => 0-x
        return new_binary(ND_SUB,new_node_num(0), unary());
    }
    if (consume("*")) {
        return new_binary(ND_DEREF, unary(), NULL);
    }
    if (consume("&")) {
        return new_binary(ND_ADDR, unary(), NULL);
    }
    return primary();
}
