.intel_syntax noprefix
.global main
main:
  push rbp
  mov rbp, rsp
  sub rsp, 0
  push 1
  pop rax
  push 2
  pop rax
  push 3
  pop rax
  mov rsp, rbp
  pop rbp
  ret
