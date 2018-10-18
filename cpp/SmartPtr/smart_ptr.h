//#pragma once

// counter.weak_count = (weak pointers amount) + (counter.shared_count > 0 ? 1 : 0)
// everything is non atomic

#include <string>
#include <cassert>

using String = std::string;

struct ControlBlock {
    int shared_count;
    int weak_count;
    String* data;

    ControlBlock() : shared_count(0), weak_count(0), data(nullptr) {
    }

    ControlBlock(int shcnt, int wcnt) : shared_count(shcnt), weak_count(wcnt), data(nullptr) {
    }

    ~ControlBlock() {
        assert(shared_count == 0);
        assert(weak_count == 0);
        assert(data == nullptr);
    }
};

class WeakPtr;

class SharedPtr {
public:
    friend class WeakPtr;

    void Reset(String* str) {
        Clean();
        counter_ = new ControlBlock(1, 1);
        counter_->data = str;
    }

    String* Get() {
        return counter_->data;
    }

    String& operator*() {
        return *Get();
    }

    SharedPtr(const WeakPtr& rhs);

    SharedPtr() {
        counter_ = new ControlBlock(1, 1);
    }

    SharedPtr(String* str) : SharedPtr() {
        counter_->data = str;
    }

    SharedPtr(const SharedPtr& other) {
        counter_ = other.counter_;
        ++counter_->shared_count;
    }

    SharedPtr(SharedPtr&& other) {
        counter_ = other.counter_;
        other.counter_ = new ControlBlock(1, 1);
    }

    SharedPtr& operator=(const SharedPtr& other) {
        if (&other == this) {
            return *this;
        }
        Clean();
        counter_ = other.counter_;
        ++counter_->shared_count;
        return *this;
    }

    SharedPtr& operator=(SharedPtr&& other) {
        if (&other == this) {
            return *this;
        }
        Clean();
        counter_ = other.counter_;
        other.counter_ = new ControlBlock(1, 1);
        return *this;
    }

    ~SharedPtr() {
        Clean();
    }

private:
    void Clean() {
        assert(counter_ != nullptr);
        assert(counter_->shared_count > 0);
        --counter_->shared_count;
        if (counter_->shared_count == 0) {
            assert(counter_->weak_count > 0);
            if (counter_->data) {
                delete counter_->data;
                counter_->data = nullptr;
            }
            --counter_->weak_count;
            if (counter_->weak_count == 0) {
                delete counter_;
            }
        }
    }

    ControlBlock* counter_;
};

class WeakPtr {
public:
    friend class SharedPtr;

    bool IsExpired() {
        assert(counter_ != nullptr);
        return counter_->shared_count == 0;
    }

    SharedPtr Lock() {
        return SharedPtr(*this);
    }

    WeakPtr() {
        counter_ = new ControlBlock(0, 1);
    }

    WeakPtr(const SharedPtr& sptr) {
        counter_ = sptr.counter_;
        ++counter_->weak_count;
    }

    WeakPtr(const WeakPtr& other) {
        counter_ = other.counter_;
        ++counter_->weak_count;
    }

    WeakPtr(WeakPtr&& other) {
        counter_ = other.counter_;
        other.counter_ = new ControlBlock(0, 1);
    }

    WeakPtr& operator=(const WeakPtr& other) {
        if (&other == this) {
            return *this;
        }
        Clean();
        counter_ = other.counter_;
        ++counter_->weak_count;
        return *this;
    }

    WeakPtr& operator=(WeakPtr&& other) {
        if (&other == this) {
            return *this;
        }
        Clean();
        counter_ = other.counter_;
        other.counter_ = new ControlBlock(0, 1);
        return *this;
    }

    ~WeakPtr() {
        Clean();
    }

private:
    void Clean() {
        assert(counter_->weak_count > 0);
        assert(counter_->shared_count >= 0);
        --counter_->weak_count;
        if (counter_->weak_count == 0) {
            assert(counter_->shared_count == 0);
            delete counter_;
        }
    }

    ControlBlock* counter_;
};

SharedPtr::SharedPtr(const WeakPtr& rhs) {
    counter_ = rhs.counter_;
    assert(counter_->weak_count > 0);
    ++counter_->shared_count;
    if (counter_->shared_count == 1) {
        ++counter_->weak_count;
    }
}
