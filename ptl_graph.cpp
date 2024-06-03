//
// Created by andrew on 3/20/24.
//

#include <chrono>
#include <cassert>
#include "ptl_graph.h"
#include "timer.h"

inline std::string to_string(Lexer *lexer) {
    auto [c, _] = lexer->get();
    auto type = get_type(c);

    if (type == SymType::UnaryOperator) {
        return c + to_string(lexer);
    } else if (type == SymType::BinaryOperator) {
        auto left = to_string(lexer);
        auto right = to_string(lexer);

        return "(" + left + ") " + c + " (" + right + ")";
    } else {
        return {c};
    }
}


// <start, end>
inline std::pair<size_t, size_t> get_formula(Lexer *lexer) {
    auto [c, pos] = lexer->get();
    auto type = get_type(c);

    if (type == SymType::UnaryOperator) {
        auto tmp = get_formula(lexer);
        assert(pos + 1 == tmp.first);

        return {pos, tmp.second};

    } else if (type == SymType::BinaryOperator) {
        auto left = get_formula(lexer);
        auto right = get_formula(lexer);
        assert(left.second == right.first);
        assert(pos + 1 == left.first);

        return {pos, right.second};
    } else {
        return {pos, pos + 1};
    }
}


inline std::string formula_to_str(const Formula &f) {
    Lexer lexer(f.formula);

    auto result = to_string(&lexer);

    if (f.marked_) {
        result += " * ";
    }

    return result;
}

inline std::pair<CowString, CowString> get_left_right_operands(const CowString &formula) {
    auto f = formula.substr(1);
    Lexer lexer(f);

    auto left = get_formula(&lexer);
    auto right = get_formula(&lexer);

    return {f.substr(left.first, left.second - left.first),
            f.substr(right.first, right.second - right.first)};
}

inline DecomposeResult decompose(const CowString &string) {
    using Variant = DecomposeResult::Variant;
    auto doubleneg = [] (const CowString &string) {
        if (string.starts_with("~~")) {
            return string.substr(2);
        }

        return string;
    };


    if (string.starts_with("~~")) {
        return {Variant::One, string.substr(2)};
    }

    if (string.starts_with("|")) {
        auto [left, right] = get_left_right_operands(string);
        return {Variant::One, "~&~" + left.get_copy() + "~" + right.get_copy()};
    }

    if (string.starts_with("~X")) {
        std::string res = string.get_copy();
        std::swap(res[0], res[1]);
        return {Variant::One, res};
    }

    if (string.starts_with("&")) {
        auto [left, right] = get_left_right_operands(string);
        return {Variant::Two, left, right};
    }

    if (string.starts_with("~&")) {
        auto [left, right] = get_left_right_operands(string.substr(1));
        return {Variant::TwoSeparate, doubleneg("~" + left.get_copy()), doubleneg("~" + right.get_copy())};
    }

    if (string.starts_with("F")) {
        return {Variant::TwoSeparate, string.substr(1), "X" + string.get_copy()};
    }

    if (string.starts_with("~F")) {
        return {Variant::Two, doubleneg("~" + string.substr(2).get_copy()), "X~F" + string.substr(2).get_copy()};
    }

    if (string.starts_with("U")) {
        auto [left, right] = get_left_right_operands(string);
        return {Variant::Ultimate, right, left, "XU" + left.get_copy() + right.get_copy()};
    }

    if (string.starts_with("~U")) {
        auto [left, right] = get_left_right_operands(string.substr(1));
        return {Variant::Two, doubleneg("~" + right.get_copy()),
                "&~" + left.get_copy() + "~XU" + left.get_copy() + right.get_copy()};
    }

    std::terminate();
}

LTLGraphWolper::LTLGraphWolper(const std::string &init_formula) {
    auto formula = init_formula;
    formula.erase(std::remove(formula.begin(), formula.end(), ' '), formula.end());

    root_ = new PtlNode;
    root_->prestate = true;
    root_->add_formula(formula, false);

    set_to_ptr_[root_->formulae_set] = root_;
}

PtlNode *LTLGraphWolper::registrate(const FormulaeSet &set) {
    return nullptr;
}

