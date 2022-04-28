#ifndef INC_NOSHARED_HPP_
#define INC_NOSHARED_HPP_
#pragma once

#include <iostream>
#include <memory>

namespace nstd {

enum struct noshared_type {
    kNormal,
    kLocked,
    kWeak
};

template <typename T>
class noshared_ptr { // non-copyable pointer to an object
public:
    using pointer = T*;
    using element_type = T;

    constexpr noshared_ptr() noexcept
        : type_(noshared_type::kNormal)
        , sptr_() {}

    constexpr noshared_ptr(nullptr_t) noexcept
        : type_(noshared_type::kNormal)
        , sptr_() {}

    noshared_ptr &operator=(nullptr_t) noexcept {
        reset();
        return *this;
    }

    explicit noshared_ptr(pointer ptr) noexcept
        : type_(noshared_type::kNormal),
        sptr_(ptr) {}

    noshared_ptr(noshared_ptr &&right) noexcept
        : noshared_ptr(construct(std::move(right)), 0) {}

    template <class T2,
        typename std::enable_if<std::is_convertible<typename noshared_ptr<T2>::pointer, pointer>::value, int>::type = 0>
    noshared_ptr(noshared_ptr<T2> &&right) noexcept
        : noshared_ptr(construct2(std::move(right)), 0) {}

    template <class T2,
        typename std::enable_if<std::is_convertible<typename noshared_ptr<T2>::pointer, pointer>::value, int>::type = 0>
    noshared_ptr &operator=(noshared_ptr<T2> &&right) noexcept {
        noshared_ptr(std::forward<noshared_ptr>(right)).swap(*this);
        return *this;
    }

    noshared_ptr &operator=(noshared_ptr &&right) noexcept {
        noshared_ptr(std::forward<noshared_ptr>(right)).swap(*this);
        return *this;
    }

    void swap(noshared_ptr &right) noexcept {
        std::swap(this->type_, right.type_);
        std::swap(this->wptr_, right.wptr_);
        std::swap(this->sptr_, right.sptr_);
    }

    ~noshared_ptr() noexcept {
    }

    typename std::add_lvalue_reference<T>::type operator*() const /* strengthened */ {
        return *get();
    }

    pointer operator->() const noexcept {
        return get();
    }

    pointer get() const noexcept {
        return (type_ == noshared_type::kWeak ? wptr_.lock() : sptr_).get();
    }

    explicit operator bool() const noexcept {
        return (type_ == noshared_type::kWeak ? wptr_.lock() : sptr_).operator bool();
    }

    void reset(pointer ptr = nullptr) noexcept {
        type_ = noshared_type::kNormal;
        sptr_.reset(ptr);
        wptr_.reset();
    }

    template <typename T2>
    bool owner_before(const noshared_ptr<T2> &right) const noexcept { // compare addresses of manager objects
        const std::weak_ptr<T> &x = (type_ == noshared_type::kWeak ? wptr_ : sptr_);
        const std::weak_ptr<T> &y = (right.type_ == noshared_type::kWeak ? right.wptr_ : right.sptr_);
        return x.owner_before(y);
    }

    noshared_ptr(const noshared_ptr &) = delete;
    noshared_ptr &operator=(const noshared_ptr &) = delete;
private:

    noshared_type type_;
    std::shared_ptr<T> sptr_;
    std::weak_ptr<T> wptr_;

    noshared_ptr(noshared_type type, const std::shared_ptr<T> &sptr, const std::weak_ptr<T> &wptr) noexcept
        : type_(type)
        , sptr_(sptr)
        , wptr_(wptr) {
    }
    
    noshared_ptr(const noshared_ptr &other, int /*dummy */)
        : type_(std::move(other.type_))
        , sptr_(std::move(other.sptr_))
        , wptr_(std::move(other.wptr_)) {
    }

    static noshared_ptr construct(noshared_ptr &&other) {
        if(other.type_ == noshared_type::kNormal) {
            std::shared_ptr<T> sptr = std::move(other.sptr_);
            return noshared_ptr(noshared_type::kNormal, sptr, {});
        }
        else if(other.type_ == noshared_type::kLocked) {
            std::weak_ptr<T> wptr = other.sptr_; // not move sptr_
            return noshared_ptr(noshared_type::kWeak, {}, wptr);
        }
        else /* if(other.type_ == noshared_type::kWeak) */ {
            std::weak_ptr<T> wptr = std::move(other.wptr_);
            return noshared_ptr(noshared_type::kWeak, {}, wptr);
        }
    }  
    
