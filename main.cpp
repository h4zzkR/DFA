#include <iostream>
#include "automata.h"

int main() {
    Automata<char> autom('#');

    auto st = autom.add_state(false);
    auto st1 = autom.add_state(true);

    autom.add_transition(autom.begin(), 'a', autom.begin());
    autom.add_transition(autom.begin(), 'a', st);
    autom.add_transition(autom.begin(), 'b', st1);
    autom.add_transition(st, 'a', st);
    autom.add_transition(st, 'b', autom.begin());
    autom.add_transition(st, 'b', st1);
    autom.add_transition(st1, 'a', st1);
    autom.add_transition(st1, 'b', st1);


//    std::string ex("ab");
//    autom.read_expression(ex.begin(), ex.end());
    autom.thompson_nfa2dfa();
    return 0;
}

// TODO: epsilon elimination
// TODO: std::hash instead custom-make hash