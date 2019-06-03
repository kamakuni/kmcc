#include "compiler.h"

Vector *new_vector() {
    Vector *vec = malloc(sizeof(Vector));
    vec->data = malloc(sizeof(void *)* 16);
    vec->capacity = 16;
    vec->len = 0;
    return vec;
}

void vec_push(Vector *vec, void *elem) {
    if (vec->capacity == vec->len) {
        vec->capacity *= 2;
        vec->data = realloc(vec->data, sizeof(void *) * vec->capacity );
    }
    vec->data[vec->len++] = elem;
}

Token *new_token(int ty, char *input) {
    Token *t = malloc(sizeof(Token));
    t->ty = ty;
    t->input = input;
    return t;
}

Token *new_token_num(int val, char *input) {
    Token *t = malloc(sizeof(Token));
    t->ty = TK_NUM;
    t->val = val;
    t->input = input;
    return t;
}

Token *new_token_ident(char *input) {
    Token *t = malloc(sizeof(Token));
    t->ty = TK_IDENT;
    t->input = malloc(sizeof(char));
    strncpy(t->input,input,1);
    return t;
}

Tokens *new_tokens(){
    Tokens *t = malloc(sizeof(Tokens));
    //t->i = 0;
    t->vec = new_vector();
    return t;
}

void append(Tokens *t, Token *elem) {
    vec_push(t->vec,(void *) elem);
}

Token *get(Tokens *t, int i) {
    return (Token *) t->vec->data[i];
}

int expect(int line, int expected, int actual) {
    if (expected == actual) 
        return 0;
    fprintf(stderr,"%d: %d expected, but got %d\n",line, expected, actual);
    exit(1);
}

void runtest(){
    Vector *vec = new_vector();
    expect(__LINE__, 0, vec->len);

    for (int i = 0; i < 100; i++)
        vec_push(vec,(void *)i);

    expect(__LINE__, 100, vec->len);
    expect(__LINE__, 0, (long)vec->data[0]);
    expect(__LINE__, 50, (long)vec->data[50]);
    expect(__LINE__, 99, (long)vec->data[99]);

    Tokens *tokens = new_tokens();
    append(tokens, new_token_num(2,"3"));
    Token *token = get(tokens,0);

    expect(__LINE__, TK_NUM, token->ty);
    expect(__LINE__, 2, token->val);
    expect(__LINE__, 0, strcmp("3", token->input));

    append(tokens, new_token_num(3,"4"));

    token = get(tokens,1);

    expect(__LINE__, TK_NUM, token->ty);
    expect(__LINE__, 3, token->val);
    expect(__LINE__, 0, strcmp("4", token->input));

    printf("OK\n");
}
