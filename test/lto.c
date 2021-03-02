// [opt]
// RUN: %clang %s -O2 -S -emit-llvm -o %t.ll
// RUN: llvm-as %t.ll -o %t.bc
// RUN: opt -enable-new-pm=0 -load %hello_lto_pass_path --legacy-hello -hello-test %t.ll -o %t.opt.ll \
// RUN:   > %t.opt.legacypm.log 2>&1
// RUN: opt -enable-new-pm=1 -load-pass-plugin %hello_lto_pass_path --passes="hello-new-pm" %t.ll -o %t.opt.ll \
// RUN:   > %t.opt.newpm.log 2>&1
//
// [clang fullLTO]
// RUN: %clang %s -flegacy-pass-manager -O2 -fuse-ld=lld -flto -Wl,-mllvm=-load=%hello_lto_pass_path \
// RUN:   -Wl,-mllvm=-hello-test -o %t \
// RUN:   > %t.clang.full.legacypm.log 2>&1
//
// RUN: %clang %s -fno-legacy-pass-manager -O2 -fuse-ld=lld -flto \
// RUN: -Wl,-load-pass-plugin=%hello_lto_pass_path \
// RUN: -Wl,-mllvm=-load=%hello_lto_pass_path -o %t \
// RUN:    > %t.clang.full.newpm.log 2>&1
//
// [clang thinLTO]
// RUN: %clang %s -flegacy-pass-manager -O2 -fuse-ld=lld -flto=thin -Wl,-mllvm=-load=%hello_lto_pass_path \
// RUN:   -Wl,-mllvm=-hello-test -o %t \
// RUN:   > %t.clang.thin.legacypm.log 2>&1
//
// RUN: %clang %s -fno-legacy-pass-manager -O2 -fuse-ld=lld -flto=thin \
// RUN: -Wl,-load-pass-plugin=%hello_lto_pass_path \
// RUN: -Wl,-mllvm=-load=%hello_lto_pass_path -o %t \
// RUN:   > %t.clang.thin.newpm.log 2>&1
//
// [lld fullLTO]
// RUN: %clang %s -flto -O2 -c -o %t.o
// RUN: ld.lld -plugin-opt=legacy-pass-manager -mllvm=-load=%hello_lto_pass_path -mllvm=-hello-test %t.o \
// RUN:   > %t.lld.full.legacypm.log 2>&1
//
// RUN: ld.lld -plugin-opt=new-pass-manager -load-pass-plugin=%hello_lto_pass_path \
// RUN:   -mllvm=-load=%hello_lto_pass_path %t.o \
// RUN:   > %t.lld.full.newpm.log 2>&1
//
// [lld thinLTO]
// RUN: %clang %s -flto=thin -O2 -c -o %t.o
// RUN: ld.lld -plugin-opt=legacy-pass-manager -mllvm=-load=%hello_lto_pass_path -mllvm=-hello-test %t.o -o %t \
// RUN:   > %t.lld.thin.legacypm.log 2>&1
// RUN: ld.lld -plugin-opt=new-pass-manager -load-pass-plugin=%hello_lto_pass_path \
// RUN:   -mllvm=-load=%hello_lto_pass_path %t.o \
// RUN:   > %t.lld.thin.newpm.log 2>&1

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
  test_func(argc);
  return 0;
}
