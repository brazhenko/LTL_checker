#pragma

#include <vector>
#include <cstddef>
#include <algorithm>
#include <cassert>
#include <ctime>
#include <cstdlib>
#include <unordered_set>
#include <queue>
#include <algorithm>

class Graph {
public:
    explicit Graph(size_t count) : vertices(count) {}

    struct Vertex {
        std::unordered_set<size_t> children;
        int refcount = 0;
    };

    void addEdge(size_t left, size_t right) {
        if (vertices[left].children.contains(right)) {
            return;
        }

        vertices[left].children.insert(right);
        vertices[right].refcount++;
    }

    void removeEdge(size_t left, size_t right) {
        if (vertices[left].children.contains(right)) {
            return;
        }

        vertices[left].children.insert(right);
        vertices[right].refcount--;
    }

    auto &get_vertex(size_t idx) {
        return vertices[idx];
    }

    std::vector<size_t> getSources() const {
        std::vector<size_t> res;

        for (size_t i = 0; i < vertices.size(); ++i) {
            if (vertices[i].refcount == 0) {
                res.push_back(i);
            }
        }

        return res;
    }

    size_t nvertex() const {
        return vertices.size();
    }

    bool reachable(size_t left, size_t right) const {
        std::queue<size_t> queue;
        std::vector<int> visited(vertices.size());

        queue.push(left);
        visited[left] = true;

        while (!queue.empty()) {
            auto f = queue.front();
            queue.pop();

            if (f == right) {
                return true;
            }

            for (auto &child: vertices[f].children) {
                if (!visited[child]) {
                    queue.push(child);
                    visited[child] = true;
                }
            }
        }

        return false;
    }

    Graph transpose() {
        Graph res(vertices.size());

        for (size_t i = 0; i < vertices.size(); ++i) {
            for (auto &child: vertices[i].children) {
                res.addEdge(child, i);
            }
        }

        return res;
    }

private:
    std::vector<Vertex> vertices;

};


Graph generateRandomDAG(size_t n, size_t m) {
    Graph graph(n);

    assert(n > 0);
    assert(m > 0);

    for (size_t i = 0; i < m; ++i) {
        int right = 1 + rand() % (n - 1);
        int left = rand() % right;

        graph.addEdge(left, right);
    }

    return graph;
}

Graph generateStronglyConnectedGraph(size_t n, size_t m) {
    std::vector<int> vertices(n);
    for (int i = 0; i < n; i++) {
        vertices[i] = i;
    }

    std::random_shuffle(vertices.begin(), vertices.end());

    Graph graph(n);

    for (int i = 0; i < n; ++i) {
        graph.addEdge(vertices[i], vertices[(i + 1) % n]);
    }

    for (int i = n; i < m; ++i) {
        int from = rand() % n;
        int to = rand() % n;
        if (from == to) {
            continue;
        }
        graph.addEdge(from, to);
    }

    return graph;
}