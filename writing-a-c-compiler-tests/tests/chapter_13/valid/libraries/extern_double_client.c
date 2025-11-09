/* Test linking against a double defined in another file */
extern double d;

int main(void) { return d == 100000000000000000000.0; }