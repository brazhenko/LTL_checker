#pragma once


#include <cstddef>
#include "cow_string.h"

struct Lexer {
    Lexer() = delete;

    explicit Lexer(const CowString &string) : cow(string) {}

    CowString cow;
    size_t pos_ = 0;
    std::pair<char, size_t> get() {
        auto res = cow.at(pos_);
        auto pos = pos_;

        pos_++;

        return {res, pos};
    }
};