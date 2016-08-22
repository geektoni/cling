/**
 * Simple test to check the preprocessor only functionality.
 */

//RUN: %cling -E %s > %t1
//RUN: clang -E -x c++ %s > %t2
//RUN: diff %t1 %t2

int a = 1;