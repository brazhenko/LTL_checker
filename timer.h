#pragma once

#include <iostream>
#include <chrono>
#include <string>

class Timer {
public:
    Timer(const std::string& name) : name(name), start(std::chrono::high_resolution_clock::now()) {}

    ~Timer() {
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration<double, std::milli>(end - start).count();

        std::cout << "> " << name << ": " << duration << " ms" << std::endl;
    }

private:
    std::string name;
    std::chrono::high_resolution_clock::time_point start;
};