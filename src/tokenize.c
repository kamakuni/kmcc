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
    fprintf(stderr, "\n");
    exit(1);
}

bool is_alpha(char c) {
  return  ('a' <= c && c <= 'z') ||
    ('A' <= c && c <= 'Z') ||
    (c == '_');
}

bool is_alnum(char c) {
  return is_alpha(c) ||
    ('0' <= c && c <= '9');
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
        if (ispunct(*p)) {
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

        if (is_alpha(*p)) {
            int i = 0;
            while(is_alnum(p[i]))
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