    template<typename T2>
    static noshared_ptr construct2(noshared_ptr<T2> &&other) {
        if(other.type_ == noshared_type::kNormal) {
            std::shared_ptr<T> sptr = std::dynamic_pointer_cast<T>(std::move(other.sptr_));
            return noshared_ptr(noshared_type::kNormal, sptr, {});
        }
        else if(other.type_ == noshared_type::kLocked) {
            std::shared_ptr<T> sptr = std::dynamic_pointer_cast<T>(other.sptr_); // not move sptr_
            std::weak_ptr<T> wptr = sptr;
            return noshared_ptr(noshared_type::kWeak, {}, wptr);
        }
        else /* if(other.type_ == noshared_type::kWeak) */ {
            std::weak_ptr<T> wptr = std::move(other.wptr_);
            return noshared_ptr(noshared_type::kWeak, {}, wptr);
        }
    }  
    

    template <class T2>
    friend class noshared_ptr;
    template <class T2>
    friend class noweak_ptr;

    template<typename T1, typename T2>
    friend noshared_ptr<T1> dynamic_pointer_cast(noshared_ptr<T2> &&other);
};

template<typename T1, typename T2>
noshared_ptr<T1> dynamic_pointer_cast(noshared_ptr<T2> &&other) {
    return noshared_ptr<T1>::construct2(std::move(other));
}


// FUNCTION TEMPLATE make_unique
template <class T, class... Types>
noshared_ptr<T> make_noshared(Types&&... _Args) { // make a noshared_ptr
    return noshared_ptr<T>(new T(std::forward<Types>(_Args)...));
}

template <class T>
void swap(noshared_ptr<T> &left, noshared_ptr<T> &right) noexcept {
    left.swap(right);
}

template <class T1, class T2>
bool operator==(const noshared_ptr<T1> &left, const noshared_ptr<T2> &right) {
    return left.get() == right.get();
}

template <class T1, class T2>
bool operator!=(const noshared_ptr<T1> &left, const noshared_ptr<T2> &right) {
    return !(left == right);
}

template <class T1, class T2>
bool operator<(const noshared_ptr<T1> &left, const noshared_ptr<T2> &right) {
    using Ptr1 = typename noshared_ptr<T1>::pointer;
    using Ptr2 = typename noshared_ptr<T2>::pointer;
    using Common = typename std::common_type<Ptr1, Ptr2>::type;
    return std::less<Common>{}(left.get(), right.get());
}

template <class T1, class T2>
bool operator>=(const noshared_ptr<T1> &left, const noshared_ptr<T2> &right) {
    return !(left < right);
}

template <class T1, class T2>
bool operator>(const noshared_ptr<T1> &left, const noshared_ptr<T2> &right) {
    return right < left;
}

template <class T1, class T2>
bool operator<=(const noshared_ptr<T1> &left, const noshared_ptr<T2> &right) {
    return !(right < left);
}

template <class T>
bool operator==(const noshared_ptr<T> &left, nullptr_t) noexcept {
    return !left;
}

template <class T>
bool operator==(nullptr_t, const noshared_ptr<T> &right) noexcept {
    return !right;
}

template <class T>
bool operator!=(const noshared_ptr<T> &left, nullptr_t right) noexcept {
    return !(left == right);
}

template <class T>
bool operator!=(nullptr_t left, const noshared_ptr<T> &right) noexcept {
    return !(left == right);
}

template <class T>
bool operator<(const noshared_ptr<T> &left, nullptr_t right) {
    using Ptr = typename noshared_ptr<T>::pointer;
    return std::less<Ptr>{}(left.get(), right);
}

template <class T>
bool operator<(nullptr_t left, const noshared_ptr<T> &right) {
    using Ptr = typename noshared_ptr<T>::pointer;
    return std::less<Ptr>{}(left, right.get());
}

template <class T>
bool operator>=(const noshared_ptr<T> &left, nullptr_t right) {
    return !(left < right);
}

template <class T>
bool operator>=(nullptr_t left, const noshared_ptr<T> &right) {
    return !(left < right);
}

template <class T>
bool operator>(const noshared_ptr<T> &left, nullptr_t right) {
    return right < left;
}

template <class T>
bool operator>(nullptr_t left, const noshared_ptr<T> &right) {
    return right < left;
}

template <class T>
bool operator<=(const noshared_ptr<T> &left, nullptr_t right) {
    return !(right < left);
}

template <class T>
bool operator<=(nullptr_t left, const noshared_ptr<T> &right) {
    return !(right < left);
}

// noweak_ptr

// CLASS TEMPLATE noweak_ptr
template <class T>
class noweak_ptr {
public:
    constexpr noweak_ptr() noexcept 
        : wptr_() {
    }

