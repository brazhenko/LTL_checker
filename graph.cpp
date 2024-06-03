#include "graph.h"15 800 000
#include <queue>
#include <iostream>

std::unordered_set<size_t> find_unreachable(Graph g, size_t w) {
    auto sources = g.getSources();

    std::unordered_set<size_t> U;

    std::queue<size_t> queue;
    for (auto source: sources) {
        if (source != w) {
            queue.push(source);
        }
    }

    while (!queue.empty()) {
        auto f = queue.front();
        queue.pop();

        U.insert(f);

        for (auto &child: g.get_vertex(f).children) {
            --g.get_vertex(child).refcount;
            if (g.get_vertex(child).refcount == 0 and child != w) {
                queue.push(child);
            }
        }
    }

    return U;
}

void test_find_unreachable() {
    for (int i = 0; i < 20; ++i) {
        int n = 60;
        auto dag = generateRandomDAG(n, 120);


        for (int i = 0; i < n; ++                                        i) {
            auto U = find_unreachable(dag, i);

            for (int j = 0; j < n; ++j) {
                if (!dag.reachable(i, j) != U.contains(j)) {
                    throw std::runtime_error("shit");
                }
            }
        }
    }
}

void test_strongly_connected_generator() {
    for (int _ = 0; _ < 100; _++) {
        int n = 20;
        auto scc = generateStronglyConnectedGraph(n, 50);

        for (int i = 0; i < n; ++i)
        for (int j = 0; j < n; ++j) {
            assert(scc.reachable(i, j));
        }
    }
}

int main() {
    test_strongly_connected_generator();

    return 0;
}

