#include "kmcc.h"

// Return the contents of a given file.
static char *read_file(char *path) {
  // Open and read file.
  FILE *fp = fopen(path, "r");
  if (!fp)
    error("cannot open %s: %s", path, strerror(errno));
  
  int filemax = 10 * 1024 * 1024;
  char *buf = malloc(filemax);
  int size = fread(buf, 1, filemax - 2, fp);
  if (!feof(fp))
    error("file too large");
  
  // Make sure that the string ends with "\n\0".
  if (size == 0 | buf[size - 1] != '\n')
    buf[size++] = '\n';
  buf[size] = '\0';
  return buf;
}

int main(int argc, char **argv) {

  if (argc != 2)
    error("%s: invalid number of arguments", argv[0]);
    
  // Run testing codes.
  if (strcmp(argv[1],"-test") == 0){
    //runtest();
    return 0;
  }

  // Tokenize and parse.
  filename = argv[1];
  user_input = read_file(argv[1]);
  token = tokenize();
  Program *prog = program();

  // Assign offsets to local variables.
  for (Function *fn = prog->fns; fn; fn = fn->next) {
    int offset = fn->has_varargs ? 56 : 0;
    for (VarList *vl = fn->locals; vl; vl = vl->next) {
	    Var *var = vl->var;
      offset = align_to(offset, var->ty->align);
	    offset += var->ty->size;
	    var->offset = offset;
    }
    fn->stack_size = align_to(offset, 8);
  }

  // Traverse the AST to emit assembly.
  codegen(prog);
  
  return 0;
}
