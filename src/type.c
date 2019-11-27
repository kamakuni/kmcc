#include "kmcc.h"

Type *int_type = &(Type){ TY_INT };

bool is_integer(Type *ty){
  return ty->kind == TY_INT;
}
