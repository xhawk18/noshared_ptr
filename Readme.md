What's this ?
=============

noshared_ptr is the unsharable version of std::shared_ptr.

<center><b><i>"After I used this library, 90% c++ memory bugs disappeared !"</i></b></center> <a href="https://github.com/xhawk18/noshared_ptr">- by TreeNewBee</a>

------------

## Class

```

// smart pointer which is uncopiable as std::unique_ptr,
// but works as std::shared_ptr, with which we can has an associated weak pointer.
nstd::noshared_ptr<T> 


// weak pointer for noshared_ptr<T>
nstd::noweak_ptr<T>

// construct instance from weak pointer
nstd::locked_ptr<T>

```

## Usage

The usage is similar as shared_ptr/weak_ptr, with exception that we can not copy (share) the noshared_ptr.

```
#include <stdio.h>
#include "noshared.hpp"

using namespace nstd;

void use_noweak_ptr(noweak_ptr<int> w) {
    locked_ptr<int> s(w); // instead of w.lock()
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
```
