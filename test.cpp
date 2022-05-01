/// @file: noshared.hpp
/// @author: xhawk18
#include <stdio.h>
#include <list>
#include "noshared.hpp"

using namespace nstd;
struct T {
    T(int v) : v_(v) {
        printf("in %s\n", __FUNCTION__);
    }
    ~T() {
        printf("in %s\n", __FUNCTION__);
    }
    int v_;
};

void use_observer_ptr(observer_ptr<T> w) {
    // s will be freed at the end of scope
    unique_ptr<T> s = w.lock();
    printf("get value = %d\n", s->v_);

    // ptr from lock() is unmovable,
    // s2 can be used, but do not have the ownership of object T here
    unique_ptr<T> *s2 = new unique_ptr<T>(std::move(s));
    printf("check value = %d %d\n", s->v_, (*s2)->v_);
}


void test1() {
    unique_ptr<T> s = make_unique<T>(5);
    printf("create value = %d\n", s->v_);

    // can not copy it!
    observer_ptr<T> w = s;

    unique_ptr<T> s2 = std::move(s);
    printf("check value = %p %p\n", s.get(), s2.get());

    use_observer_ptr(w);

}

void test2() {
    // create smart pointer
    noshared_ptr<int> s = make_noshared<int>(18);
    noweak_ptr<int> w = s;
    
    // put smart pointer to the list
    std::list<noshared_ptr<int>> lists;
    lists.push_back(std::move(s));
    
    // remove smart pointer from list by weak ptr
    lists.remove(w.lock());
    
    printf("lists.length = %d\n", (int)lists.size());
}

int main() {
    test1();
    test2();
    return 0;
}
