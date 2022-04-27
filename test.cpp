#include <stdio.h>
#include <mutex>

#include "noshared.hpp"
using namespace nstd;

void use_noweak_ptr(noweak_ptr<int> w) {
    noshared_ptr<int> s = w.lock();
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