void LTLGraphWolper::build() {
    std::deque<PtlNode *> traverse;
    traverse.push_back(root_);

    // BFS for graph building
    while (!traverse.empty()) {
        PtlNode *current = traverse.front();
        traverse.pop_front();

        if (current->visited) {
            continue;
        }

        current->visited = true;

//        std::cout << "=== Traverse, state: " << current->state << ", prestate: " << current->prestate << ", hash: "
//                  << std::hex << current->formulae_set.hash() << ", eliminated: " << current->eliminated
//                  << std::endl;
//
//        for (auto &el: current->formulae_set.get_ref()) {
//            std::cout << formula_to_str(el) << std::endl;
//        }


        auto object = current->formulae_set.find_non_elementary_non_marked();

        // step 2)
        if (object.has_value()) {
            auto [value, copy] = object.value();

            auto res = decompose(value.formula);

            if (res.variant == DecomposeResult::Variant::One) {
                auto new_node = new PtlNode;
                for (auto &el: copy.get_ref()) {
                    new_node->add_formula(el);
                }
                new_node->add_formula(res.left, false);


                auto it = set_to_ptr_.find(new_node->formulae_set);
                if (it != set_to_ptr_.end()) {
                    current->left = it->second;
                    delete new_node;
                } else {
                    current->left = new_node;
                    set_to_ptr_[new_node->formulae_set] = new_node;
                    traverse.push_back(new_node);
                }

            } else if (res.variant == DecomposeResult::Variant::Two) {
                auto new_node = new PtlNode;

                for (auto &el: copy.get_ref()) {
                    new_node->add_formula(el);
                }

                new_node->add_formula(res.left, false);
                new_node->add_formula(res.right, false);

                auto it = set_to_ptr_.find(new_node->formulae_set);
                if (it != set_to_ptr_.end()) {
                    current->left = it->second;
                    delete new_node;
                } else {
                    current->left = new_node;
                    set_to_ptr_[new_node->formulae_set] = new_node;
                    traverse.push_back(new_node);
                }

            } else if (res.variant == DecomposeResult::Variant::TwoSeparate) {
                auto new_node1 = new PtlNode;
                for (auto &el: copy.get_ref()) {
                    new_node1->add_formula(el);
                }
                auto new_node2 = new PtlNode;
                for (auto &el: copy.get_ref()) {
                    new_node2->add_formula(el);
                }

                new_node1->add_formula(res.left, false);
                new_node2->add_formula(res.right, false);


                auto it = set_to_ptr_.find(new_node1->formulae_set);
                if (it != set_to_ptr_.end()) {
                    current->left = it->second;
                    delete new_node1;
                } else {
                    current->left = new_node1;
                    set_to_ptr_[new_node1->formulae_set] = new_node1;
                    traverse.push_back(new_node1);
                }

                auto it2 = set_to_ptr_.find(new_node2->formulae_set);
                if (it2 != set_to_ptr_.end()) {
                    current->right = it2->second;
                    delete new_node2;
                } else {
                    current->right = new_node2;
                    set_to_ptr_[new_node2->formulae_set] = new_node2;
                    traverse.push_back(new_node2);
                }

            } else if (res.variant == DecomposeResult::Variant::Ultimate) {
                auto new_node1 = new PtlNode;

                for (auto &el: copy.get_ref()) {
                    new_node1->add_formula(el);
                }

                auto new_node2 = new PtlNode;
                for (auto &el: copy.get_ref()) {
                    new_node2->add_formula(el);
                }

                new_node1->add_formula(res.left, false);
                new_node2->add_formula(res.right, false);
                new_node2->add_formula(res.ultimate, false);

                auto it = set_to_ptr_.find(new_node1->formulae_set);
                if (it != set_to_ptr_.end()) {
                    current->left = it->second;
                    delete new_node1;
                } else {
                    current->left = new_node1;
                    set_to_ptr_[new_node1->formulae_set] = new_node1;
                    traverse.push_back(new_node1);
                }

                auto it2 = set_to_ptr_.find(new_node2->formulae_set);
                if (it2 != set_to_ptr_.end()) {
                    current->right = it2->second;
                    delete new_node2;
                } else {
                    current->right = new_node2;
                    set_to_ptr_[new_node2->formulae_set] = new_node2;
                    traverse.push_back(new_node2);
                }
            } else {
                std::terminate();
            }

        } else { // step 3)
            auto son = new PtlNode;
            son->prestate = true;

            // step 3)
            for (const auto &formula: current->formulae_set.get_ref()) {
                if (formula.formula.starts_with("X")) {
                    son->add_formula(formula.formula.substr(1), false);
                }
            }

            if (!son->formulae_set.empty()) {
                auto it = set_to_ptr_.find(son->formulae_set);
                if (it == set_to_ptr_.end()) {
                    current->left = son;
                    set_to_ptr_[son->formulae_set] = son;
                    traverse.push_back(son);
                } else {
                    delete son;
                    current->left = it->second;
                    it->second->prestate = true;
                }
            } else {
                delete son;
            }
        }


    }

    std::cout << "Nodes: " << std::dec << set_to_ptr_.size() << std::endl;
}

