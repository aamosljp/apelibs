/*
 * main.c - Application using the math library
 */

#include <stdio.h>
#include "mathlib.h"

int main(void) {
	printf("Math Library Demo\n");
	printf("=================\n\n");

	/* Basic operations */
	printf("Basic Operations:\n");
	printf("  10 + 3 = %d\n", math_add(10, 3));
	printf("  10 - 3 = %d\n", math_sub(10, 3));
	printf("  10 * 3 = %d\n", math_mul(10, 3));
	printf("  10 / 3 = %d\n", math_div(10, 3));

	printf("\n");

	/* Advanced operations */
	printf("Advanced Operations:\n");
	printf("  2^10 = %d\n", math_pow(2, 10));
	printf("  5! = %d\n", math_factorial(5));
	printf("  gcd(48, 18) = %d\n", math_gcd(48, 18));

	return 0;
}
