#include "ptl_graph.h"
#include <cassert>

#include <iostream>
#include "timer.h"

bool test_sat_wolper(const std::string &inp) {
    LTLGraphWolper graph(inp);
    {
        Timer timer("Build");
        graph.build();
    }
    {
        Timer timer("Eliminate");
        graph.eliminate();
    }
    return graph.sat();
}

void test_sat() {
    std::vector<std::string> tests = {
            "a",
            "~a",
            "X a",
            "U a b",
            "& ~F~ F a ~F~ F ~a",
            "U U U U U U U U a b c d e f g h i ",
            "U e U d U c U a b",
            "& & Uac Ubc  & ~F~~a b "
    };

    for (auto &test: tests) {
        auto res = test_sat_wolper(test);
        assert(res == true);
    }
}

void test_unsat() {
    std::vector<std::string> tests = {
            "& ~ a a",
            "& ~ F ~ a ~ F a",
            "F & a ~a",
            "U a & b ~b",
            "& ~ F ~ p F ~ p",
            "& & Uac Ubc  & ~F~~a & b ~c"
    };

    for (auto &test: tests) {
        auto res = test_sat_wolper(test);
        assert(res == false);
    }
}

void all_tests() {
    test_sat();
    test_unsat();
}


void benchmark() {
    std::vector<std::string> tests = {
            "U a b",
            "U c U a b",
            "U d U c U a b",
            "U e U d U c U a b",
            "U f U e U d U c U a b",
            "U g U f U e U d U c U a b",
            "U h U g U f U e U d U c U a b",
            "U i U h U g U f U e U d U c U a b",
    };

    std::vector<std::string> tests2 = {
            "U a b",
            "U U a b c",
            "U U U a b c d",
            "U U U U a b c d e",
            "U U U U U a b c d e f",
            "U U U U U U a b c d e f g",
            "U U U U U U U a b c d e f g h",
            "U U U U U U U U a b c d e f g h i",
            "U U U U U U U U U a b c d e f g h i j",
            "U U U U U U U U U U a b c d e f g h i j k",
            "U U U U U U U U U U U a b c d e f g h i j k l",
//            "U U U U U U U U U U U U a b c d e f g h i j k l m",
//            "U U U U U U U U U U U U U a b c d e f g h i j k l m n",
//            "U U U U U U U U U U U U U U a b c d e f g h i j k l m n o",
    };

    std::vector<std::string> tests3 = {
            "~F~ a",
            "& ~F~ a ~F~ b",
            "& & ~F~ a ~F~ b ~F~ c",
            "& & & ~F~ a ~F~ b ~F~ c ~F~ d",
            "& & & & ~F~ a ~F~ b ~F~ c ~F~ d ~F~ e",
            "& & & & & ~F~ a ~F~ b ~F~ c ~F~ d ~F~ e ~F~ f",
            "& & & & & & ~F~ a ~F~ b ~F~ c ~F~ d ~F~ e ~F~ f ~F~ g",
            "& & & & & & & ~F~ a ~F~ b ~F~ c ~F~ d ~F~ e ~F~ f ~F~ g ~F~ h",
            "& & & & & & & & ~F~ a ~F~ b ~F~ c ~F~ d ~F~ e ~F~ f ~F~ g ~F~ h ~F~ i",
            "& & & & & & & & & ~F~ a ~F~ b ~F~ c ~F~ d ~F~ e ~F~ f ~F~ g ~F~ h ~F~ i ~F~ j",
    };

    for (auto &test: tests2) {
        Timer timer(test);
        test_sat_wolper(test);
    }
}

int main() {
    benchmark();
    all_tests();
    return 0;
}
