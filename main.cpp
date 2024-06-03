#include "ptl_graph.h"
#include <cassert>
#include <iostream>
#include "timer.h"

int PtlNode::node_count = 0;


bool test_sat_wolper(const std::string &inp) {
    LTLGraphWolper graph(inp);
    {
//        Timer timer(inp);
        {
//            Timer timer("Build " + inp);
            graph.build();
        }


        {
            Timer timer("Eliminate " + inp);
            graph.eliminate();
        }
    }

    return graph.sat();
}


void test_sat() {
    std::vector<std::string> tests = {
            "a",
            "~a",
            "X a",
            "U a b",
            "& F F a F F ~a",
            "U U U U U U U U a b c d e f g h i ",
            "U e U d U c U a b",
            "& & Uac Ubc  & F~a b "
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
//            "& & Uac Ubc  & F~a & b ~c"
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

    std::vector<std::string> tests1 = {
            "F a",
            "& F a F b",
            "& & F a F b F c",
            "& & & F a F b F c F d",
            "& & & & F a F b F c F d F e",
            "& & & & & F a F b F c F d F e F f",
            "& & & & & & F a F b F c F d F e F f F g",
            "& & & & & & & F a F b F c F d F e F f F g F h",
            "& & & & & & & & F a F b F c F d F e F f F g F h F i",
            "& & & & & & & & & F a F b F c F d F e F f F g F h F i F j",
    };

    std::vector<std::string> tests2 = {
            "~F~ a",
            "& ~F~ a ~F~ b",
            "& & ~F~ a ~F~ b ~F~ c",
            "& & & ~F~ a ~F~ b ~F~ c ~F~ d",
            "& & & & ~F~ a ~F~ b ~F~ c ~F~ d ~F~ e",
            "& & & & & ~F~ a ~F~ b ~F~ c ~F~ d ~F~ e ~F~ f" ,
            "& & & & & ~F~ a ~F~ b ~F~ c ~F~ d ~F~ e ~F~ f  ~F~ g",
            "& & & & & & ~F~ a ~F~ b ~F~ c ~F~ d ~F~ e ~F~ f ~F~ g ~F~ h",
            "& & & & & & & ~F~ a ~F~ b ~F~ c ~F~ d ~F~ e ~F~ f ~F~ g ~F~  h ~F~ i ",
            "& & & & & & & & ~F~ a ~F~ b ~F~ c ~F~ d ~F~ e ~F~ f ~F~ g ~F~ h ~F~ i ~F~ j ",
    };

    std::vector<std::string> tests3 = {
            "a",
            "U a b",
            "U U a b c",
            "U U U a b c d",
            "U U U U a b c d e",
            "U U U U U a b c d e f",
            "U U U U U U a b c d e f g",
            "U U U U U U U a b c d e f g h",
            "U U U U U U U U a b c d e f g h i",
            "U U U U U U U U U a b c d e f g h i j",
    };

    std::vector<std::string> tests4 = {
            "a",
            "U a b",
            "U c U a b",
            "U d U c U a b",
            "U e U d U c U a b",
            "U f U e U d U c U a b",
            "U g U f U e U d U c U a b",
            "U h U g U f U e U d U c U a b",
            "U i U h U g U f U e U d U c U a b",
            "U j U i U h U g U f U e U d U c U a b",
    };

    std::vector<std::string> tests5 = {
            "~F~F a",
            "| ~F~F a ~F~F b",
            "| | ~F~F a ~F~F b ~F~F c",
            "| | | ~F~F a ~F~F b ~F~F c ~F~F d",
            "| | | | ~F~F a ~F~F b ~F~F c ~F~F d ~F~F e",
            "| | | | | ~F~F a ~F~F b ~F~F c ~F~F d ~F~F e ~F~F f ",
            "| | | | | | ~F~F a ~F~F b ~F~F c ~F~F d ~F~F e ~F~F f ~F~F g",
            "| | | | | | | ~F~F a ~F~F b ~F~F c ~F~F d ~F~F e ~F~F f ~F~F g ~F~F h",
            "| | | | | | | | ~F~F a ~F~F b ~F~F c ~F~F d ~F~F e ~F~F f ~F~F g ~F~F h ~F~F i",
            "| | | | | | | | | ~F~F a ~F~F b ~F~F c ~F~F d ~F~F e ~F~F f ~F~F g ~F~F h ~F~F i ~F~F j",
    };

    std::vector<std::string> tests6 = {
            "~F~F a",
            "& ~F~F a ~F~F b",
            "& & ~F~F a ~F~F b ~F~F c",
            "& & & ~F~F a ~F~F b ~F~F c ~F~F d",
            "& & & & ~F~F a ~F~F b ~F~F c ~F~F d ~F~F e",
            "& & & & & ~F~F a ~F~F b ~F~F c ~F~F d ~F~F e ~F~F f ",
            "& & & & & & ~F~F a ~F~F b ~F~F c ~F~F d ~F~F e ~F~F f ~F~F g",
            "& & & & & & & ~F~F a ~F~F b ~F~F c ~F~F d ~F~F e ~F~F f ~F~F g ~F~F h",
            "& & & & & & & & ~F~F a ~F~F b ~F~F c ~F~F d ~F~F e ~F~F f ~F~F g ~F~F h ~F~F i",
//            "& & & & & & & & & ~F~F a ~F~F b ~F~F c ~F~F d ~F~F e ~F~F f ~F~F g ~F~F h ~F~F i ~F~F j",
    };


//    std::vector<std::string> tests_a{
//            "& ~F~a F~a",
//            "& & ~F~a F~a & ~F~b F~b",
//            "& & & ~F~a F~a & ~F~b F~b  & ~F~c F~c",
//            "& & & & ~F~a F~a & ~F~b F~b  & ~F~c F~c  & ~F~d F~d",
//            "& & & & & ~F~a F~a & ~F~b F~b  & ~F~c F~c  & ~F~d F~d & ~F~e F~e",
//            "& & & & & & ~F~a F~a & ~F~b F~b  & ~F~c F~c  & ~F~d F~d & ~F~e F~e & ~F~f F~f",
//            "& & & & & & & ~F~a F~a & ~F~b F~b  & ~F~c F~c  & ~F~d F~d & ~F~e F~e & ~F~f F~f & ~F~g F~g",
//            "& & & & & & & & ~F~a F~a & ~F~b F~b  & ~F~c F~c  & ~F~d F~d & ~F~e F~e & ~F~f F~f & ~F~g F~g & ~F~h F~h",
//            "& & & & & & & & & ~F~a F~a & ~F~b F~b  & ~F~c F~c  & ~F~d F~d & ~F~e F~e & ~F~f F~f & ~F~g F~g & ~F~h F~h & ~F~i F~i",
//    };

    std::vector<std::string> tests7{
            "& ~F~a F~a",
            "U & ~F~a F~a  & ~F~b F~b",
            "U U & ~F~a F~a  & ~F~b F~b  & ~F~c F~c",
            "U U U & ~F~a F~a  & ~F~b F~b  & ~F~c F~c  & ~F~d F~d",
            "U U U U & ~F~a F~a  & ~F~b F~b  & ~F~c F~c  & ~F~d F~d & ~F~e F~e",
            "U U U U U & ~F~a F~a  & ~F~b F~b  & ~F~c F~c  & ~F~d F~d & ~F~e F~e & ~F~f F~f",
    };

    for (auto &test: tests7) {
        test_sat_wolper(test);
    }
}

std::string random_formula(int depth, int var) {
    const char *vars = "abcdefghijklmnopqrstuvwxyz";
    const char *operators = "~XFU&";

    if (depth == 1) {
        return {vars[rand() % var]};
    }

    char op = operators[rand() % 5];
    auto type = get_type(op);
    if (type == SymType::BinaryOperator) {
        return op + random_formula(depth - 1, var) + random_formula(depth - 1, var);
    } else if (type == SymType::UnaryOperator) {
        return op + random_formula(depth - 1, var);
    }

    std::terminate();
}


void generate_random() {
    for (int i = 0; i < 1000; ++i) {
        auto f = random_formula(6, 5);
        std::cout << " ==== " << i << ") " << f << " ==== " << std::endl;
        auto res = test_sat_wolper(f);
        std::cout << " ==== " << res << " ==== " << std::endl;

        if (!res) {
            std::cerr << f << std::endl;
        }
    }
}

int main() {
//    all_tests();
    // "U~XUXbFa&UUUbb&aaX&ceFUFeXb"
//    test_sat_wolper("a");

//    generate_random();
    benchmark();
    return 0;
}
