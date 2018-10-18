#pragma once

#include <initializer_list>
#include <algorithm>
#include <deque>
#include <cassert>

class Buffer {
public:
    using ValueType = int*;
    using ArrayType = ValueType*;

    Buffer() {
        array_ = CreateArray(1);
        capacity_ = 1;
        size_ = 0;
        first_ = 0;
        last_ = 0;
    }

    Buffer(Buffer&& other) {
        // TODO : dangerous, array_ = nullptr
        array_ = other.array_;
        capacity_ = other.capacity_;
        size_ = other.size_;
        first_ = other.first_;
        last_ = other.last_;
        other.array_ = nullptr;
        other.capacity_ = 0;
        other.size_ = 0;
        other.first_ = 0;
        other.last_ = 0;
    }

    void PushBack(ValueType pointer) {
        assert(capacity_ > 0);
        if (size_ == capacity_) {
            IncreaseCapacity();
        }
        if (size_ > 0) {
            last_ = (last_ + 1) % capacity_;
        } else {
            assert(last_ == 0);
        }
        array_[last_] = pointer;
        ++size_;
    }

    void PushFront(ValueType pointer) {
        assert(capacity_ > 0);
        if (size_ == capacity_) {
            IncreaseCapacity();
        }
        if (size_ > 0) {
            first_ = (first_ + capacity_ - 1) % capacity_;
        } else {
            assert(first_ == 0);
        }
        array_[first_] = pointer;
        ++size_;
    }

    void PopBack() {
        assert(capacity_ > 0);
        assert(size_ > 0);
        array_[last_] = nullptr;
        last_ = (last_ + capacity_ - 1) % capacity_;
        --size_;
        if (size_ == 0) {
            first_ = last_ = 0;
        }
    }

    void PopFront() {
        assert(capacity_ > 0);
        assert(size_ > 0);
        array_[first_] = nullptr;
        first_ = (first_ + 1) % capacity_;
        --size_;
        if (size_ == 0) {
            first_ = last_ = 0;
        }
    }

    ValueType operator[](int index) {
        assert(capacity_ > 0);
        return array_[(first_ + index) % capacity_];
    }

    ValueType operator[](int index) const {
        assert(capacity_ > 0);
        return array_[(first_ + index) % capacity_];
    }

    ValueType Back() {
        return array_[last_];
    }

    ValueType Front() {
        return array_[first_];
    }

    int Size() {
        return size_;
    }

    void Clear() {
        if (capacity_ > 0) {
            for (int i = 0; i < size_; ++i) {
                array_[(first_ + i) % capacity_] = nullptr;
            }
        }
        size_ = 0;
        first_ = 0;
        last_ = 0;
    }

    void Swap(Buffer& other) {
        std::swap(array_, other.array_);
        std::swap(capacity_, other.capacity_);
        std::swap(size_, other.size_);
        std::swap(first_, other.first_);
        std::swap(last_, other.last_);
    }

    ~Buffer() {
        if (array_) {
            delete[] array_;
        }
    }

private:
    // decrease capacity would be nice

    void IncreaseCapacity() {
        assert(size_ > 0);
        auto array = CreateArray(capacity_ * 2);
        for (int i = 0; i < size_; ++i) {
            array[i] = array_[(first_ + i) % capacity_];
        }
        capacity_ *= 2;
        first_ = 0;
        last_ = size_ - 1;
        // size_ remains
        delete[] array_;
        array_ = array;
    }

    ArrayType CreateArray(int size) {
        auto array = new ValueType[size];
        for (int i = 0; i < size; ++i) {
            array[i] = nullptr;
        }
        return array;
    }

    ArrayType array_;
    int capacity_;
    int size_;
    int first_;
    int last_;
};

class Deque {
public:
    static const int kBlockSize = 128;
    static const int kMiddle = 0;
    using Block = int*;

    Deque() {
        size_ = 0;
        first_ = kMiddle;
        last_ = kMiddle;
    }

