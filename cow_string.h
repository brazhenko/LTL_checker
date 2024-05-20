#pragma once

#include <memory>
#include <string>
#include <stdexcept>
#include <iostream>
#include <utility>



class CowString {
public:
    CowString() {}
    CowString(const std::string& str)
            : str_(std::make_shared<std::string>(str)),
              data_(str_->data(), str_->size()) {}

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
        return CowString(str_, std::string_view(data_.data() + pos, len));
    }

    CowString substr(size_t pos) const {
        if (pos > data_.size()) {
            throw std::out_of_range("Out of range");
        }
        return {str_, std::string_view(data_.data() + pos, data_.size() - pos)};
    }

    size_t size() const {
        return data_.size();
    }

    bool empty() const {
        return data_.empty();
    }

    template<class S>
    bool starts_with(S s) const {
        return data_.starts_with(s);
    }

    char at(size_t idx) const {
        return data_.at(idx);
    }

    bool operator==(const CowString &right) const {
        return data_ == right.data_;
    }

    auto operator<=>(const CowString &right) const  {
        return data_ <=> right.data_;
    }

    size_t hash() const {
        return std::hash<std::string_view>{}(data_);
    }

    const char *c_str() const {
        return data_.data();
    }
    std::string get_copy() const {
        return {data_.data(), data_.size()};
    }

    friend std::ostream& operator<<(std::ostream& os, const CowString& str) {
        return os << std::string(str.data_.data(), str.data_.size());
    }

private:
    CowString(std::shared_ptr<std::string> str, std::string_view data)
            : str_(std::move(str)),
              data_(data.data(), data.size()) {}

    std::shared_ptr<std::string> str_;
    std::string_view data_;
};
