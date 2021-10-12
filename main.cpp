#include <iostream>
//#include "automata.h"

#include <vector>
#include <map>
#include <queue>
#include <unordered_set>
#include <functional>
#include <set>

namespace automata_stuff {
    struct hash {
        std::size_t operator()(std::set<size_t> const& container) const {
            std::size_t seed = container.size();
            for(auto& i : container) {
                seed ^= i + 0x9e3779b9 + (seed << 6) + (seed >> 2);
            }
            return seed;
        }
    };
}

template<typename T=char>
class Automata {
public:
    explicit Automata(T epsilon_symbol): epsilon_transition_symbol(epsilon_symbol) {
        initial_state = 0;
        states.emplace_back(0);
    }

    explicit Automata(size_t states_cnt) {
        states.clear();
        states.reserve(states_cnt);
        for (int i = 0; i < states_cnt; ++i)
            add_state(false);
        number_of_states = states_cnt;
    }

    Automata& operator=(Automata&& aut)  noexcept {
        number_of_states = aut.number_of_states;
        epsilon_transition_symbol = aut.epsilon_transition_symbol;
        initial_state = aut.initial_state;
        alphabet = std::move(aut.alphabet);
        states = std::move(aut.states);
        return *this;
    }

    size_t add_state(bool is_terminal = false, bool is_initial = false) {
        states.emplace_back(number_of_states, is_terminal);
        ++number_of_states;
        return number_of_states - 1;
    }

    void add_transition(size_t from, T label, size_t to, bool is_terminal = false, bool check = true) {
        try {
            auto tr = Transition(label);
            states.at(from).transitions.insert({tr, to});
            states.at(to).is_terminal |= is_terminal;
            if (check)
                alphabet.insert(label);
        } catch (const std::out_of_range& e) {
            std::cout << "ERR: Add state before using it\n";
        }
    }

    size_t begin() {
        return initial_state;
    }

    /* AUTOMATA TRANSFORMATIONS */

    // https://neerc.ifmo.ru/wiki/index.php?title=Построение_по_НКА_эквивалентного_ДКА,_алгоритм_Томпсона
    void thompson_nfa2dfa() {

        epsilon_edges_elimination();

        std::queue<std::pair<size_t, std::set<size_t>>> Q;
        Automata<T> dfa(epsilon_transition_symbol);
        std::unordered_map<size_t, size_t> tracking_q;
        auto hasher = automata_stuff::hash{};

        size_t initial_hash = hasher({initial_state});
        Q.push({initial_hash, {initial_state}});
        tracking_q.insert({initial_hash, 0});

        while (!Q.empty()) {
            auto [hash, vset] = Q.front(); Q.pop();

            if (vset.size() == 1 && states[*vset.begin()].transitions.empty())
                continue;

            for (auto& label : alphabet) {
                std::set<size_t> to_vset;
                bool is_terminal = false;
                for (size_t v : vset) {

                    auto range = states[v].transitions.equal_range(Transition(label));
                    for (auto i = range.first; i != range.second; ++i) {
                        if (states[i->second].is_terminal)
                            is_terminal = true;
                        to_vset.insert(i->second);
                    }

                }

                if (to_vset.empty()) continue;

                size_t vhash = hasher(to_vset);
                auto found = tracking_q.find(vhash);

                if (found == tracking_q.end()) {
                    dfa.add_state(is_terminal, false);
                    size_t from = tracking_q.find(hash)->second;
                    dfa.add_transition(from, label, tracking_q.size());
                    tracking_q.insert({vhash, tracking_q.size()});
                    Q.push({vhash, to_vset});
                } else {
                    size_t from = tracking_q.find(hash)->second;
                    size_t to   = found->second;
                    dfa.add_transition(from, label, to);
                }
            }
        }
        *this = std::move(dfa);
    }

    // https://neerc.ifmo.ru/wiki/index.php?title=Автоматы_с_eps-переходами._Eps-замыкание
    void epsilon_edges_elimination() {
        for (auto& state : states) {
            // find transitive closure
            std::set<size_t> closure;
            dfs(closure, state.id); // if state id IS POSITION <TODO>
            for (size_t v : closure) {
                for (T label : alphabet) {
                    auto range = states[v].transitions.equal_range(Transition(label));
                    for (auto to = range.first; to != range.second; ++to) {
                        add_transition(state.id, label, to->second, false, false);
                    }
                }
//                for (auto& [label, to] : states[v].transitions) {
//                    add_transition(state.id, label.label, to, false, false);
//                }
            }
        }

        for (auto& state : states) {
            auto range = state.transitions.equal_range(Transition(epsilon_transition_symbol));
            auto to = range.first;
            while (to != range.second) {
                auto prey = to; ++to; // anti-invalidation
                state.transitions.erase(prey);
            }
        }

    }

    /* TRAVERSING */

    template <typename IterType>
    bool read_expression(IterType iter, IterType end) {
        while (iter != end) {
            auto info = step(*iter);
            if (info.first == -1)
                return false;
            if (info.second == 1)
                return true;
            ++iter;
        }
        return false;
    }

    std::pair<int,bool> step(T label) {
        auto it = states[current_state_id].transitions.find(Transition(label));
        if (it != states[current_state_id].transitions.end()) {
            current_state_id = it->second;
            return { current_state_id, states[it->second].is_terminal };
        } else {
            return {-1, false};
        }
    }

private:

    void dfs_(std::set<size_t>& closure, size_t v) {
        closure.insert(v);
        for (auto& [label, state] : states[v].transitions) {
            if (!closure.count(state) && label.label == epsilon_transition_symbol) {
                dfs_(closure, state);
            }
        }
    }

    void dfs(std::set<size_t>& closure, size_t v) {
        dfs_(closure, v);
        closure.erase(v); // remove self
    }


    struct Transition {
        T label;
        explicit Transition(T label = T()): label(label) {}
    };

    struct cmp {
        bool operator()(const Transition& t1, const Transition& t2) const {
            return t1.label < t2.label;
        }
    };

    struct State {
        size_t id;
        bool is_terminal = false;
        std::multimap<Transition, size_t, cmp> transitions;
        State(size_t id = 0, bool is_terminal = false): id(id), is_terminal(is_terminal) {}
    };


    size_t current_state_id = 0;
    size_t number_of_states = 1;
    T epsilon_transition_symbol;

    size_t initial_state = 0;

    std::set<T> alphabet;
    std::vector<State> states;
//    std::vector<size_t> dfs_labels;

};


int main() {
    Automata<char> autom('#');     // 1

    auto st = autom.add_state(false); // 2
    auto st1 = autom.add_state(true); // 3

    autom.add_transition(autom.begin(), 'a', autom.begin());
    autom.add_transition(autom.begin(), 'a', st);
    autom.add_transition(autom.begin(), 'b', st1);
    autom.add_transition(st, 'a', st);
    autom.add_transition(st, 'b', autom.begin());
//    autom.add_transition(st, 'b', st1);
    autom.add_transition(st, '#', st1);
    autom.add_transition(st1, 'a', st1);
    autom.add_transition(st1, 'b', st1);

    autom.epsilon_edges_elimination();


//    std::string ex("ab");
//    autom.read_expression(ex.begin(), ex.end());
    autom.thompson_nfa2dfa();
    return 0;
}

// TODO: epsilon elimination
// TODO: std::hash instead custom-make hash