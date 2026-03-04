/*
 * main.c - Simple C program to be compiled to WebAssembly
 *
 * This program demonstrates a basic Emscripten-compatible C application.
 * It prints to stdout (which gets redirected to the HTML output area)
 * and exports a function callable from JavaScript.
 */

#include <stdio.h>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

/* A function exported to JavaScript via EXPORTED_FUNCTIONS */
#ifdef __EMSCRIPTEN__
EMSCRIPTEN_KEEPALIVE
#endif
int add(int a, int b) {
	int result = a + b;
	printf("add(%d, %d) = %d\n", a, b, result);
	return result;
}

int main(void) {
	printf("Hello from WebAssembly!\n");
	printf("This program was compiled with Emscripten.\n");
	printf("2 + 3 = %d\n", add(2, 3));
	return 0;
}
