/*
 * advanced.c - Advanced math operations
 */

#include "mathlib.h"

int math_pow(int base, int exp)
{
	if (exp < 0)
		return 0;

	int result = 1;
	for (int i = 0; i < exp; i++) {
		result *= base;
	}
	return result;
}

int math_factorial(int n)
{
	if (n < 0)
		return 0;
	if (n <= 1)
		return 1;

	int result = 1;
	for (int i = 2; i <= n; i++) {
		result *= i;
	}
	return result;
}

int math_gcd(int a, int b)
{
	if (a < 0)
		a = -a;
	if (b < 0)
		b = -b;

	while (b != 0) {
		int t = b;
		b = a % b;
		a = t;
	}
	return a;
}
