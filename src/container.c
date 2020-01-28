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
    var->ty = NULL;
    var->offset = 0;
    return var;
}

void var_insert_first(Var **var, Type *ty, char *name, int offset){
    Var *new = malloc(sizeof(Var));
    new->ty = ty;
    new->offset = offset;
    *var = new;
}

Tokens *new_tokens(){
    Tokens *t = malloc(sizeof(Tokens));
    t->vec = new_vector();
    return t;
}

void append(Tokens *t, Token *elem) {
    vec_push(t->vec,(void *) elem);
}

Token *get(Tokens *t, int i) {
    return (Token *) t->vec->data[i];
}

