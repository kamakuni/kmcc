#include "kmcc.h"

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

void *vec_get(Vector *vec, int i) {
    return vec->data[i];
}

Map *new_map(){
    Map *map = malloc(sizeof(Map));
    map->keys = new_vector();
    map->vals = new_vector();
    return map;
};
void map_put(Map *map, char *key, void *val){
    vec_push(map->keys, key);
    vec_push(map->vals, val);
};

Map *map_get(Map *map,char *key){
    for(int i = map->keys->len - 1; i >=0; i--){
        if(strcmp(map->keys->data[i],key) == 0)
            return map->vals->data[i];
    }
    return NULL;
};

Var *new_var(){
    Var *var = malloc(sizeof(Var));
    var->next = NULL;
    var->ty = NULL;
    var->name = "";
    var->offset = 0;
    return var;
}

void var_insert_first(Var **var, Type *ty, char *name, int offset){
    Var *new = malloc(sizeof(Var));
    new->next = *var;
    new->ty = ty;
    new->name = name;
    new->offset = offset;
    *var = new;
}

int var_exist(Var *var, char *name){
    while(var->next != NULL){
        if(strcmp(var->name, name) == 0){
            return 1;
        }
        var = var->next;
    };
    return 0;
}

Var *var_get(Var *var,char *name) {
    while(var->next != NULL){
        if(strcmp(var->name, name) == 0){
            Var *res = new_var();
            res->ty = var->ty;
            res->name = var->name;
            res->offset = var->offset;
            return res;
        }
        var = var->next;
    };
    return NULL;
}

int var_get_offset(Var *var, char *name){
    while(var->next != NULL){
        if(strcmp(var->name, name) == 0){
            return var->offset;
        }
        var = var->next;
    };
    return 0;
}

int var_len(Var *var){
    int i = 0;
    while(var->next != NULL){
        i++;
        var = var->next;
    };
    return i;
}

void var_append(Var *var, Type *ty, char *name){
    int offset = (var_len(variables) + 1) * 8;
    int current = var_get_offset(variables, name);
    if (current != 0)
        offset = current;
    var_insert_first(&variables, ty, name, offset);
}

Token *new_token(TokenKind kind, char *input) {
    Token *t = malloc(sizeof(Token));
    t->kind = kind;
    t->input = input;
    return t;
}

Token *new_token_num(int val, char *input) {
    Token *t = malloc(sizeof(Token));
    t->kind = TK_NUM;
    t->val = val;
    t->input = input;
    return t;
}

Token *new_token_ident(char *input, int len) {
    // if(len > strlen(input))
    // return NULL; 
    Token *t = calloc(1,sizeof(Token));
    //Token *t = malloc(sizeof(Token));
    t->kind = TK_IDENT;
    t->name = strndup(input, len);
    t->input = input;
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

void test_map() {
    Map *map = new_map();
    expect(__LINE__, 0,(long)map_get(map,"foo"));
    
    map_put(map, "foo", (void *)2);
    expect(__LINE__, 2,(long)map_get(map,"foo"));
    
    map_put(map, "bar", (void *)3);
    expect(__LINE__, 3,(long)map_get(map,"bar"));
    
    map_put(map, "foo", (void *)4);
    expect(__LINE__, 4,(long)map_get(map,"foo"));
}

void test_vector(){
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
    
    expect(__LINE__, TK_NUM, token->kind);
    expect(__LINE__, 2, token->val);
    expect(__LINE__, 0, strcmp("3", token->input));
    
    append(tokens, new_token_num(3,"4"));
    
    token = get(tokens,1);
    
    expect(__LINE__, TK_NUM, token->kind);
    expect(__LINE__, 3, token->val);
    expect(__LINE__, 0, strcmp("4", token->input));
}

void test_linked_list(){
    Var *var = new_var();
    expect(__LINE__, 0, var->offset);
    expect(__LINE__, 0, strcmp("", var->name));
    Type *ty = malloc(sizeof(Type));
    ty->ty = INT;
    var_insert_first(&var, ty, "name1", (var_len(var)+1)*8);
    expect(__LINE__, 8, var->offset);
    expect(__LINE__, INT, var->ty->ty);
    expect(__LINE__, 0, strcmp("name1", var->name));
    ty = malloc(sizeof(Type));
    ty->ty = PTR;
    var_insert_first(&var, ty, "name2", (var_len(var)+1)*8);
    expect(__LINE__, 16, var->offset);
    expect(__LINE__, PTR, var->ty->ty);
    expect(__LINE__, 0, strcmp("name2", var->name));
    int offset = var_get_offset(var, "name1");
    expect(__LINE__, 8, offset);
}

void runtest(){
    test_vector();
    test_map();
    test_linked_list();
    printf("OK\n");
}