    Deque(const Deque& rhs) {
        size_ = 0;
        first_ = kMiddle;
        last_ = kMiddle;
        for (int i = 0; i < rhs.size_; ++i) {
            PushBack(rhs[i]);
        }
    }

    Deque(Deque&& rhs) : buffer_(std::move(rhs.buffer_)) {
        // TODO: dangerous, use swap 
        size_ = rhs.size_;
        first_ = rhs.first_;
        last_ = rhs.last_;
        rhs.size_ = 0;
        rhs.first_ = 0;
        rhs.last_ = 0;
    }

    explicit Deque(size_t size) {
        size_ = 0;
        first_ = kMiddle;
        last_ = kMiddle;
        for (size_t i = 0; i < size; ++i) {
            PushBack(0);
        }
    }

    Deque(std::initializer_list<int> list) {
        size_ = 0;
        first_ = kMiddle;
        last_ = kMiddle;
        for (auto it = list.begin(); it != list.end(); ++it) {
            PushBack(*it);
        }
    }

    Deque& operator=(Deque rhs) {
        Swap(rhs);
        return *this;
    }

    void Swap(Deque& rhs) {
        buffer_.Swap(rhs.buffer_);
        std::swap(size_, rhs.size_);
        std::swap(first_, rhs.first_);
        std::swap(last_, rhs.last_);
    }

    void PushBack(int value) {
        if (size_ == 0) {
            auto block = CreateBlock();
            buffer_.PushBack(block);
            size_ = 1;
            first_ = last_ = kMiddle;
            buffer_.Back()[last_] = value;
            return;
        }
        if (last_ + 1 == kBlockSize) {
            auto block = CreateBlock();
            buffer_.PushBack(block);
        }
        last_ = (last_ + 1) % kBlockSize;
        buffer_.Back()[last_] = value;
        ++size_;
    }

    void PopBack() {
        if (size_ == 1) {
            delete[] buffer_.Back();
            buffer_.PopBack();
            first_ = last_ = kMiddle;
            size_ = 0;
            return;
        }
        if (last_ == 0) {
            delete[] buffer_.Back();
            buffer_.PopBack();
        }
        last_ = (last_ + kBlockSize - 1) % kBlockSize;
        --size_;
    }

    void PushFront(int value) {
        if (size_ == 0) {
            auto block = CreateBlock();
            buffer_.PushBack(block);
            size_ = 1;
            first_ = last_ = kMiddle;
            buffer_.Back()[first_] = value;
            return;
        }
        if (first_ == 0) {
            auto block = CreateBlock();
            buffer_.PushFront(block);
        }
        first_ = (first_ + kBlockSize - 1) % kBlockSize;
        buffer_.Front()[first_] = value;
        ++size_;
    }

    void PopFront() {
        if (size_ == 1) {
            delete[] buffer_.Front();
            buffer_.PopFront();
            first_ = last_ = kMiddle;
            size_ = 0;
            return;
        }
        if (first_ + 1 == kBlockSize) {
            delete[] buffer_.Front();
            buffer_.PopFront();
        }
        first_ = (first_ + 1) % kBlockSize;
        --size_;
    }

    int& operator[](size_t ind) {
        return buffer_[(first_ + ind) / kBlockSize][(first_ + ind) % kBlockSize];
    }

    int operator[](size_t ind) const {
        return buffer_[(first_ + ind) / kBlockSize][(first_ + ind) % kBlockSize];
    }

    size_t Size() const {
        return size_;
    }

    void Clear() {
        for (int i = 0; i < buffer_.Size(); ++i) {
            delete[] buffer_[i];
        }
        buffer_.Clear();
        size_ = 0;
        first_ = 0;
        last_ = 0;
    }

    ~Deque() {
        Clear();
    }

private:
    Block CreateBlock() {
        Block block = new int[kBlockSize];
        for (int i = 0; i < kBlockSize; ++i) {
            block[i] = 0;
        }
        return block;
    }

    Buffer buffer_;
    int size_;
    int first_;
    int last_;
};

const int Deque::kBlockSize;
const int Deque::kMiddle;
