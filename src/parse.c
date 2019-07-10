#include "compiler.h"

void error(char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, "\n");
    exit(1);
}

void error_at(char *loc, char *msg) {
    int pos = loc - user_input;
    fprintf(stderr, "%s\n", user_input);
    fprintf(stderr, "%*s", pos, ""); // pos個の空白を出力
    fprintf(stderr, "^ %s\n", msg);
    exit(1);
}

int is_alnum(char c) {
    return ('a' <= c && c <= 'z') ||
    ('A' <= c && c <= 'Z') ||
    ('0' <= c && c <= '9') ||
    (c == '_');
}

Node *new_node(int ty, Node *lhs, Node *rhs) {
    Node *node = malloc(sizeof(Node));
    node->ty = ty;
    node->lhs = lhs;
    node->rhs = rhs;
    return node;
}

Node *new_node_block(Vector *stmts) {
    Node *node = malloc(sizeof(Node));
    node->ty = ND_BLOCK;
    node->stmts = stmts;
    return node;
}

Node *new_node_call(char *name, Vector *args) {
    Node *node = malloc(sizeof(Node));
    node->ty = ND_CALL;
    node->name = name;
    node->args = args;
    return node;
}

Node *new_node_function(char *name, Var *args, Node *block) {
    Node *node = malloc(sizeof(Node));
    node->ty = ND_FUNC;
    node->name = name;
    node->args = args;
    node->block = block;
    return node;
}

Node *new_node_num(int val) {
    Node *node = malloc(sizeof(Node));
    node->ty = ND_NUM;
    node->val = val;
    return node;
}

Node *new_node_ident(char *name) {
    int offset = (var_len(variables) + 1) * 8;
    int current = var_get_offset(variables, name);
    if (current != 0)
        offset = current;
    var_insert_first(&variables, name, offset);
    Node *node = malloc(sizeof(Node));
    node->ty = ND_IDENT;
    node->name = malloc(sizeof(char));
    strncpy(node->name, name, strlen(name));
    return node;
}

Node *new_node_for(Node *init, Node *cond, Node * incdec, Node *body) {
    Node *node = malloc(sizeof(Node));
    node->ty = ND_FOR;
    node->init = init;
    node->cond = cond;
    node->incdec = incdec;
    node->body = body;
    return node;
}

Node *new_node_if(Node *cond, Node *body, Node *elseBody) {
    Node *node = malloc(sizeof(Node));
    node->ty = ND_IF;
    node->cond = cond;
    node->body = body;
    node->elseBody = elseBody;
    return node;
}

Node *new_node_while(Node *cond, Node *body) {
    Node *node = malloc(sizeof(Node));
    node->ty = ND_WHILE;
    node->cond = cond;
    node->body = body;
    return node;
}

Node *code[100];

Node *assign() {
    Node *node = equality();
    if (consume('='))
        node = new_node('=', node, assign());
    return node;
}

Node *expr() {
    return assign();
}

Node *function() {
    Node *node;
    char *name = get(tokens,pos)->name;
    if (!consume(TK_IDENT))
        error_at(get(tokens,pos)->input, "関数名ではないではないトークンです");
    if (!consume('('))
        error_at(get(tokens,pos)->input, "'('ではないトークンです");        
    Var *args = new_var();
    while (get(tokens,pos)->ty != ')') {
        if (get(tokens,pos)->ty == TK_IDENT) {
            //vec_push(args, (void *) get(tokens,pos++)->name);
            var_append(args,(void *) get(tokens,pos++)->name);
        } else if (get(tokens,pos)->ty ==  ','){
            pos++;
        }
    }
    if (!consume(')'))
        error("開き括弧に対する閉じ括弧がありません。：%s", get(tokens,pos)->input);
    if (!consume('{'))
        error_at(get(tokens,pos)->input, "'{'ではないトークンです");        
    Vector *stmts = new_vector();
    while(get(tokens, pos)->ty != '}') {
        vec_push(stmts, stmt());
    }
    if (!consume('}'))
        error_at(get(tokens,pos)->input, "'}'ではないトークンです");        
    return new_node_function(name, args, new_node_block(stmts));
}

