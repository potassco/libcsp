#pragma once


template<class T>
T gcd(T a, T b)
{
    while (true)
    {
        T r = a % b;
        if (r == 0)
            break;
        a = b;
        b = r;
    }
    return b;
}
