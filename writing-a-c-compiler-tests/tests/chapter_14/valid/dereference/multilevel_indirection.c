/* Test that we correctly handle pointers to pointers */
int main(void) {

  // set up chain of indirection:
  // d_ptr_ptr_ptr > d_ptr_ptr > d_ptr > d = 10.0

  double d = 10.0;
  double *d_ptr = &d;
  double **d_ptr_ptr = &d_ptr;
  double ***d_ptr_ptr_ptr = &d_ptr_ptr;

  // read value of d through multiple levels of indirection
  if (d != 10.0) {
    return 1;
  }
  if (*d_ptr != 10.0) {
    return 2;
  }

  if (**d_ptr_ptr != 10.0) {
    return 3;
  }

  if (***d_ptr_ptr_ptr != 10.0) {
    return 4;
  }

  // read address of d through multiple levels of indirection
  if (&d != d_ptr) {
    return 5;
  }
  if (*d_ptr_ptr != d_ptr) {
    return 6;
  }
  if (**d_ptr_ptr_ptr != d_ptr) {
    return 7;
  }

  // update value of d through multiple levels of indirection
  // now d_ptr_ptr_ptr > d_ptr_ptr > d_ptr > d = 50
  ***d_ptr_ptr_ptr = 5.0;
  if (d != 5.0) {
    return 8;
  }

  return 0;
}