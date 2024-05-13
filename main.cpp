#include "ptl_graph.h"
#include <cassert>

#include <iostream>

void test_get_left_right_operands() {
    {
        auto [left, right] = get_left_right_operands("&ab");
        assert(left == "a");
        assert(right == "b");
    }
}

void all_tests() {
    test_get_left_right_operands();
}

int main() {
    test_get_left_right_operands();

    auto f1 = "& ~F ~F a  ~F ~F ~a";
    auto f2 = "& ~ F ~ p F ~ p";

    PtlGraph graph(f2);


    graph.build();
    graph.eliminate();
    \

    return 0;
}