    noweak_ptr(const noweak_ptr &other) noexcept 
        : wptr_(other.wptr_) {
    }

    template <class T2, typename std::enable_if<std::is_convertible<T2 *, T *>::value, int>::type = 0>
    noweak_ptr(const noshared_ptr<T2> &other) noexcept
        : wptr_(other.type_ == noshared_type::kWeak ? other.wptr_ : other.sptr_) {
    }

    template <class T2, typename std::enable_if<std::is_convertible<T2 *, T *>::value, int>::type = 0>
    noweak_ptr(const noweak_ptr<T2> &other) noexcept
        : wptr_(other.wptr_) {
    }

    noweak_ptr(noweak_ptr &&other) noexcept
        : wptr_(std::move(other.wptr_)) {
    }

    template <class T2, typename std::enable_if<std::is_convertible<T2 *, T *>::value, int>::type = 0>
    noweak_ptr(noweak_ptr<T2> &&other) noexcept
        : wptr_(std::move(other.wptr_)) {
    }

    ~noweak_ptr() noexcept {
    }

    noweak_ptr &operator=(const noweak_ptr &right) noexcept {
        noweak_ptr(right).swap(*this);
        return *this;
    }

    template <class T2>
    noweak_ptr &operator=(const noweak_ptr<T2> &right) noexcept {
        noweak_ptr(right).swap(*this);
        return *this;
    }

    noweak_ptr &operator=(noweak_ptr &&right) noexcept {
        noweak_ptr(std::move(right)).swap(*this);
        return *this;
    }

    template <class T2>
    noweak_ptr &operator=(noweak_ptr<T2> &&right) noexcept {
        noweak_ptr(std::move(right)).swap(*this);
        return *this;
    }

    template <class T2>
    noweak_ptr &operator=(const noshared_ptr<T2> &right) noexcept {
        noweak_ptr(right).swap(*this);
        return *this;
    }

    void reset() noexcept { // release resource, convert to null noweak_ptr object
        noweak_ptr{}.swap(*this);
    }

    void swap(noweak_ptr &other) noexcept {
        wptr_.swap(other.wptr_);
    }

    bool expired() const noexcept {
        return wptr_.expired();
    }

    noshared_ptr<T> lock() const noexcept { // convert to shared_ptr
        return noshared_ptr<T>(noshared_type::kLocked, wptr_.lock(), {});
    }

    template <typename T2>
    bool owner_before(const noweak_ptr<T2> &right) const noexcept { // compare addresses of manager objects
        return wptr_.owner_before(right.wptr_);
    }
private:
    std::weak_ptr<T> wptr_;
    template <class T2>
    friend class noshared_ptr;
    template <class T2>
    friend class noweak_ptr;
};


template <class Elem, class Traits, class T>
std::basic_ostream<Elem, Traits> &operator<<(std::basic_ostream<Elem, Traits> &out, const noshared_ptr<T> &ptr) {
    // write contained pointer to stream
    return out << ptr.get();
}

template <class Elem, class Traits, class T>
std::basic_ostream<Elem, Traits> &operator<<(std::basic_ostream<Elem, Traits> &out, const noweak_ptr<T> &ptr) {
    // write contained pointer to stream
    return out << ptr.lock().get();
}

// Alias ...

template<typename T>
using unique_ptr = noshared_ptr<T>;

template<typename T>
using observer_ptr = noweak_ptr<T>;

// FUNCTION TEMPLATE make_unique
template <class T, class... Types>
unique_ptr<T> make_unique(Types&&... _Args) { // make a noshared_ptr
    return unique_ptr<T>(new T(std::forward<Types>(_Args)...));
}

}

#endif
