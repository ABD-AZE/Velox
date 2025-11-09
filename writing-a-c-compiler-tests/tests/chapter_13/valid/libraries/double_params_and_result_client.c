double get_max(double a, double b, double c, double d, double e, double f,
               double g, double h, double i, double j, double k);

int main(void) {
  double result = get_max(100.3, 200.1, 0.01, 1.00004*100000.0, 55.555, -4., 6543.2,
                          9.0*1000000000.0, 8.0*100000000.0, 7.6, 10.0*1000.0 * 1100000.0);
  return result == 10000.0 * 1100000.0;
}