void LTLGraphWolper::eliminate() {
    bool simple_contr = true;
    int count = 0;

    while (true) {
        visited_eliminate_.clear();

        // eliminate
        any_eliminated = false;
        traverse(root_, simple_contr);
        simple_contr = false;

        ++count;

        if (!any_eliminated) {
            break;
        }
    }

    if (!sat()) {
        std::cout << "NOT SAT" << std::endl;
        std::cout << "E3: " << rule3_deletions << std::endl;
        std::cout << "E3 time: " << micros_per_rule3 << std::endl;
        std::cout << "Iters: " << count << std::endl;
    }

//    std::cout << "=== Traverse, state: " << root_->state << ", prestate: " << root_->prestate << ", hash: "
//              << std::hex << root_->formulae_set.hash() << ", eliminated: " << root_->eliminated << std::endl;
}

void LTLGraphWolper::traverse(PtlNode *node, bool simple_contr) {
    if (!node) {
        return;
    }
    if (node->eliminated) {
        return;
    }

    if (visited_eliminate_.contains(node)) {
        return;
    }

    visited_eliminate_.insert(node);

    // Elimination Rule 1
    if (simple_contr) {
        if (contain_prop_contraversion(node)) {
            node->eliminated = true;
            any_eliminated = true;
            return;
        }
    }

        // Elimination Rule 2
    else if (all_children_eliminated(node)) {
        node->eliminated = true;
        any_eliminated = true;
        return;
    }

        // Elimination Rule 3
    else if (rule3(node)) {
        node->eliminated = true;
        any_eliminated = true;

        ++rule3_deletions;
        return;
    }

    traverse(node->left, simple_contr);
    traverse(node->right, simple_contr);
}

bool LTLGraphWolper::all_children_eliminated(PtlNode *node) {
    if (node->left and node->right) {
        return node->left->eliminated and node->right->eliminated;
    } else if (node->left) {
        return node->left->eliminated;
    }

    return false;
}

bool LTLGraphWolper::contain_prop_contraversion(PtlNode *node) {
    for (auto &f: node->formulae_set.get_ref()) {
        if (f.literal()) {
            if (node->formulae_set.contains("~" + f.formula.get_copy()) != 0) {
                return true;
            }
        }
    }

    return false;
}

bool LTLGraphWolper::rule3(PtlNode *node) {
    auto start = std::chrono::high_resolution_clock::now();

    if (!node->prestate) {
        return false;
    }

    for (auto &f: node->formulae_set.get_ref()) {
        if (f.formula.at(0) == 'F' || f.formula.at(0) == 'U') {
            auto search = f.formula.at(0) == 'U' ? get_left_right_operands(f.formula).second : f.formula.substr(1);

            visited_eliminate_rule3_.clear();
            if (!rule3_trav(node, search)) {
                auto end = std::chrono::high_resolution_clock::now();
                micros_per_rule3 += std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
                return true;
            }
        }
    }

    auto end = std::chrono::high_resolution_clock::now();
    micros_per_rule3 += std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();

    return false;
}

