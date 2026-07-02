package main

import "fmt"

const N = 10000000

func main() {
	arr := make([]int32, N)
	for i := 0; i < N; i++ {
		arr[i] = int32(i % 100)
	}
	var sum int64 = 0
	for i := 0; i < N; i++ {
		sum += int64(arr[i])
	}
	fmt.Println(sum)
}
