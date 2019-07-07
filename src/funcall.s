.intel_syntax noprefix
.global main
main:
  push rbp
  mov rbp, rsp
  sub rsp, 0
  call foo
  pop rax
  mov rsp, rbp
  pop rbp
  ret