bool LTLGraphWolper::rule3_trav(PtlNode *node, const CowString &search) {
    if (!node) {
        return false;
    }

    if (node->eliminated) {
        return false;
    }

    if (visited_eliminate_rule3_.contains(node)) {
        return false;
    }

    visited_eliminate_rule3_.insert(node);

    if (node->formulae_set.contains(search)) {
        return true;
    }

    return rule3_trav(node->left, search) || rule3_trav(node->right, search);
}

LTLGraphWolper::~LTLGraphWolper() {
    std::unordered_map<PtlNode *, int> node_to_count;
    int max = 0, state = 0, prestate = 0;
    std::cout << "SCC: " << scc_count() << std::endl;


    for (auto &[_, node]: set_to_ptr_) {
        state += node->state;
        prestate += node->prestate;
        if (node->left) {
            node_to_count[node->left]++;
            max = std::max(max, node_to_count[node->left]);
        }
        if (node->right) {
            node_to_count[node->right]++;
            max = std::max(max, node_to_count[node->right]);
        }

        delete node;
    }

    std::cout << "Max ref: " << max << ", state: " << state << ", prestate: " << prestate  << std::endl;
}


void dfs1(size_t v, std::vector<size_t> &used, std::vector<std::vector<size_t>> &g, std::vector<size_t> &order) {
    used[v] = true;
    for (size_t i = 0; i < g[v].size(); ++i)
        if (!used[g[v][i]])
            dfs1(g[v][i], used, g, order);
    order.push_back(v);
}

void dfs2(size_t v, std::vector<size_t> &used, std::vector<std::vector<size_t>> &gr, std::vector<size_t> &component) {
    used[v] = true;
    component.push_back(v);
    for (size_t i = 0; i < gr[v].size(); ++i)
        if (!used[gr[v][i]]) {
            dfs2(gr[v][i], used, gr, component);
        }
}

size_t LTLGraphWolper::scc_count() const {
    const auto n = set_to_ptr_.size();

    std::vector<PtlNode *> nodes;
    std::unordered_map<PtlNode *, size_t> node_to_idx;

    std::vector<std::vector<size_t>> g(n), gr(n);

    {
        size_t i = 0;
        for (const auto &[_, node]: set_to_ptr_) {
            nodes.push_back(node);
            node_to_idx[node] = i++;
        }
    }

    for (size_t i = 0; i < n; ++i) {
        if (nodes[i]->left) {
            g[i].push_back(node_to_idx.at(nodes[i]->left));
            gr[node_to_idx.at(nodes[i]->left)].push_back(i);
        }
        if (nodes[i]->right) {
            g[i].push_back(node_to_idx.at(nodes[i]->right));
            gr[node_to_idx.at(nodes[i]->right)].push_back(i);
        }
    }

    std::vector<size_t> used, order, component;

    used.assign(n, false);
    for (size_t i = 0; i < n; ++i) {
        if (!used[i]) {
            dfs1(i, used, g, order);
        }
    }


    used.assign(n, false);
    size_t scc_count = 0;

    for (size_t i = 0; i < n; ++i) {
        size_t v = order[n - 1 - i];
        if (!used[v]) {
            dfs2(v, used, gr, component);
            ++scc_count;
            component.clear();
        }
    }


    return scc_count;
}

#include <fstream>

void LTLGraphWolper::to_file(const std::string& file) const {
    std::ofstream ofs(file, std::ios::trunc);
    if (!ofs.is_open()) {
        std::cerr << "Error: Unable to open file " << file << std::endl;
        return;
    }

    for (const auto &[_, node] : set_to_ptr_) {
        ofs << "NODE" << std::endl;
        ofs << std::hex << node << std::endl;

        if (node->left) {
            ofs << std::hex << node->left << " ";
        }
        if (node->right) {
            ofs << std::hex << node->right;
        }

        ofs << std::endl;
        ofs << int(node->state) << " " << int(node->prestate) << " " << int(node->eliminated) << std::endl;

        auto decompose = node->formulae_set.find_non_elementary_non_marked();
        if (decompose.has_value()) {
            ofs << "D*: " << formula_to_str(decompose.value().first) << std::endl;
        }

        for (auto &f : node->formulae_set.get_ref()) {
            ofs << formula_to_str(f) << std::endl;
        }
    }
    ofs.close();
}



