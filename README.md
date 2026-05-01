# Grammar Analyzer in C++

A C++ program that reads a context-free grammar and performs a suite of 
compiler construction analyses and transformations, built as part of CSE340 
at Arizona State University.

## Overview

This tool implements six core algorithms from compiler theory, all operating 
on a custom context-free grammar input format parsed with a hand-written 
recursive-descent parser.

## Features

- **Task 1 – Symbol Classification** — identifies and lists all terminals and 
  non-terminals in the order they appear in the grammar
- **Task 2 – Nullable Non-terminals** — computes the set of non-terminals 
  that can derive the empty string
- **Task 3 – FIRST Sets** — computes FIRST sets for all non-terminals
- **Task 4 – FOLLOW Sets** — computes FOLLOW sets for all non-terminals, 
  including EOF ($) handling
- **Task 5 – Left Factoring** — transforms a grammar into an equivalent 
  left-factored grammar suitable for predictive parsing
- **Task 6 – Left Recursion Elimination** — eliminates direct and indirect 
  left recursion from a grammar using the standard two-pass algorithm

## Tech Stack

![C++](https://img.shields.io/badge/C++-00599C?style=flat&logo=cplusplus&logoColor=white)
![GCC](https://img.shields.io/badge/GCC-A8B9CC?style=flat&logo=gnu&logoColor=black)
![Ubuntu](https://img.shields.io/badge/Ubuntu_22.04-E95420?style=flat&logo=ubuntu&logoColor=white)

## How to Compile & Run

```bash
# Compile
g++ -std=c++11 parser.cpp lexer.cpp -o a.out

# Run a specific task (1–6) with an input grammar file
./a.out <task_number> < input.txt

# Example: compute FIRST sets
./a.out 3 < grammar.txt
```

## Example

**Input grammar:**
```
decl -> idList colon ID *
idList -> ID idList1 *
idList1 -> *
idList1 -> COMMA ID idList1 *
#
```

**Task 3 output (FIRST sets):**
```
FIRST(decl) = { ID }
FIRST(idList) = { ID }
FIRST(idList1) = { COMMA }
```

## What I Learned

- Implementing classic compiler construction algorithms (FIRST, FOLLOW, 
  nullable sets) iteratively
- Transforming grammars via left factoring and left recursion elimination
- Building a recursive-descent parser from a formal grammar spec
- Working with lexical analysis, symbol tables, and grammar representations 
  in C++
