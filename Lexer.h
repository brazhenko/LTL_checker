#pragma once


struct Lexer {
    Lexer() = delete;

    explicit Lexer(const char *data) : data_(data) {}

    const char *data_;

    bool end() const {
        return *data_ == 0;
    }

    char get() {
        auto ret = *data_;
        data_++;

        if (ret == ' ' || ret == '\n') {
            return get();
        }

        return ret;
    }
};