#include "kmcc.h"

char *filename;
char *user_input;

// Report an error and exit.
void error(char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, "\n");
    exit(1);
}

// Report an error message in the following format and exit.
//
// foo.c:10: x = y + 1;
//               ^ <error message here>
void verror_at(char *loc, char *fmt, va_list ap) {
    // Find a line containing `loc`.
    char *line = loc;
    while (user_input < line && line[-1] != '\n')
        line--;
    
    char *end = loc;
    while (*end != '\n')
        end++;
    
    // Get a line number.
    int line_num = 1;
    for (char *p = user_input; p < line; p++)
        if (*p == '\n')
            line_num++;
    
    // Print out the line.
    int ident = fprintf(stderr, "%s:%d: ", filename, line_num);
    fprintf(stderr, "%.*s\n", (int)(end - line), line);

    // Show the error message.
    int pos = loc - line + ident;
    fprintf(stderr, "%*s", pos, ""); // print pos spaces.
    fprintf(stderr, "^ ");
    fprintf(stderr, fmt, ap);
    fprintf(stderr, "\n");
    exit(1);
}

void error_at(char *loc, char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);

    int pos = loc - user_input;
    fprintf(stderr, "%s\n", user_input);
    fprintf(stderr, "%*s", pos, ""); // pos個の空白を出力
    fprintf(stderr, "^ ");
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, "¥n");
    exit(1);
}

int is_alnum(char c) {
    return ('a' <= c && c <= 'z') ||
    ('A' <= c && c <= 'Z') ||
    ('0' <= c && c <= '9') ||
    (c == '_');
}

Node *new_node(int kind, Node *lhs, Node *rhs) {
    Node *node = malloc(sizeof(Node));
    node->kind = kind;
    node->lhs = lhs;
    node->rhs = rhs;
    return node;
}

Node *new_node_block(Vector *stmts) {
    Node *node = malloc(sizeof(Node));
    node->kind = ND_BLOCK;
    node->stmts = stmts;
    return node;
}

Node *new_node_call(char *name, Vector *args) {
    Node *node = malloc(sizeof(Node));
    node->kind = ND_CALL;
    node->name = name;
    node->args = args;
    return node;
}

Node *new_node_function(char *name, Vector *args, Node *block) {
    Node *node = malloc(sizeof(Node));
    node->kind = ND_FUNC;
    node->name = name;
    node->args = args;
    node->block = block;
    return node;
}

Node *new_node_num(long val) {
    Node *node = malloc(sizeof(Node));
    node->kind = ND_NUM;
    node->val = val;
    return node;
}

Node *new_node_ident(Type *ty, char *name) {
    Node *node = malloc(sizeof(Node));
    node->kind = ND_IDENT;
    //node->name = malloc(sizeof(char));
    node->ty = ty;
    node->name = strndup(name, strlen(name));
    var_append(variables, ty, name);
    return node;
}

Node *new_node_for(Node *init, Node *cond, Node * incdec, Node *body) {
    Node *node = malloc(sizeof(Node));
    node->kind = ND_FOR;
    node->init = init;
    node->cond = cond;
    node->incdec = incdec;
    node->body = body;
    return node;
}

Node *new_node_if(Node *cond, Node *body, Node *elseBody) {
    Node *node = malloc(sizeof(Node));
    node->kind = ND_IF;
    node->cond = cond;
    node->body = body;
    node->elseBody = elseBody;
    return node;
}

Node *new_node_while(Node *cond, Node *body) {
    Node *node = malloc(sizeof(Node));
    node->kind = ND_WHILE;
    node->cond = cond;
    node->body = body;
    return node;
}

Node *code[100];

Node *assign() {
    Node *node = equality();
    if (consume("="))
        node = new_node('=', node, assign());
    return node;
}

Node *expr() {
    return assign();
}

Node *function() {
    Node *node;
    if (!consume("int"))
        error_at(token->str, "intではないトークンです");
    while (consume("*")) {
        // do something
        //if (!consume("*"))
        //    error_at(token->str, "'*'ではないではないトークンです");
    }
    Token *ident = consume_ident();
    if (ident == NULL)
        error_at(token->str, "関数名ではないではないトークンです");
    if (!consume("("))
        error_at(token->str, "'('ではないトークンです");        
    Vector *args = new_vector();
    while (!consume(")")) {
        if (!consume("int"))
            error_at(token->str, "intではないトークンです");
        Type *ty = malloc(sizeof(Type));
        ty->ty = INT;
        Token *var = consume_ident();
        if (var != NULL) {
            var_append(variables, ty, strndup(var->str,var->len));
            vec_push(args, (void *) strndup(var->str,var->len));
        } else {
            while (!consume_ident()) {
                if (!consume("*"))
                    error_at(token->str, "'*'ではないではないトークンです");
                Type *next = ty;
                ty = malloc(sizeof(Type));
                ty->ty = PTR;
                ty->ptr_to = next;
            }
        }
        consume(",");
    }
    if (!consume("{"))
        error_at(token->str, "'{'ではないトークンです");        
    Vector *stmts = new_vector();
    while(!consume("}")) {
        vec_push(stmts, stmt());
    }
    Node *block = new_node_block(stmts);
    return new_node_function(strndup(ident->str,ident->len), args, block);
}

