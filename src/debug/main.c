#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Values for token types
typedef enum {
    TK_RESERVED, // Keywords or punctuators
    TK_NUM, // Integer literals
    TK_IDENT, // Identifiers
    TK_EOF, // End-of-file markers
} TokenKind;

typedef struct Token Token;
// Type for tokens
struct Token {
    TokenKind kind; // token kind
    Token *next;
    int len; // token length
    long val; // value for Integer token
    char *name; // name for Ident
    char *str; // token stirng for debugging
};

char *user_input;
int pos;
Token *token;

Token *new_token(TokenKind kind, Token *cur, char *str, int len) {
    Token *tok = calloc(1, sizeof(Token));
    tok->kind = kind;
    tok->len = len;
    tok->str = str;
    cur->next = tok;
    return tok;
}

int is_alnum(char c) {
    return ('a' <= c && c <= 'z') ||
    ('A' <= c && c <= 'Z') ||
    ('0' <= c && c <= '9') ||
    (c == '_');
}

Token *tokenize(char *p) {
    Token head = {};
    Token *cur = &head;
    
    while (*p) {
        if (isspace(*p)) {
            p++;
            continue;
        }
        
        if (strncmp(p,"<=",2) == 0){
            cur = new_token(TK_RESERVED, cur, p, 2);
            p = p + 2;
            continue;
        }
        
        if (strncmp(p,">=",2) == 0){
            cur = new_token(TK_RESERVED, cur, p, 2);
            p = p + 2;
            continue;
        }
        
        if ( *p == '+'
          || *p == '-'
          || *p == '*'
          || *p == '/'
          || *p == '('
          || *p == ')'
          || *p == '{'
          || *p =='}'
          || *p == '<'
          || *p == '>'
          || *p == ';'
          || *p == ','
          || *p == '&') {
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
        
        if (isdigit(*p)) {
            char *start = p;
            long val = strtol(p, &p, 10);
            cur = new_token(TK_NUM,cur,p,p - start);
            cur->val = val;
            continue;
        }
        
        if (strncmp(p,"==",2) == 0){
            cur = new_token(TK_RESERVED,cur,p,2);
            p = p + 2;
            continue;
        }
        
        if (strncmp(p,"!=",2) == 0){
            cur = new_token(TK_RESERVED,cur,p,2);
            p = p + 2;
            continue;
        }
        
        if ( *p == '=' ) {
            cur = new_token(TK_RESERVED,cur,p,1);
            p++;
            continue;
        }
        
        //error("トークナイズできません: %s", p);
        exit(1);
    }
    
    new_token(TK_EOF,cur,p,0);
    return head.next;
}


int main(int argc, char **argv) {
    if (argc != 2) {
        fprintf(stderr, "引数の個数が正しくありません\n");
        return 1;
    }
    // run testing codes
    if (strcmp(argv[1],"-test") == 0){
        //runtest();
        return 0;
    }
    
    pos = 0;
    //variables = new_var();
    user_input = argv[1];
    token = tokenize(user_input);
    
    // tokens to syntax tree
    //program();
    
    printf(".intel_syntax noprefix\n");
    printf(".global main\n");

    //for (int i = 0; code[i]; i++) {
    //    gen_func(code[i]);

    return 0;
}

/*char *strndup(char *p,int len) {
    char *buf = malloc(len + 1);
    strncpy(buf, p , len);
    buf[len] = '\0';
    return buf;
}*/

/*
Token *new_token_num(Token *cur, int val, char *str) {
    Token *tok = calloc(1, sizeof(Token));
    tok->kind = TK_NUM;
    tok->val = val;
    tok->str = str;
    cur->next = tok;
    return tok;
}

Token *new_token_ident(Token *cur, char *str, int len) {
    Token *tok = calloc(1,sizeof(Token));
    tok->kind = TK_IDENT;
    tok->name = strndup(str, len);
    tok->str = str;
    cur->next = tok;
    return tok;
}
*/
