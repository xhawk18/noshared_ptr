#include <stdio.h>
#include <mutex>

// define NSTD_USE_LOCKED_PTR to enable lock function for weak pointer
// #define NSTD_USE_LOCKED_PTR
#include "noshared.hpp"
using namespace nstd;

void use_noweak_ptr(noweak_ptr<int> w) {

    // By default w.lock() is disabled, please use locked_ptr instread.
    // To enable w.lock(), we need "#define NSTD_USE_LOCKED_PTR"
    // noshared_ptr<int> s = w.lock();

    locked_ptr<int> s(w);

    printf("get value = %d\n", *s);
}


int main() {
    noshared_ptr<int> s = make_noshared<int>(5);
    printf("create value = %d\n", *s);

    // can not copy it!
    // noshared_ptr<int> s2 = s;

    noweak_ptr<int> w = s;
    use_noweak_ptr(w);
    
    return 0;
}

