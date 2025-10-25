int idx = 3;
int main(void) {
  int arr[5] = {1, 2, 3, 4, 5};
  int *ptr = arr + 1;
  int result = ++ptr--[idx];
  if (result != 6) {
    return 1;
  }
  if (*ptr != 1) {
    return 2;
  }
  if (ptr != arr) {
    return 3;
  }
  if (*ptr++ != 1) {
    return 4;
  }
  if (*ptr != 2) {
    return 5;
  }
  for (int i = 0; i < 4; i++) {
    if (arr[i] != i + 1) {
      return 6;
    }
  }
  if (arr[4] != 6) {
    return 7;
  }
  return 0;
}