Node *stmt() {
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
            char *name = strndup(ident->str,ident->len);
            node = new_node_ident(ty, name);
            if (!consume(";"))
                error_at(token->str, "';'ではないトークンです");
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
        if(!consume("("))
            error_at(token->str, "'('ではないトークンです");
        Node *cond = expr();
        if(!consume(")"))
            error_at(token->str, "')'ではないトークンです");
        Node *body = stmt();
        if(consume("else")) {
            Node *elseBody = stmt();
            return new_node_if(cond, body, elseBody);
        }
        return new_node_if(cond, body, NULL);
    }
    if (consume("while")) {
        if(!consume("("))
            error_at(token->str, "'('ではないトークンです");
        Node *cond = expr();
        if(!consume(")"))
            error_at(token->str, "')'ではないトークンです");
        Node *body = stmt();
        return new_node_while(cond, body);
    }
    if (consume("for")) {
        if(!consume("("))
            error_at(token->str, "'('ではないトークンです");
        Node *init = NULL;
        if(!consume(";")) {
            init = expr();
            if(!consume(";"))
                error_at(token->str, "';'ではないトークンです");
        }
        Node *cond = NULL;
        if(!consume(";")) {
            cond = expr();
            if(!consume(";"))
                error_at(token->str, "';'ではないトークンです");
        }
        Node *incdec = NULL;
        if(!consume(")")) {
            incdec = expr();
            if(!consume(")"))
                error_at(token->str, "')'ではないトークンです");
        }
        Node *body = stmt();
        return new_node_for(init, cond, incdec, body);
    }
    if (consume("return")) {
        node = new_node(ND_RETURN, expr(), NULL);
    } else {
        node = expr();
    }
    if (!consume(";"))
        error_at(token->str, "';'ではないトークンです");
    return node;
}

void program() {
    int i = 0;
    while (!at_eof()) {
        //code[i++] = stmt();
        code[i++] = function();
    }
    code[i] = NULL;
}

Node *equality() {
    Node *node = relational();
    
    for (;;) {
        if (consume("=="))
            node = new_node(ND_EQ, node, relational());
        else if (consume("!="))
            node = new_node(ND_NE, node, relational());
        else
            return node;
    }
}

Node *relational() {
    // lhs
    Node *node = add();
    
    for (;;) {
        if (consume("<"))
            node = new_node(ND_LT, node, add());
        else if (consume("<="))
            node = new_node(ND_LE, node, add());
        else if (consume(">"))
            node = new_node(ND_LT, add(), node);
        else if (consume(">="))
            node = new_node(ND_LE, add(), node);
        else
            return node;
    }
}

Node *add() {
    // lhs
    Node *node = mul();
    
    for (;;) {
        if (consume("+"))
            node = new_node('+', node, mul());
        else if (consume("-"))
            node = new_node('-', node, mul());
        else
            return node;
    }
}

Node *mul() {
    Node *node = unary();
    
    for (;;) {
        if (consume("*"))
            node = new_node('*', node, unary());
        else if (consume("/"))
            node = new_node('/', node, unary());
        else
            return node;
        
    }
}

Node *term() {
    if (consume("(")) {
        Node *node = equality();
        if (!consume(")")) {
            error("開き括弧に対する閉じ括弧がありません。：%s", token->str);
        }
        return node;
    }
    
    if (token->kind == TK_NUM ) {
        return new_node_num(expect_number());
    }
    Token *ident = consume_ident();
    if (ident != NULL) {
        char *name = strndup(ident->str,ident->len);
        if (!consume("(")) {
            if (!var_exist(variables, name)) 
                error("未定義の変数です。：%s", name);
            /*Var *var;
            while(variables->next != NULL){
                if(strcmp(variables->name, name) == 0){
                    var = variables;
                }
                variables = variables->next;
            };*/
            Var *var = var_get(variables, name);
            return new_node_ident(var->ty,name);
        }
        Vector *args = new_vector();
        while (!consume(")")) {
            Node *node = add();
            vec_push(args, (void *) node);
            consume(",");
            /*if (get(tokens,pos)->kind == TK_NUM) {
                vec_push(args, (void *) get(tokens,pos++)->val);
                consume(",");
            }*/
        }
        
        return new_node_call(name, args);
    }
    
    error("数値でも開き括弧でもないトークンです： %s ", token->str);
    return NULL;
}

