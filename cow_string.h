#pragma once

#include <memory>
#include <string>
#include <stdexcept>
#include <iostream>

class StringData {
public:
    StringData(const char* data, size_t size)
            : data_(data),
              size_(size) {}

    const char* data() const {
        return data_;
    }

    size_t size() const {
        return size_;
    }

    bool empty() const {
        return size_ == 0;
    }

private:
    const char* data_;
    size_t size_;
};

class CowString {
public:
    CowString(const std::string& str)
            : str_(std::make_shared<std::string>(str)),
              data_(str_.get()->data(), str_.get()->size()) {}

    CowString(const CowString& other)
            : str_(other.str_),
              data_(other.data_.data(), other.data_.size()) {}

    CowString& operator=(const CowString& other) {
        if(&other != this) {
            str_ = other.str_;
            data_ = other.data_;
        }
        return *this;
    }

    CowString substr(size_t pos, size_t len) const {
        if (pos + len > data_.size()) {
            throw std::out_of_range("Out of range");
        }
        return CowString(str_, StringData(data_.data() + pos, len));
    }

    CowString substr(size_t pos) const {
        if (pos > data_.size()) {
            throw std::out_of_range("Out of range");
        }
        return CowString(str_, StringData(data_.data() + pos, data_.size() - pos));
    }

    size_t size() const {
        return data_.size();
    }

    bool empty() const {
        return data_.empty();
    }

    friend std::ostream& operator<<(std::ostream& os, const CowString& str) {
        return os << std::string(str.data_.data(), str.data_.size());
    }

private:
    CowString(std::shared_ptr<std::string> str, StringData data)
            : str_(str),
              data_(data.data(), data.size()) {}

    std::shared_ptr<std::string> str_;
    StringData data_;
};
