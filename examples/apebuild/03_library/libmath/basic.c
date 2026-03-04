/*
 * basic.c - Basic math operations
 */

#include "mathlib.h"

int math_add(int a, int b) { return a + b; }

int math_sub(int a, int b) { return a - b; }

int math_mul(int a, int b) { return a * b; }

int math_div(int a, int b) {
	if (b == 0) return 0;
	return a / b;
}
