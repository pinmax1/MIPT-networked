#pragma once

#include<stdint.h>
#include <iostream>
#include <vector>

class Bitstream {
public:
    Bitstream() = default;

    template <typename T>
    Bitstream(const T* obj, size_t size) {
        Data_.resize(size * sizeof(uint8_t));

        const uint8_t* obj_bits = reinterpret_cast<const uint8_t*>(obj);
        for (size_t i = 0; i < Data_.size(); ++i) {
            Data_[i] = obj_bits[i];
        }
    }

    template <typename T>
    void Write(const T& obj) {
        size_t old = Data_.size();
        Data_.resize(Data_.size() + sizeof(T));

        const uint8_t* obj_bits = reinterpret_cast<const uint8_t*>(&obj);
        for (size_t i = old; i < Data_.size(); ++i) {
            Data_[i] = obj_bits[i - old];
        }
    }

    template <typename T>
    void Read(T& obj) {
        uint8_t* obj_bits = reinterpret_cast<uint8_t*>(&obj);

        for (size_t i = Offset_; i < Offset_ + sizeof(T); ++i) {
            obj_bits[i - Offset_] = Data_[i];
        }

        Skip<T>();
    }

    template <typename T>
    void Skip() {
        Offset_ += sizeof(T);
    }

    uint8_t* Data() {
        return Data_.data() + Offset_;
    }

    size_t Size() {
        return Data_.size() - Offset_;
    }

private:
    std::vector<uint8_t> Data_;
    size_t Offset_ = 0;
};