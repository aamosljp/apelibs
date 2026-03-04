/*
 * calc.c - Calculator module implementation
 */

#include "calc.h"

int calc_add(int a, int b) { return a + b; }

int calc_sub(int a, int b) { return a - b; }

int calc_mul(int a, int b) { return a * b; }

int calc_div(int a, int b) {
	if (b == 0) return 0; /* Avoid division by zero */
	return a / b;
}
