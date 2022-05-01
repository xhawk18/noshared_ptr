What's this ?
=============

[中文版文档 (chinese version)](Readme.zh-CN.md)

noshared_ptr is the unsharable version of std::shared_ptr.

<b><i>"After I used this library, 90% c++ memory bugs disappeared !"</i></b> <a href="https://github.com/xhawk18/noshared_ptr">- by TreeNewBee</a>

<b><i>"Do not share a pointer by shared_ptr, share it by weak_ptr !"</i></b> <a href="https://github.com/xhawk18/noshared_ptr">- by NewBeeGui</a>

------------

## Class

```

// smart pointer which is uncopiable as std::unique_ptr,
// but works as std::shared_ptr, with which we can has an associated weak pointer.
nstd::noshared_ptr<T> 

// weak pointer for noshared_ptr<T>
nstd::noweak_ptr<T>

// Alias names of the two smart pointer class above
nstd::unique_ptr<T>    // alias of nstd::noshared_ptr<T>
nstd::observer_ptr<T>  // alias of nstd::noweak_ptr<T>

```

## Usage

The usage is similar as shared_ptr/weak_ptr, with some exceptions --

1. noshared_ptr is not copiable.
   
   so the code can not be compiled

```
    unique_ptr<int> s = make_unique<int>(8);
    unique_ptr<int> s2 = s;  // error, can not compile
```

   instead, we should use

```
    unique_ptr<int> s2 = std::move(s);
    // then s will be empty, and s2 take the object's ownership
```

2. the pointer returned by noweak_ptr::lock() can not be moved.

```
    observer_ptr<int> w = s2;
    unique_ptr<int> s3 = w.lock();
    unique_ptr<int> s4 = std::move(s3);
    // s3 will not keep old value even after std::move
    // s4 will be a virtual pointer that do not take the object's ownership.
```

## Example

```
#include <stdio.h>
#include "noshared.hpp"

using namespace nstd;

void use_observer_ptr(observer_ptr<int> w) {
    unique_ptr<int> s = w.lock();
    printf("get value = %d\n", *s);
}


int main() {
    unique_ptr<int> s = make_unique<int>(5);
    printf("create value = %d\n", *s);

    // can not copy it!
    // unique_ptr<int> s2 = s;

    observer_ptr<int> w = s;
    use_observer_ptr(w);
    
    return 0;
}
```
