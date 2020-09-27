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

// Reports an error location and exit.
void error_tok(Token *tok, char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  verror_at(tok->str,fmt, ap);
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
  return strncmp(p, q, strlen(q)) == 0;
}

static char *starts_with_reserved(char *p) {
  // Keyword
  static char *kw[] = {"return", "if", "else", "while", "for", "int", "char", "sizeof"};

  for (int i = 0; i < sizeof(kw) / sizeof(*kw); i++) {
    int len = strlen(kw[i]);
    if (startswith(p, kw[i]) && !is_alnum(p[len]))
      return kw[i];
  }

  // Multi-letter punctuator
  static char *ops[] = {"==", "!=", "<=", ">="};

  for (int i = 0; i < sizeof(ops) / sizeof(*ops); i++)
    if (startswith(p, ops[i]))
      return ops[i];

  return NULL;
}

// Returns true if the current token matches a ginven string.
Token *peek(char *s){
    if (token->kind != TK_RESERVED || strlen(s) != token->len ||
        strncmp(token->str,s,token->len))
        return NULL;
    return token;
}

Token *consume(char *op) {
    if (token->kind != TK_RESERVED || strlen(op) != token->len ||
        strncmp(token->str,op,token->len))
        return NULL;
    Token *t = token;
    token = token->next;
    return t;
}

Token *consume_ident(void){
    if (token->kind != TK_IDENT)
        return NULL;
    Token *t = token;
    token = token->next;
    return t;
}

// Ensure that he current token is `op`
void expect(char *s) {
    if (!peek(s))
        error_at(token->str, "expected \"%s\"", s);
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

static char get_escape_char(char c) {
  if (c == 'a') {
    return '\a';
  } else if (c == 'b') {
    return '\b';
  } else if (c == 't') {
    return '\t';
  } else if (c == 'n') {
    return '\n';
  } else if (c == 'v') {
    return '\v';
  } else if (c == 'f') {
    return '\f';
  } else if (c == 'r') {
    return '\r';
  } else if (c == 'e') {
    return 27;
  } else if (c == '0') {
    return 0;
  } else {
    return c;
  }
}

char *strndup(char *p,int len) {
    char *buf = malloc(len + 1);
    strncpy(buf, p , len);
    buf[len] = '\0';
    return buf;
}

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

static Token *read_string_literal(Token *cur, char *start) {
  char *p = start + 1;
  char buf[1024];
  int len = 0;

  for (;;) {
    if (len == sizeof(buf))
      error_at(start, "string literal too large");
    if (*p == '\0')
      error_at(start, "unclosed string literal");
    if (*p == '"')
      break;

    if (*p == '\\') {
      p++;
      buf[len++] = get_escape_char(*p++);
    } else {
      buf[len++] = *p++;
    }
  }

  Token *tok = new_token(TK_STR, cur, start, p - start + 1);
  tok->contents = malloc(len + 1);
  memcpy(tok->contents, buf, len);
  tok->contents[len] = '\0';
  tok->cont_len = len + 1;
  return tok;
}

Token *tokenize(char *p) {
  Token head = {};
  Token *cur = &head;
    
  while (*p) {
    // Skip whitespace characters.
    if (isspace(*p)) {
      p++;
      continue;
    }

    // Skip line comments.
    if (startswith(p, "//")) {
      p += 2;
      while(*p != '\n')
        p++;
      continue;
    }

    // Skip block comments.
    if (startswith(p, "/*")) {
      char *q = strstr(p + 2, "*/");
      if (!q)
        error_at(p, "unclosed block comments");
      p = q + 2;
      continue;
    }

    // String literal
    if (*p == '"') {
      cur = read_string_literal(cur, p);
      p += cur->len;
      continue;
    }
          
    // Keywords or Multi-letter punctuators
    char *kw = starts_with_reserved(p);
    if (kw) {
      int len = strlen(kw);
      cur = new_token(TK_RESERVED, cur, p, len);
      p += len;
      continue;
    }

    // Identifier
    if (is_alpha(*p)) {
      char *q = p++;
      while(is_alnum(*p))
        p++;
      cur = new_token(TK_IDENT, cur, q, p - q);
      continue;
    }

    // Single-letter punctuators
    if (ispunct(*p)) {
      cur = new_token(TK_RESERVED, cur, p++, 1);
      continue;
    }
      
    // Integer literal
    if (isdigit(*p)) {
      cur = new_token(TK_NUM, cur, p ,0);
      char *q = p;
      cur->val = strtol(p, &p, 10);
      cur->len = p - q;
      continue;
    }
      
    error_at(p, "invalid token");
  }
  
  new_token(TK_EOF ,cur ,p ,0 );
  return head.next;
}
