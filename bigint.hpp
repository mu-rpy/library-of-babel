#pragma once
#include <cstdint>
#include <cstring>
#include <algorithm>
#include <string>
#include <iostream>



struct BigInt {
    uint32_t d[LIMBS] = {};

    BigInt() { memset(d, 0, sizeof(d)); }

    BigInt(uint64_t v) {
        memset(d, 0, sizeof(d));
        d[0] = (uint32_t)(v & 0xFFFFFFFF);
        d[1] = (uint32_t)(v >> 32);
    }

    explicit BigInt(const std::string& s) {
        memset(d, 0, sizeof(d));
        for (char c : s) {
            uint64_t carry = (c - '0');
            for (int i = 0; i < LIMBS; i++) {
                uint64_t cur = (uint64_t)d[i] * 10 + carry;
                d[i]  = (uint32_t)(cur & 0xFFFFFFFF);
                carry = cur >> 32;
            }
        }
    }

    BigInt operator+(const BigInt& o) const {
        BigInt r;
        uint64_t carry = 0;
        for (int i = 0; i < LIMBS; i++) {
            uint64_t s = (uint64_t)d[i] + o.d[i] + carry;
            r.d[i] = (uint32_t)(s & 0xFFFFFFFF);
            carry  = s >> 32;
        }
        return r;
    }

    BigInt operator-(const BigInt& o) const {
        BigInt r;
        int64_t borrow = 0;
        for (int i = 0; i < LIMBS; i++) {
            int64_t diff = (int64_t)d[i] - o.d[i] - borrow;
            if (diff < 0) { diff += (int64_t)1 << 32; borrow = 1; } else borrow = 0;
            r.d[i] = (uint32_t)(uint64_t)diff;
        }
        return r;
    }

    BigInt operator*(const BigInt& o) const {
        BigInt r;
        for (int i = 0; i < LIMBS; i++) {
            if (!d[i]) continue;
            uint64_t carry = 0;
            int jmax = LIMBS - i;
            for (int j = 0; j < jmax; j++) {
                uint64_t cur = (uint64_t)d[i] * o.d[j] + r.d[i+j] + carry;
                r.d[i+j] = (uint32_t)(cur & 0xFFFFFFFF);
                carry     = cur >> 32;
            }
        }
        return r;
    }

    uint32_t operator%(uint32_t m) const {
        uint64_t rem = 0;
        for (int i = LIMBS - 1; i >= 0; i--)
            rem = ((rem << 32) | d[i]) % m;
        return (uint32_t)rem;
    }

    bool is_zero() const {
        for (int i = 0; i < LIMBS; i++) if (d[i]) return false;
        return true;
    }


    std::string str() const {
        uint32_t tmp[LIMBS];
        memcpy(tmp, d, sizeof(d));
        std::string s;
        bool all_zero = false;
        while (!all_zero) {
            uint64_t rem = 0;
            all_zero = true;
            for (int i = LIMBS-1; i >= 0; i--) {
                uint64_t cur = (rem << 32) | tmp[i];
                tmp[i] = (uint32_t)(cur / 10);
                rem    = cur % 10;
                if (tmp[i]) all_zero = false;
            }
            s += (char)('0' + rem);
        }
        if (s.empty()) s = "0";
        std::reverse(s.begin(), s.end());
        return s;
    }

    BigInt& operator+=(const BigInt& o) { *this = *this + o; return *this; }
    BigInt& operator-=(const BigInt& o) { *this = *this - o; return *this; }
    BigInt& operator*=(const BigInt& o) { *this = *this * o; return *this; }
};

inline std::ostream& operator<<(std::ostream& os, const BigInt& b) { return os << b.str(); }

inline BigInt ext_gcd_inv(const BigInt& a, const BigInt& /*mod*/) {
    BigInt x(1ULL);
    BigInt two(2ULL);
    for (int i = 0; i < 15; i++)
        x = x * (two - a * x);
    return x;
}

inline BigInt parse36(const std::string& s) {
    BigInt result(0ULL);
    BigInt b36(36ULL);
    for (char ch : s) {
        result = result * b36;
        if (ch >= '0' && ch <= '9') result = result + BigInt((uint64_t)(ch - '0'));
        else if (ch >= 'a' && ch <= 'z') result = result + BigInt((uint64_t)(ch - 'a' + 10));
    }
    return result;
}

inline BigInt pow2(int exp) {
    BigInt r(0ULL);
    r.d[exp/32] = (1u << (exp%32));
    return r;
}

inline BigInt mod_pow2(const BigInt& a, int) { return a; }

inline BigInt powmod(BigInt base, BigInt exp, const BigInt& /*mod*/) {
    BigInt result(1ULL);
    while (!exp.is_zero()) {
        if (exp.d[0] & 1u) result = result * base;
        base = base * base;
        for (int i = 0; i < LIMBS-1; i++)
            exp.d[i] = (exp.d[i] >> 1) | (exp.d[i+1] << 31);
        exp.d[LIMBS-1] >>= 1;
    }
    return result;
}
