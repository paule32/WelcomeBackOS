// ---------------------------------------------------------------------------
// \file  math.c
// \note  (c) 2025 by Jens Kallup - paule32
//        all rights reserved.
// ---------------------------------------------------------------------------
int k_power(int base,int n)
{
    int i, p;
    
    if (n == 0)
    return 1;
    
    p = 1;
    
    for (i = 1; i <= n; ++i)
    p = p * base;
    
    return p;
}

int k_abs(int i) {
    return i < 0 ? -i : i;
}

double k_sqrt(double n) {
    if (n < 0) return -1; // Fehlerwert für negative Zahlen

    double x = n;
    double last = 0.0;

    while (x != last) {
        last = x;
        x = 0.5 * (x + n / x);
    }
    return x;
}
