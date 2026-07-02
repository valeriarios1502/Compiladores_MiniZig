#include <stdio.h>
#define N 10000000

int main() {
    static int arr[N];
    for (int i = 0; i < N; i++) {
        arr[i] = i % 100;
    }
    long sum = 0;
    for (int i = 0; i < N; i++) {
        sum += arr[i];
    }
    printf("%ld\n", sum);
    return 0;
}
