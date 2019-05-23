#include "compiler.h"

void error(char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, "\n");
    exit(1);
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

    if ( get(tokens,pos)->ty == TK_NUM )
        return new_node_num(get(tokens,pos++)->val);
    
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

void tokenize(char *p) {

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

        if ('a' <= *p && *p <= 'z') {
            append(tokens, new_token(TK_IDENT,p));
            p++;
            continue;
        }

        if (isdigit(*p)) {
            append(tokens, new_token_num(TK_NUM, strtol(p, &p, 10), p));
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

        error("トークナイズできません: %s", p);
        exit(1);
    }

    append(tokens, new_token(TK_EOF,p));
}
