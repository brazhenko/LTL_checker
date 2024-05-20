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
#include "cow_string.h"
// Доступные символы
// a-z -- variables
// F -- finally
// ~ -- not
// & -- and
// U -- until
// X -- neXt


struct Formula {
    CowString formula;
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
            return formula.at(0) == '~';
        }

        return false;
    }

    bool literal() const {
        if (formula.size() == 1) {
            return true;
        }

        if (formula.size() == 2) {
            return formula.at(0) == '~';
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
    static size_t get_formula_hash(const Formula &formula) {
        auto tmp = formula.formula.hash();
        if (formula.marked_) {
            tmp = ~tmp;
        }

        return tmp;
    }

    void add_formula(const Formula &formula) {
        auto tmp = get_formula_hash(formula);

        if (!set.contains(formula)) {
            set.insert(formula);
            hash_ ^= tmp;
        }
    }

    std::optional<std::pair<Formula, FormulaeSet>> find_non_elementary_non_marked() {
        for (auto it = set.begin(); it != set.end(); ++it) {
            if (!it->marked_ and !it->elementary()) {
                auto tmp = set.extract(it);
                auto tmp_hash = get_formula_hash(tmp.value());
                hash_ ^= tmp_hash;

                auto formula_copy = tmp.value();
                formula_copy.marked_ = true;
                auto copy = *this;
                copy.add_formula(formula_copy);

                hash_ ^= tmp_hash;
                set.insert(std::move(tmp));

                return std::pair{formula_copy, std::move(copy)};
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

    bool contains(const Formula &formula) {
        return set.contains(formula);
    }

    bool contains(const CowString &formula) {
        return set.contains(Formula{.formula=formula, .marked_=true}) ||
               set.contains(Formula{.formula=formula, .marked_=false});
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

    void add_formula(const CowString &formula, bool marked) {
        Formula form = {formula, marked};
        if (!marked && !form.elementary()) {
            state = false;
        }

        formulae_set.add_formula(form);
    }

    void add_formula(const Formula &form) {
        if (!form.marked_ && !form.elementary()) {
            state = false;
        }

        formulae_set.add_formula(form);
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
    CowString left, right, ultimate;
};

enum class SymType {
    UnaryOperator = 0,
    BinaryOperator,
    Var
};

class LTLGraphWolper {
public:
    LTLGraphWolper() = delete;

    explicit LTLGraphWolper(const std::string &init_formula);

    void build();

    void eliminate();

    bool sat() const {
        return !root_->eliminated;
    }

private:
    std::unordered_set<PtlNode *> visited_eliminate_;
    std::unordered_set<PtlNode *> visited_eliminate_rule3_;
    std::unordered_map<FormulaeSet, PtlNode *> set_to_ptr_;
    PtlNode *root_;

    bool any_eliminated = false;

    bool all_children_eliminated(PtlNode *node);

    bool contain_prop_contraversion(PtlNode *node);

    bool rule3_trav(PtlNode *node, const CowString &search);

    bool rule3(PtlNode *node);

    void traverse(PtlNode *node);

    // stats
    uint64_t rule3_deletions = 0;
    uint64_t micros_per_rule3 = 0;
};


