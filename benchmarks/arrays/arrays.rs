fn main() {
    const N: usize = 10_000_000;
    let mut arr = vec![0i32; N];
    for i in 0..N {
        arr[i] = (i % 100) as i32;
    }
    let mut sum: i64 = 0;
    for i in 0..N {
        sum += arr[i] as i64;
    }
    println!("{}", sum);
}
