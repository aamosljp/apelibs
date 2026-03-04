/*
 * main.c - Main entry point for calculator example
 */

#include <stdio.h>
#include "calc.h"

int main(void) {
	printf("Calculator Example\n");
	printf("==================\n\n");

	int a = 10, b = 3;

	printf("%d + %d = %d\n", a, b, calc_add(a, b));
	printf("%d - %d = %d\n", a, b, calc_sub(a, b));
	printf("%d * %d = %d\n", a, b, calc_mul(a, b));
	printf("%d / %d = %d\n", a, b, calc_div(a, b));

	return 0;
}
