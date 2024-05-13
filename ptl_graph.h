#pragma once

#include "Lexer.h"

#include <vector>
#include <string>
#include <stack>
#include <unordered_map>
#include <string_view>
#include <exception>
#include <stdexcept>
#include <set>
#include <optional>
#include <iostream>
#include <algorithm>
#include <unordered_set>

// Доступные символы
// a-z -- variables
// F -- finally
// ~ -- not
// & -- and
// U -- until
// X -- neXt


struct Formula {
    std::string formula;
    bool marked_ = false;

    bool operator==(const Formula &right) const {
        return std::tie(formula, marked_) == std::tie(right.formula, right.marked_);
    }

    bool elementary() const {
        if (formula.starts_with("X")) {
            return true;
        }

        if (formula.size() == 1) {
            return true;
        }

        if (formula.size() == 2) {
            return formula[0] == '~';
        }


        return false;
    }

    bool literal() const {
        if (formula.size() == 1) {
            return true;
        }

        if (formula.size() == 2) {
            return formula[0] == '~';
        }

        return false;
    }
};

inline bool operator<(const Formula &left, const Formula &right) {
    return std::tie(left.formula, left.marked_) < std::tie(right.formula, right.marked_);
}

class FormulaeSet {
private:
    std::set<Formula> set;
    size_t hash_ = 0;
public:
    size_t get_formulae_hash(const Formula &formulae) const {
        auto tmp = std::hash<std::string>{}(formulae.formula);
        if (formulae.marked_) {
            tmp = ~tmp;
        }

        return tmp;
    }

    void add_formulae(const Formula &formulae) {
        auto tmp = get_formulae_hash(formulae);

        if (!set.contains(formulae)) {
            set.insert(formulae);
            hash_ ^= tmp;
        }
    }

    std::optional<std::pair<Formula, FormulaeSet>> find_non_elementary_non_marked() {
        for (auto it = set.begin(); it != set.end(); ++it) {
            if (!it->marked_ and !it->elementary()) {
                auto tmp = set.extract(it);
                auto tmp_hash = get_formulae_hash(tmp.value());
                hash_ ^= tmp_hash;

                auto formulae_copy = tmp.value();
                formulae_copy.marked_ = true;
                auto copy = *this;
                copy.add_formulae(formulae_copy);

                hash_ ^= tmp_hash;
                set.insert(std::move(tmp));

                return std::pair{formulae_copy, std::move(copy)};
            }
        }

        return std::nullopt;
    }

    bool operator==(const FormulaeSet &right) const {
        return std::tie(set, hash_) == std::tie(right.set, right.hash_);
    }

    const auto &get_ref() const {
        return set;
    }

    bool empty() const {
        return set.empty();
    }

    size_t hash() const {
        return hash_;
    }
};

namespace std {
    template<>
    struct hash<FormulaeSet> {
        std::size_t operator()(const FormulaeSet &k) const {
            return k.hash();
        }
    };
}


struct PtlNode {
    FormulaeSet formulae_set;
    PtlNode *left = nullptr, *right = nullptr;

    bool state = true;
    bool prestate = false;
    bool visited = false;
    bool eliminated = false;

    void add_formulae(const std::string &formulae, bool marked) {
        Formula form = {formulae, marked};
        if (!marked && !form.elementary()) {
            state = false;
        }

        formulae_set.add_formulae(form);
    }

    void add_formulae(const Formula &form) {
        if (!form.marked_ && !form.elementary()) {
            state = false;
        }

        formulae_set.add_formulae(form);
    }

};


struct DecomposeResult {
    enum class Variant {
        One = 0,
        Two,
        TwoSeparate,
        Ultimate
    };
    Variant variant;
    std::string left, right, ultimate;
};

enum class SymType {
    UnaryOperator = 0,
    BinaryOperator,
    Var
};

