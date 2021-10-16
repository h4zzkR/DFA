# deus_ex_automata
Abstract automata (NFA/DFA) pure C++ implementation. Supported features like epsilon edges elimination, nfa2dfa transformation, etc.

## Changelog
  - thompson algorithm implemented, tested on some example

## Tests
```
cmake .. -DBUILD_TEST=ON && make && ctest 
```
  
## TODOs
  - ~~epsilon elimination~~
  - std::hash instead custom-make hash
  - nice-looking comments with logic explained
  - test base algorithms on some examples
