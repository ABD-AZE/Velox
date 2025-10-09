double get_max(double a, double b, double c, double d, double e, double f,
               double g, double h, double i, double j, double k);

int main(void) {
  double result = get_max(100.3, 200.1, 0.01, 1.000045, 55.555, -4., 6543.2,
                          99, 88, 7.6, 103 * 115);
  return result == 103 * 115;
}