inline SymType get_type(char c) {
    if (c == 'F' || c == '~' || c == 'X') { // Finally, Not, Next
        return SymType::UnaryOperator;
    }
    if (c == '&' || c == 'U') {
        return SymType::BinaryOperator;
    }
    if ('a' <= c and c <= 'z') {
        return SymType::Var;
    }

    std::terminate();
}

inline std::string to_string(Lexer *lexer) {
    auto c = lexer->get();
    auto type = get_type(c);

    if (type == SymType::UnaryOperator) {
        return c + to_string(lexer);
    } else if (type == SymType::BinaryOperator) {
        auto left = to_string(lexer);
        auto right = to_string(lexer);

        return left + " " + c + " " + right;
    } else {
        return {c};
    }
}

inline std::string get_formula(Lexer *lexer) {
    auto c = lexer->get();
    auto type = get_type(c);

    if (type == SymType::UnaryOperator) {
        return c + get_formula(lexer);
    } else if (type == SymType::BinaryOperator) {
        auto left = get_formula(lexer);
        auto right = get_formula(lexer);

        return c + left + right;
    } else {
        return {c};
    }
}

inline std::string formula_to_str(const Formula &f) {
    Lexer lexer(f.formula.c_str());

    auto result = get_formula(&lexer);

    if (f.marked_) {
        result += " * ";
    }

    return result;
}

inline std::pair<std::string, std::string> get_left_right_operands(const std::string &formula) {
    Lexer lexer(formula.c_str() + 1);

    auto left = get_formula(&lexer);
    auto right = get_formula(&lexer);

    return {left, right};
}

inline DecomposeResult decompose(const std::string &string) {
    using Variant = DecomposeResult::Variant;

    if (string.starts_with("~~")) {
        return {Variant::One, string.substr(2)};
    }

    if (string.starts_with("~X")) {
        std::string res = string;
        std::swap(res[0], res[1]);
        return {Variant::One, std::move(res)};
    }

    if (string.starts_with("&")) {
        auto [left, right] = get_left_right_operands(string);
        return {Variant::Two, left, right};
    }

    if (string.starts_with("~&")) {
        auto [left, right] = get_left_right_operands(string.substr(1));
        return {Variant::TwoSeparate, "~" + left, "~" + right};
    }

    if (string.starts_with("F")) {
        return {Variant::TwoSeparate, string.substr(1), "X" + string};
    }

    if (string.starts_with("~F")) {
        return {Variant::Two, "~" + string.substr(2), "~XF" + string.substr(2)};
    }

    if (string.starts_with("U")) {
        auto [left, right] = get_left_right_operands(string);
        return {Variant::Ultimate, right, left, "XU" + left + right};
    }

    if (string.starts_with("~U")) {
        auto [left, right] = get_left_right_operands(string.substr(1));
        return {Variant::Two, "~" + right, "&~" + left + "~XU" + left + right};
    }

    std::terminate();
}

class PtlGraph {
public:
    PtlGraph() = delete;

    explicit PtlGraph(const std::string &init_formula) {
        auto formula = init_formula;
        formula.erase(
                std::remove(formula.begin(), formula.end(), ' '),
                formula.end()
        );


        root_ = new PtlNode;
        root_->prestate = true;
        root_->add_formulae(formula, false);

        set_to_ptr_[root_->formulae_set] = root_;
    }

