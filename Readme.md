什么是 noshared_ptr ?
=============

[(english version)](Readme.en-US.md)

noshared_ptr 类似于 std::shared_ptr，但它是不可复制的。
某种程度上，它更像一个支持weak_ptr功能的std::unique_ptr.

<b><i>"当我使用了这个库，90% c++ 内存问题消失了！"</i></b> <a href="https://github.com/xhawk18/noshared_ptr">- by TreeNewBee</a>

<b><i>"不要用shared_ptr分享一个指针，分享请用weak_ptr !"</i></b> <a href="https://github.com/xhawk18/noshared_ptr">- by NewBeeGui</a>

------------

## 一些比较

### noshared_ptr 和 shared_ptr, unique_ptr 比较

|                | noshared_ptr       | shared_ptr         | unique_ptr         |
|----------------|--------------------|--------------------|--------------------|
| 可以复制？     | :x:                | :heavy_check_mark: | :x:                |
| 可以move？     | :heavy_check_mark: | :heavy_check_mark: | :heavy_check_mark: |
| 支持weak_ptr？ | :heavy_check_mark: | :heavy_check_mark: | :x:                |

### noweak_ptr 和 weak_ptr 比较

|             | noweak_ptr         | weak_ptr           |
|-------------|--------------------|--------------------|
| 可以lock ？ | :heavy_check_mark: | :heavy_check_mark: |

### lock() 返回的指针 比较

|                | noweak_ptr::lock()                        | weak_ptr::lock()   |
|----------------|-------------------------------------------|--------------------|
| 可以复制？     | :x:                                       | :heavy_check_mark: |
| 可以move？     | :exclamation:[说明](#noweak_ptr_lock) | :heavy_check_mark: |
| 支持weak_ptr？ | :heavy_check_mark:                        | :x:                |

## Class 和 API

```

// 智能指针，它是不可复制的。该指针类似于 std::shared_ptr，可以有个关联的 weak引用指针。
nstd::noshared_ptr<T> 

// noshared_ptr的weak引用指针
nstd::noweak_ptr<T>

// 创建一个智能指针
noshared_ptr<T> nstd::make_noshared(Args...)


// 以上类和函数的别名（效果和以上类/函数完全等价）
nstd::unique_ptr<T>         // nstd::noshared_ptr<T> 的别名
nstd::observer_ptr<T>       // nstd::noweak_ptr<T> 的别名
nstd::make_unique(Args...)  // nstd::make_noshared() 函数的别名
```

## 用法

本智能指针的用法和 shared_ptr/weak_ptr 大致相同，除了一些例外情况 --

1. noshared_ptr 不可以复制.
   
   以下代码没法通过编译

```
    noshared_ptr<int> s = make_noshared<int>(8);
    noshared_ptr<int> s2 = s;  // error, can not compile
```

   我们可以用 move 操作代替

```
    noshared_ptr<int> s2 = std::move(s);
    // 这是 s 将是空的，s2将获得对象的所有权
```

2. <a id="noweak_ptr_lock">noweak_ptr::lock() 返回的 noshared_ptr 不可以 move</a>

```
    noweak_ptr<int> w = s2;
    noshared_ptr<int> s3 = w.lock();
    noshared_ptr<int> s4 = std::move(s3);
    // lock()返回的 noshared_ptr，我们做了特别设计，它不同于直接创建的 noshared_ptr，两点不同
    //   1) 这里 std::move虽然不会编译报错，但是实际上 s3 不会有任何改动，仍保持对象的所有权
    //   2) s4 可以用来访问对象，但不拥有对象的所有权
```

以上两点，保障了

1. 初始创建的 noshared_ptr，具有对象的唯一所有权
2. lock()返回的 noshared_ptr，具有对象的临时所有权。该所有权出了变量的花括号作用域即失效。并且不可被传播出去（仅限于花括号作用域的范围）。


## 例子

```
// 这个例子用别名  nstd::unique_ptr/nstd::observer_ptr 实现的
// 替换成  nstd::noshared_ptr/nstd::noweak_ptr 效果是一样的。

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
