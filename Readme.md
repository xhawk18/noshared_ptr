什么是 noshared_ptr ?
=============

[(english version)](Readme.en-US.md)

noshared_ptr 类似于 std::shared_ptr，但它是不可复制的。
某种程度上，它更像一个支持weak_ptr功能的std::unique_ptr.

<b><i>"当我使用了这个库，90% c++ 内存问题消失了！"</i></b> <a href="https://github.com/xhawk18/noshared_ptr">- by TreeNewBee</a>

<b><i>"不要用shared_ptr分享一个指针，分享请用weak_ptr !"</i></b> <a href="https://github.com/xhawk18/noshared_ptr">- by NewBeeGui</a>

------------

## 为什么需要noshared_ptr

1. shared_ptr 的问题

    shared_ptr 在无疑能正确表达共享所有权的概念。然而正确的概念，并不一定天然带来正确的结果。
    一个项目，往往立项之初，对象的所有权会设计的很好；随着项目的推进，或许几个月后、或许增加一些人手、或许仅仅是作者的遗忘，这些所有权会逐渐的不明朗起来。强力的代码管理和评审可以延缓这种腐化的过程，实际最终腐化是无可避免的。最终，大家会花费无数的精力，用来解决对象所有权不释放的bug。

    对象所有权不释放的bug，通常很棘手，甚至比野指针bug更难定位和查找。所以，shared_ptr肯定是不适合大型项目的。

    那么，如果不用shared_ptr，始终采用unique_ptr，是否可行呢？也有问题。

2. unique_ptr 的问题

    unique_ptr 表达所有权概念，无疑是明晰并且具体的。但是如果分享给第三方使用，只能用裸指针分享，这种分享极其危险，因为无法检查指针是否有效。
    
    事实上，如果有个与unique_ptr配合的weak_ptr用作分享，事情就迎刃而解。这正式是本noshared_ptr诞生的初衷。

3. noshared_ptr 的改进

    noshared_ptr 借鉴 rust 关于所有权的理念，始终保持对象只有唯一的所有者。共享所有权这件事，在这里是不存的，代之以临时分享所有权。

    用 noshared_ptr 可以构造一份 noweak_ptr，用作安全的分享。特别的，noweak_ptr可以执行lock函数，用于临时租借到对象的所有权。
    代码有特别精巧的设定，使得租借到的所有权，跳出变量作用域时即自动销毁，无法用来产生更多的对象所有者。

    简而言之，noshared_ptr 是一种新的智能指针，是除了 “裸指针”，unique_ptr，shared_ptr 外的更好选择。本人的代码换成 noshared_ptr 后，一些不时浮现的内存和对象析构问题，彻底的消失了。仅此小小的改变，节约了无数的脑细胞和奋斗的夜晚。

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
| 支持weak_ptr？ | :heavy_check_mark:                        | :heavy_check_mark: |

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
