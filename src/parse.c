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

Node *new_node_num(int val) {
    Node *node = malloc(sizeof(Node));
    node->ty = ND_NUM;
    node->val = val;
    return node;
}

Node *new_node_ident(char *name) {
    Node *node = malloc(sizeof (Node));
    node->ty = ND_IDENT;
    node->name = name;
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

Node *stmt() {
    Node *node;
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
        code[i++] = stmt();
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

    if (get(tokens,pos)->ty == TK_IDENT ){
        return new_node_ident(get(tokens,pos++)->input);
    }
    error("数値でも開き括弧でもないトークンです： %s ", get(tokens,pos)->input);

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

        if ( *p == '+' || *p == '-' || *p == '*' || *p == '/' || *p == '(' || *p ==')' || *p == '<' || *p == '>' ) {
            append(tokens, new_token(*p,p));
            p++;
            continue;
        }

        if (strncmp(p, "return", 6) == 0 && !is_alnum(p[6])){
            append(tokens, new_token(TK_RETURN,p));
            p += 6;
            continue;
        }

        if ('a' <= *p && *p <= 'z') {
            /*Token *t = new_token(TK_IDENT, NULL);
            t->input = malloc(sizeof(char));
            strncpy(t->input,p,1);*/
            append(tokens, new_token_ident(p));
            p++;
            continue;
        }

        if ( *p == ';' ) {
            append(tokens, new_token(*p,p));
            p++;
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
