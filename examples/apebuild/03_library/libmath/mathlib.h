/*
 * mathlib.h - Simple math library header
 */

#ifndef MATHLIB_H
#define MATHLIB_H

/* Basic operations */
int math_add(int a, int b);
int math_sub(int a, int b);
int math_mul(int a, int b);
int math_div(int a, int b);

/* Advanced operations */
int math_pow(int base, int exp);
int math_factorial(int n);
int math_gcd(int a, int b);

#endif /* MATHLIB_H */