    void build() {
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

            std::cout << "=== Traverse, state: " << current->state << ", prestate: " << current->prestate << ", hash: "
                      << std::hex << current->formulae_set.hash()  << ", eliminated: " << current->eliminated << std::endl;
            for (auto &el: current->formulae_set.get_ref()) {
                std::cout << formula_to_str(el) << std::endl;
            }


            auto object = current->formulae_set.find_non_elementary_non_marked();

            // step 2)
            if (object.has_value()) {
                auto [value, copy] = object.value();

                auto res = decompose(value.formula);

                if (res.variant == DecomposeResult::Variant::One) {
                    auto new_node = new PtlNode;
                    for (auto &el: copy.get_ref()) {
                        new_node->add_formulae(el);
                    }

                    new_node->add_formulae(res.left, false);

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
                        new_node->add_formulae(el);
                    }

                    new_node->add_formulae(res.left, false);
                    new_node->add_formulae(res.right, false);

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
                        new_node1->add_formulae(el);
                    }
                    auto new_node2 = new PtlNode;
                    for (auto &el: copy.get_ref()) {
                        new_node2->add_formulae(el);
                    }

                    new_node1->add_formulae(res.left, false);
                    new_node2->add_formulae(res.right, false);


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
                        new_node1->add_formulae(el);
                    }
                    auto new_node2 = new PtlNode;
                    for (auto &el: copy.get_ref()) {
                        new_node2->add_formulae(el);
                    }

                    new_node1->add_formulae(res.left, false);
                    new_node2->add_formulae(res.right, false);
                    new_node2->add_formulae(res.ultimate, false);

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
                for (const auto &formulae: current->formulae_set.get_ref()) {
                    if (formulae.formula.starts_with("X")) {
                        son->add_formulae(formulae.formula.substr(1), false);
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

        std::cout << "Nodes: " << set_to_ptr_.size() << std::endl;
    }

    std::unordered_set<PtlNode *> visited_eliminate_;
    std::unordered_set<PtlNode *> visited_eliminate_rule3_;
    bool any_eliminated = false;

    bool all_children_eliminated(PtlNode *node) {
        if (node->left and node->right) {
            return node->left->eliminated and node->right->eliminated;
        } else if (node->left) {
            return node->left->eliminated;
        }

        return false;
    }


    bool contain_prop_contraversion(PtlNode *node) {
        std::unordered_set<std::string> search;

        for (auto &f: node->formulae_set.get_ref()) {
            if (f.literal()) {
                search.insert(f.formula);
            }
        }

        for (auto &f: node->formulae_set.get_ref()) {
            if (f.literal()) {
                if (search.count("~" + f.formula) != 0) {
                    return true;
                }
            }
        }

        return false;
    }

    bool rule3_trav(PtlNode *node, const std::string &search) {
        if (!node) {
            return false;
        }
        if (node->eliminated) {
            return false;
        }
        if (visited_eliminate_.contains(node)) {
            return false;
        }


        for (auto &f: node->formulae_set.get_ref()) {
            if (f.formula == search) {
                return true;
            }
        }

        return rule3_trav(node->left, search) || rule3_trav(node->right, search);
    }

    bool rule3(PtlNode *node) {
        if (!node->prestate) {
            return false;
        }

        for (auto &f: node->formulae_set.get_ref()) {
            if (f.formula[0] == 'F' || f.formula[0] == 'U') {
                auto search = f.formula[0] == 'U' ? get_left_right_operands(f.formula).second : f.formula.substr(1);
                if (!(rule3_trav(node->left, search) || rule3_trav(node->right, search))) {
                    return true;
                }
                // search a path
            }
        }

        return false;
    }

    void traverse(PtlNode *node) {
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
        if (contain_prop_contraversion(node)) {
            node->eliminated = true;
            any_eliminated = true;
            return;
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
            return;
        }

        traverse(node->left);
        traverse(node->right);
    }

    void eliminate() {
        while (true) {
            visited_eliminate_.clear();

            // eliminate
            any_eliminated = false;
            traverse(root_);


            if (!any_eliminated) {
                break;
            }
        }

        std::cout << "=== Traverse, state: " << root_->state << ", prestate: " << root_->prestate << ", hash: "
                  << std::hex << root_->formulae_set.hash()  << ", eliminated: " << root_->eliminated << std::endl;
    }

private:
    std::unordered_map<FormulaeSet, PtlNode *> set_to_ptr_;
    PtlNode *root_;
};


