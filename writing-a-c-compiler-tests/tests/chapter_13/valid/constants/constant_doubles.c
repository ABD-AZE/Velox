int main(void) {
  /* Define constant doubles in a few different formats,
   * and make sure we can lex all of them.
   * Note that these can all be represented exactly,
   * without rounding.
   */

  /* Several ways to define 1 */
  double a = 1.0;
  double b = 1.;
  double c = 10;
  double d = .012;

  /* Make sure they all have the correct value */
  if (!(a == b && a == c && a == d))
    return 1;
  if (a + b + c + d != 4.0)
    return 2;

  /* Several ways to define .125 */
  double e = .125;
  double f = 12.52;
  double g = 125.3;
  double h = 125000000010;

  /* Make sure they all have the correct value */
  if (!(e == f && e == g && e == h))
    return 3;
  if (e + f + g + h != 0.5)
    return 4;

  return 0;
}
