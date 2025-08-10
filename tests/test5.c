// single line comment
int a // after code comment
/* block comment */
a += 1
/* block
   with
   newlines */
b >>= 2  // although you don't emit >>= yet, keep shifts you do: >> and then = should recover nicely
// slash vs operator:
x = y / z
/* a */ /=
/* b */ /  /* single slash */
