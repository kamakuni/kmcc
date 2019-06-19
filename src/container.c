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
    var->name = "";
    var->offset = 0;
    return var;
}

void var_insert_first(Var **var,char *name, int offset){
    Var *new = malloc(sizeof(Var));
    new->next = *var;
    new->name = name;
    new->offset = offset;
    *var = new;
}

Var *var_get(Var *var, char *name){
    while(var->next != NULL){
        if(strcmp(var->next->name, name) == 0){
            return var->next;
        }
        var = var->next;
    };
    return NULL;
}

int var_len(Var *var){
    int i = 0;
    while(var->next != NULL){
        i++;
        var = var->next;
    };
    return i;
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

Token *new_token_ident(char *name) {
    Token *t = malloc(sizeof(Token));
    t->ty = TK_IDENT;
    t->input = malloc(sizeof(char));
    strncpy(t->input,name,strlen(name));
    t->name = malloc(sizeof(char));
    strncpy(t->name,name,strlen(name));
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
    
    expect(__LINE__, TK_NUM, token->ty);
    expect(__LINE__, 2, token->val);
    expect(__LINE__, 0, strcmp("3", token->input));
    
    append(tokens, new_token_num(3,"4"));
    
    token = get(tokens,1);
    
    expect(__LINE__, TK_NUM, token->ty);
    expect(__LINE__, 3, token->val);
    expect(__LINE__, 0, strcmp("4", token->input));
}

void test_linked_list(){
    Var *var = new_var();
    expect(__LINE__, 0, var->offset);
    expect(__LINE__, 0, strcmp("", var->name));
    var_insert_first(var, "name1", (var_len(var)+1)*8);
    expect(__LINE__, 8, var->offset);
    expect(__LINE__, 0, strcmp("name1", var->name));
    var_insert_first(var, "name2", (var_len(var)+1)*8);
    expect(__LINE__, 16, var->offset);
    expect(__LINE__, 0, strcmp("name2", var->name));
    Var *var_name1 = var_get(var, "name1");
    var_insert_first(var, var_name1->name, var_name1->offset);
    expect(__LINE__, 8, var->offset);
    expect(__LINE__, 0, strcmp("name1", var->name));
}

void runtest(){
    test_vector();
    test_map();
    test_linked_list();
    printf("OK\n");
}