Node *unary() {
    if (consume("+")) {
        return term();
    }
    if (consume("-")) {
        // -x => 0-x
        return new_node('-',new_node_num(0), term());
    }
    if (consume("*")) {
        return new_node(ND_DEREF, unary(), NULL);
    }
    if (consume("&")) {
        return new_node(ND_ADDR, unary(), NULL);
    }
    return term();
}

bool startswith(char *p, char * q) {
    return memcmp(p, q, strlen(q)) == 0;
}

Token *peek(char *s){
    if (token->kind != TK_RESERVED || strlen(s) != token->len ||
        strncmp(token->str,s,token->len))
        return NULL;
    return token;
}

bool consume(char *s) {
    if (token->kind != TK_RESERVED || strlen(s) != token->len ||
        strncmp(token->str,s,token->len))
        return false;
    token = token->next;
    return true;
}

Token *consume_ident(void){
    if (token->kind != TK_IDENT)
        return NULL;
    Token *t = token;
    token = token->next;
    return t;
}

void expect(char *op) {
    if (!peek(op))
        error_at(token->str, "expected '%c'", op);
    token = token->next;
}

long expect_number() {
    if (token->kind != TK_NUM)
        error_at(token->str,"expected a number");
    long val = token->val;
    token = token->next;
    return val;
}

char *expect_ident() {
    if (token->kind != TK_IDENT)
        error_at(token->str,"expected a identifier");
    char *s = strndup(token->str, token->len);
    token = token->next;
    return s;
}

bool at_eof() {
    return token->kind == TK_EOF;
}

char *strndup(char *p,int len) {
    char *buf = malloc(len + 1);
    strncpy(buf, p , len);
    buf[len] = '\0';
    return buf;
}
//Token *token;

Token *new_token(TokenKind kind, Token *cur, char *str, int len) {
    Token *t = calloc(1, sizeof(Token));
    t->kind = kind;
    t->len = len;
    t->str = str;
    cur->next = t;
    return t;
}

Token *new_token_num(Token *cur, int val, char *str) {
    Token *t = calloc(1, sizeof(Token));
    t->kind = TK_NUM;
    t->val = val;
    t->str = str;
    cur->next = t;
    return t;
}

Token *new_token_ident(Token *cur, char *str, int len) {
    Token *t = calloc(1,sizeof(Token));
    t->kind = TK_IDENT;
    t->name = strndup(str, len);
    t->str = str;
    cur->next = t;
    return t;
}

Token *tokenize(char *p) {
    Token head = {};
    Token *cur = &head;
    
    while (*p) {
        if (isspace(*p)) {
            p++;
            continue;
        }
        
        // Multi-letter punctuator
        if (startswith(p, "==") || startswith(p, "!=") ||
            startswith(p, "<=") || startswith(p, ">=")) {
            cur = new_token(TK_RESERVED, cur, p, 2);
            p = p + 2;
            continue;
        }
        
        // single-letter punctuator
        if (strchr("+-*/(){}<>;,&=", *p)) {
            cur = new_token(TK_RESERVED, cur, p, 1);
            p++;
            continue;
        }

        if (strncmp(p, "int", 3) == 0 && !is_alnum(p[3])){
            cur = new_token(TK_RESERVED,cur,p,3);
            p += 3;
            continue;
        }

        if (strncmp(p, "if", 2) == 0 && !is_alnum(p[2])){
            cur = new_token(TK_RESERVED,cur,p,2);
            p += 2;
            continue;
        }

        if (strncmp(p, "for", 3) == 0 && !is_alnum(p[3])){
            cur = new_token(TK_RESERVED,cur,p,3);
            p += 3;
            continue;
        }

        if (strncmp(p, "else", 4) == 0 && !is_alnum(p[4])){
            cur = new_token(TK_RESERVED,cur,p,4);
            p += 4;
            continue;
        }

        if (strncmp(p, "return", 6) == 0 && !is_alnum(p[6])){
            cur = new_token(TK_RESERVED,cur,p,6);
            p += 6;
            continue;
        }

        if (strncmp(p, "while", 5) == 0 && !is_alnum(p[5])){
            cur = new_token(TK_RESERVED,cur,p,5);
            p += 5;
            continue;
        }

        if ('a' <= *p && *p <= 'z') {
            int i = 0;
            while(isalpha(p[i]))
                i++;
            cur = new_token(TK_IDENT,cur,p,i);
            p += i;
            continue;
        }
        
        // Integer literal
        if (isdigit(*p)) {
            char *start = p;
            long val = strtol(p, &p, 10);
            cur = new_token(TK_NUM,cur,start,p - start);
            cur->val = val;
            continue;
        }
        
        error("トークナイズできません: %s", p);
        exit(1);
    }
    
    new_token(TK_EOF,cur,p,0);
    return head.next;
}
