//
// Created by h4zzkr on 03.10.2021.
//
#ifndef PROJECT_Automata_AUTOMATA_H
#define PROJECT_Automata_AUTOMATA_H

#pragma once

#include <vector>
#include <map>
#include <queue>
#include <unordered_set>
#include <unordered_map>
#include <functional>
#include <iostream>
#include <stdexcept>
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
        states.emplace_back(0, false, false);
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

    void add_transition(size_t from, T label, size_t to, bool is_terminal = false, bool check = true, bool quiet=true) {
        auto tr = Transition(label);

        if (!quiet or from >= number_of_states) {
            throw std::out_of_range("ERR: Add state before using it\n");
        } else {
            if (to >= number_of_states) {
                size_t npos = add_state(is_terminal);
                state_labels.insert({to, npos});
            }
        }

        states.at(at(to)).is_terminal |= is_terminal;
        states.at(from).transitions.insert({tr, at(to)});
        alphabet.insert(label);
    }

    void toggle_terminal(size_t state) {
        states.at(state).is_terminal ^= true;
    }

    void drop_transition(size_t from, T label, size_t to) {
        auto range = states[from].transitions.equal_range(Transition(label));
        for (auto i = range.first; i != range.second; ++i) {
            if (states[i->second].id == to) {
                states[from].transitions.erase(i);
                break;
            }
        }
    }

    bool read_string(std::string word="only_for_testing") {
        return read_expression(word.begin(), word.end());
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
            }
        }

        // drop epsilon transitions
        for (auto& state : states) {
            auto range = state.transitions.equal_range(Transition(epsilon_transition_symbol));
            auto to = range.first;
            bool is_state_starting = (state.id == initial_state);
            while (to != range.second) {
                auto prey = to; ++to; // anti-invalidation
                state.transitions.erase(prey);
            }
        }

    }

    /* TRAVERSING */

    template <typename IterType>
    bool read_expression(IterType iter, IterType end) {
        bool read_state = false;
        while (iter != end) {
            auto info = step(*iter);
            read_state = info.second;
            ++iter;
        }
        current_state_id = 0;

        return read_state;
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

    /* Get pos of real-labeled and fake-labeld states */
    size_t at(size_t pos) {
        size_t real_pos;
        if (pos < number_of_states)
            real_pos = pos;
        else {
            try {
                real_pos = state_labels.at(pos);
            } catch (const std::out_of_range& e) {
                std::cout << "ERR: Add state before using it\n";
            }
        }
        return real_pos;
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
        bool is_initial  = false;
        bool is_terminal = false;
        std::multimap<Transition, size_t, cmp> transitions;
        State(size_t id = 0, bool is_terminal = false, bool is_initial = false): id(id),
                    is_terminal(is_terminal), is_initial(is_initial) {}
    };


    size_t current_state_id = 0;
    size_t number_of_states = 1;
    T epsilon_transition_symbol;

    size_t initial_state = 0;

    std::set<T> alphabet;
    std::map<size_t, size_t> state_labels;
    std::vector<State> states;

};

#endif //PROJECT_Automata_AUTOMATA_H
