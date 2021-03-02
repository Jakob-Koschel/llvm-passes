// RUN: %clang %s -O2 -S -emit-llvm -o %t.ll
// RUN: llvm-as %t.ll -o %t.bc
// RUN: opt -enable-new-pm=0 -load %hello_pass_path --legacy-hello %t.ll -o %t.opt.ll \
// RUN:   > %t.opt.legacypm.log 2>&1
// RUN: opt -enable-new-pm=1 -load-pass-plugin %hello_pass_path --passes="hello-new-pm" %t.ll -o %t.opt.ll \
// RUN:   > %t.opt.newpm.log 2>&1
//
// RUN: %clang %s -flegacy-pass-manager -Xclang -load -Xclang %hello_pass_path -O2 -o %t \
// RUN:   > %t.clang.legacypm.log 2>&1
// RUN: %clang %s -fno-legacy-pass-manager -fpass-plugin=%hello_pass_path -O2 -o %t.ll \
// RUN:   > %t.clang.newpm.log 2>&1

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>

int test_func(int num) {
  return num * 2;
}

int main(int argc, char *argv[]) {
  int num = test_func(argc);
  printf("hello (%d)!\n", num);
  return 0;
}