Node *stmt() {
    Node *node;
    if (consume('{')) {
        Vector *stmts = new_vector();
        while(get(tokens, pos)->ty != '}') {
            vec_push(stmts, stmt());
        }
        if(!consume('}'))
            error_at(get(tokens,pos)->input, "'}'ではないトークンです");
        return new_node_block(stmts);
    }
    if (consume(TK_IF)) {
        if(!consume('('))
            error_at(get(tokens,pos)->input, "'('ではないトークンです");
        Node *cond = expr();
        if(!consume(')'))
            error_at(get(tokens,pos)->input, "')'ではないトークンです");
        Node *body = stmt();
        if(consume(TK_ELSE)) {
            Node *elseBody = stmt();
            return new_node_if(cond, body, elseBody);
        }
        return new_node_if(cond, body, NULL);
    }
    if (consume(TK_WHILE)) {
        if(!consume('('))
            error_at(get(tokens,pos)->input, "'('ではないトークンです");
        Node *cond = expr();
        if(!consume(')'))
            error_at(get(tokens,pos)->input, "')'ではないトークンです");
        Node *body = stmt();
        return new_node_while(cond, body);
    }
    if (consume(TK_FOR)) {
        if(!consume('('))
            error_at(get(tokens,pos)->input, "'('ではないトークンです");
        Node *init = NULL;
        if(get(tokens,pos)->ty != ';') {
            init = expr();
        }
        if(!consume(';'))
            error_at(get(tokens,pos)->input, "';'ではないトークンです");
        Node *cond = NULL;
        if(get(tokens,pos)->ty != ';') {
            cond = expr();
        }
        if(!consume(';'))
            error_at(get(tokens,pos)->input, "';'ではないトークンです");
        Node *incdec = NULL;
        if(get(tokens,pos)->ty != ')') {
            incdec = expr();
        }
        if(!consume(')'))
            error_at(get(tokens,pos)->input, "')'ではないトークンです");
        Node *body = stmt();
        return new_node_for(init, cond, incdec, body);
    }
    if (consume(TK_RETURN)) {
        node = new_node(ND_RETURN, expr(), NULL);
    } else {
        node = expr();
    }
    if (!consume(';'))
        error_at(get(tokens,pos)->input, "';'ではないトークンです");
    return node;
}

void program() {
    int i = 0;
    while (get(tokens,pos)->ty != TK_EOF) {
        //code[i++] = stmt();
        code[i++] = function();
    }
    code[i] = NULL;
}

Node *equality() {
    Node *node = relational();
    
    for (;;) {
        if (consume(TK_EQ))
            node = new_node(ND_EQ, node, relational());
        else if (consume(TK_NE))
            node = new_node(ND_NE, node, relational());
        else
            return node;
    }
}

Node *relational() {
    // lhs
    Node *node = add();
    
    for (;;) {
        if (consume('<'))
            node = new_node('<', node, add());
        else if (consume(TK_LE))
            node = new_node(ND_LE, node, add());
        else if (consume('>'))
            node = new_node('<', add(), node);
        else if (consume(TK_GE))
            node = new_node(ND_LE, add(), node);
        else
            return node;
    }
}

Node *add() {
    // lhs
    Node *node = mul();
    
    for (;;) {
        if (consume('+'))
            node = new_node('+', node, mul());
        else if (consume('-'))
            node = new_node('-', node, mul());
        else
            return node;
    }
}

Node *mul() {
    Node *node = unary();
    
    for (;;) {
        if (consume('*'))
            node = new_node('*', node, unary());
        else if (consume('/'))
            node = new_node('/', node, unary());
        else
            return node;
        
    }
}

Node *term() {
    if (consume('(')) {
        Node *node = equality();
        if (!consume(')')) {
            error("開き括弧に対する閉じ括弧がありません。：%s", get(tokens,pos)->input);
        }
        return node;
    }
    
    if ( get(tokens,pos)->ty == TK_NUM ) {
        return new_node_num(get(tokens,pos++)->val);
    }
    
    if (get(tokens,pos)->ty == TK_IDENT ) {
        char *name = get(tokens,pos++)->name;
        if (!consume('(')) {
            return new_node_ident(name);
        }
        Vector *args = new_vector();
        while (get(tokens,pos)->ty != ')') {
            if (get(tokens,pos)->ty == TK_NUM) {
                vec_push(args, (void *) get(tokens,pos++)->val);
                consume(',');
            }
        }
        if (!consume(')'))
            error("開き括弧に対する閉じ括弧がありません。：%s", get(tokens,pos)->input);
        
        return new_node_call(name, args);
    }
    
    error("数値でも開き括弧でもないトークンです： %s ", get(tokens,pos)->input);
    return NULL;
}

Node *unary() {
    if (consume('+')) {
        return term();
    }
    if (consume('-')) {
        // -x => 0-x
        return new_node('-',new_node_num(0), term());
    }
    return term();
}

int consume(int ty) {
    if (get(tokens, pos)->ty != ty)
        return 0;
    pos++;
    return 1;
}

void tokenize() {
    char *p = user_input;
    
    while (*p) {
        if (isspace(*p)) {
            p++;
            continue;
        }
        
        if (strncmp(p,"<=",2) == 0){
            append(tokens, new_token(TK_LE,p));
            p = p + 2;
            continue;
        }
        
        if (strncmp(p,">=",2) == 0){
            append(tokens, new_token(TK_GE,p));
            p = p + 2;
            continue;
        }
        
        if ( *p == '+' || *p == '-' || *p == '*' || *p == '/' || *p == '(' || *p ==')' || *p == '{' || *p =='}' || *p == '<' || *p == '>' || *p == ';' || *p == ',' ) {
            append(tokens, new_token(*p,p));
            p++;
            continue;
        }

        if (strncmp(p, "if", 2) == 0 && !is_alnum(p[2])){
            append(tokens, new_token(TK_IF,p));
            p += 2;
            continue;
        }

        if (strncmp(p, "for", 3) == 0 && !is_alnum(p[3])){
            append(tokens, new_token(TK_FOR,p));
            p += 3;
            continue;
        }

        if (strncmp(p, "else", 4) == 0 && !is_alnum(p[4])){
            append(tokens, new_token(TK_ELSE,p));
            p += 4;
            continue;
        }

        if (strncmp(p, "return", 6) == 0 && !is_alnum(p[6])){
            append(tokens, new_token(TK_RETURN,p));
            p += 6;
            continue;
        }

        if (strncmp(p, "while", 5) == 0 && !is_alnum(p[5])){
            append(tokens, new_token(TK_WHILE,p));
            p += 5;
            continue;
        }

        if ('a' <= *p && *p <= 'z') {
            int i = 0;
            while(isalpha(p[i]))
                i++;
            append(tokens, new_token_ident(p, i));
            p += i;
            continue;
        }
        
        if (isdigit(*p)) {
            append(tokens, new_token_num(strtol(p, &p, 10), p));
            continue;
        }
        
        if (strncmp(p,"==",2) == 0){
            append(tokens, new_token(TK_EQ,p));
            p = p + 2;
            continue;
        }
        
        if (strncmp(p,"!=",2) == 0){
            append(tokens, new_token(TK_NE,p));
            p = p + 2;
            continue;
        }
        
        if ( *p == '=' ) {
            append(tokens, new_token(*p,p));
            p++;
            continue;
        }
        
        error("トークナイズできません: %s", p);
        exit(1);
    }
    
    append(tokens, new_token(TK_EOF,p));
}
