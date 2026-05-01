#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <unordered_map>
#include <set>
#include <vector>
#include <string>
#include <sstream>
#include <queue>
#include <algorithm>
#include "lexer.h"

using namespace std;

// Represents a production rule where 'head' is the left side and 'body' the right side.
// An empty 'body' represents an epsilon production.
struct ProductionRule {
    string head;             
    vector<string> body;     
};

// Simple utility to display messages.
void display(const string &msg) {
    cout << msg << endl;
}

// Class to encapsulate a context-free grammar.
class ContextFreeGrammar {
public:
    vector<ProductionRule> ruleList;
    vector<string> symbols;
    vector<string> nonTerminals;
    vector<string> terminals;
    unordered_map<string, vector<vector<string>>> ruleMap;
    unordered_map<string, set<string>> firstSet;
    unordered_map<string, set<string>> followSet;

    // Adds a string to a vector only if it isn't already present.
    void addUniqueSymbol(vector<string> &collection, const string &sym) {
        if (find(collection.begin(), collection.end(), sym) == collection.end())
            collection.push_back(sym);
    }

    // Adds a production to the grammar.
    void addProduction(const string &head, const vector<string> &body) {
        ProductionRule pr { head, body };
        ruleList.push_back(pr);
        addUniqueSymbol(symbols, head);
        ruleMap[head].push_back(body);
        for (const string &s : body) {
            addUniqueSymbol(symbols, s);
        }
    }

    // Classifies all symbols into non-terminals and terminals.
    void classifySymbols() {
        set<string> ntSet;
        for (const ProductionRule &pr : ruleList) {
            ntSet.insert(pr.head);
        }
        nonTerminals.clear();
        terminals.clear();
        for (const string &s : symbols) {
            if (ntSet.find(s) != ntSet.end())
                addUniqueSymbol(nonTerminals, s);
            else
                addUniqueSymbol(terminals, s);
        }
    }

    // Computes nullable flags for each non-terminal.
    unordered_map<string, bool> computeNullableFlags() {
        unordered_map<string, bool> nullable;
        for (const string &nt : nonTerminals)
            nullable[nt] = false;
        bool updated = true;
        while (updated) {
            updated = false;
            for (const string &nt : nonTerminals) {
                if (!nullable[nt]) {
                    for (const vector<string> &prod : ruleMap[nt]) {
                        if (isNullable(prod, nullable)) {
                            nullable[nt] = true;
                            updated = true;
                            break;
                        }
                    }
                }
            }
        }
        return nullable;
    }

    // Determines whether a given production is nullable.
    bool isNullable(const vector<string> &prod, const unordered_map<string, bool> &nullable) {
        if (prod.empty())
            return true;
        for (const string &sym : prod) {
            bool isNonTerminal = false;
            for (const string &nt : nonTerminals) {
                if (sym == nt) { isNonTerminal = true; break; }
            }
            if (!isNonTerminal)
                return false;
            if (!nullable.at(sym))
                return false;
        }
        return true;
    }
};

ContextFreeGrammar cfg; // Global grammar instance

// --------------------- Parsing Functions ---------------------------

// Recursively parse a list of identifiers; stops when a STAR or OR token is seen.
void parseIdentifierList(LexicalAnalyzer &lexer, vector<string> &idList, int recursionDepth = 0) {
    if (recursionDepth > 1000) {
        cout << "SYNTAX ERROR !!!!!!!!!!!!!!" << endl;
        exit(1);
    }
    Token token = lexer.peek(1);
    if (token.token_type == END_OF_FILE) {
        cout << "SYNTAX ERROR !!!!!!!!!!!!!!" << endl;
        exit(1);
    }
    if (token.token_type == STAR || token.token_type == OR)
        return;
    token = lexer.GetToken();
    if (token.token_type != ID) {
        cout << "SYNTAX ERROR !!!!!!!!!!!!!!" << endl;
        exit(1);
    }
    idList.push_back(token.lexeme);
    parseIdentifierList(lexer, idList, recursionDepth + 1);
}

// Parse the right-hand side (RHS) of a production.
void parseRHS(LexicalAnalyzer &lexer, const string &head) {
    vector<string> body;
    parseIdentifierList(lexer, body, 0);
    Token token = lexer.GetToken();
    if (token.token_type == OR) {
        cfg.addProduction(head, body);
        parseRHS(lexer, head);
    } else if (token.token_type == STAR) {
        cfg.addProduction(head, body);
    } else {
        cout << "SYNTAX ERROR !!!!!!!!!!!!!!" << endl;
        exit(1);
    }
}

// Parse a single production rule.
void parseProductionRule(LexicalAnalyzer &lexer) {
    Token token = lexer.GetToken();
    if (token.token_type != ID) {
        cout << "SYNTAX ERROR !!!!!!!!!!!!!!" << endl;
        exit(1);
    }
    string head = token.lexeme;
    token = lexer.GetToken();
    if (token.token_type != ARROW) {
        cout << "SYNTAX ERROR !!!!!!!!!!!!!!" << endl;
        exit(1);
    }
    parseRHS(lexer, head);
}

// Recursively parse all production rules.
void parseRules(LexicalAnalyzer &lexer) {
    Token token = lexer.peek(1);
    if (token.token_type == HASH)
        return;
    parseProductionRule(lexer);
    parseRules(lexer);
}

// Entry point for grammar parsing.
void parseCFG(LexicalAnalyzer &lexer) {
    Token token = lexer.peek(1);
    if (token.token_type == END_OF_FILE || token.token_type == HASH) {
        cout << "SYNTAX ERROR !!!!!!!!!!!!!!" << endl;
        return;
    }
    parseRules(lexer);
    token = lexer.GetToken();
    if (token.token_type != HASH) {
        cout << "SYNTAX ERROR !!!!!!!!!!!!!!" << endl;
        return;
    }
    token = lexer.GetToken();
    if (token.token_type != END_OF_FILE) {
        cout << "SYNTAX ERROR !!!!!!!!!!!!!!" << endl;
        return;
    }
}

// Loads the grammar from standard input.
void loadCFG(ContextFreeGrammar &cfg) {
    LexicalAnalyzer lexer;
    parseCFG(lexer);
    cfg.classifySymbols();
}

// --------------------- Operations on the Grammar ---------------------------

/*
 * Operation 1: Print the list of terminals followed by the list of non-terminals.
 */
void printSymbols() {
    for (size_t i = 0; i < cfg.terminals.size(); ++i) {
        cout << cfg.terminals[i];
        if (i < cfg.terminals.size() - 1 || !cfg.nonTerminals.empty())
            cout << " ";
    }
    for (size_t i = 0; i < cfg.nonTerminals.size(); ++i) {
        cout << cfg.nonTerminals[i];
        if (i < cfg.nonTerminals.size() - 1)
            cout << " ";
    }
}

/*
 * Operation 2: Display the set of nullable non-terminals.
 */
void showNullable() {
    unordered_map<string, bool> nullableFlags = cfg.computeNullableFlags();
    cout << "Nullable = {";
    bool firstPrinted = true;
    for (const string &nt : cfg.nonTerminals) {
        if (nullableFlags[nt]) {
            if (!firstPrinted)
                cout << ", ";
            else
                cout << " ";
            cout << nt;
            firstPrinted = false;
        }
    }
    if (firstPrinted)
        cout << " }";
    else
        cout << " }" << endl;
}

/*
 * Operation 3: Compute and print FIRST sets for non-terminals.
 */
void displayFirstSets() {
    unordered_map<string, bool> nullableFlags = cfg.computeNullableFlags();
    unordered_map<string, set<string>> first;
    for (const string &nt : cfg.nonTerminals)
        first[nt] = set<string>();

    bool changeOccurred = true;
    while (changeOccurred) {
        changeOccurred = false;
        for (const string &A : cfg.nonTerminals) {
            for (const vector<string> &prod : cfg.ruleMap[A]) {
                bool allNullable = true;
                if (prod.empty()) {
                    if (first[A].find("EPSILON") == first[A].end()) {
                        first[A].insert("EPSILON");
                        changeOccurred = true;
                    }
                    continue;
                }
                for (const string &symbol : prod) {
                    bool isNT = false;
                    for (const string &nt : cfg.nonTerminals) {
                        if (symbol == nt) { isNT = true; break; }
                    }
                    if (!isNT) {
                        if (first[A].find(symbol) == first[A].end()) {
                            first[A].insert(symbol);
                            changeOccurred = true;
                        }
                        allNullable = false;
                        break;
                    } else {
                        for (const string &s : first[symbol]) {
                            if (s != "EPSILON" && first[A].find(s) == first[A].end()) {
                                first[A].insert(s);
                                changeOccurred = true;
                            }
                        }
                        if (!nullableFlags[symbol]) {
                            allNullable = false;
                            break;
                        }
                    }
                }
                if (allNullable) {
                    if (first[A].find("EPSILON") == first[A].end()) {
                        first[A].insert("EPSILON");
                        changeOccurred = true;
                    }
                }
            }
        }
    }
    for (const string &A : cfg.nonTerminals) {
        cout << "FIRST(" << A << ") = {";
        bool printedAny = false;
        for (const string &t : cfg.terminals) {
            if (first[A].find(t) != first[A].end()) {
                if (printedAny)
                    cout << ",";
                cout << " " << t;
                printedAny = true;
            }
        }
        cout << " }" << endl;
    }
}

/*
 * Operation 4: Compute and print FOLLOW sets for non-terminals.
 */
void displayFollowSets() {
    unordered_map<string, bool> nullableFlags = cfg.computeNullableFlags();
    unordered_map<string, set<string>> first;
    for (const string &nt : cfg.nonTerminals)
        first[nt] = set<string>();

    bool changed = true;
    while (changed) {
        changed = false;
        for (const string &A : cfg.nonTerminals) {
            for (const vector<string> &prod : cfg.ruleMap[A]) {
                bool allNullable = true;
                if (prod.empty()) {
                    if (first[A].find("EPSILON") == first[A].end()) {
                        first[A].insert("EPSILON");
                        changed = true;
                    }
                    continue;
                }
                for (const string &symbol : prod) {
                    bool isNT = false;
                    for (const string &nt : cfg.nonTerminals) {
                        if (symbol == nt) { isNT = true; break; }
                    }
                    if (!isNT) {
                        if (first[A].find(symbol) == first[A].end()) {
                            first[A].insert(symbol);
                            changed = true;
                        }
                        allNullable = false;
                        break;
                    } else {
                        for (const string &t : cfg.terminals) {
                            if (first[symbol].find(t) != first[symbol].end() && first[A].find(t) == first[A].end()) {
                                first[A].insert(t);
                                changed = true;
                            }
                        }
                        if (first[symbol].find("EPSILON") == first[symbol].end()) {
                            allNullable = false;
                            break;
                        }
                    }
                }
                if (allNullable) {
                    if (first[A].find("EPSILON") == first[A].end()) {
                        first[A].insert("EPSILON");
                        changed = true;
                    }
                }
            }
        }
    }

    unordered_map<string, set<string>> follow;
    for (const string &nt : cfg.nonTerminals)
        follow[nt] = set<string>();
    if (!cfg.nonTerminals.empty())
        follow[cfg.nonTerminals[0]].insert("$");

    bool updated = true;
    while (updated) {
        updated = false;
        for (const string &A : cfg.nonTerminals) {
            for (const vector<string> &prod : cfg.ruleMap[A]) {
                for (size_t i = 0; i < prod.size(); i++) {
                    string curr = prod[i];
                    bool isNT = false;
                    for (const string &nt : cfg.nonTerminals) {
                        if (curr == nt) { isNT = true; break; }
                    }
                    if (!isNT)
                        continue;
                    set<string> firstOfBeta;
                    bool betaNullable = true;
                    for (size_t j = i + 1; j < prod.size(); j++) {
                        string nextSym = prod[j];
                        bool nextNT = false;
                        for (const string &nt : cfg.nonTerminals) {
                            if (nextSym == nt) { nextNT = true; break; }
                        }
                        if (!nextNT) {
                            firstOfBeta.insert(nextSym);
                            betaNullable = false;
                            break;
                        } else {
                            for (const string &t : cfg.terminals) {
                                if (first[nextSym].find(t) != first[nextSym].end())
                                    firstOfBeta.insert(t);
                            }
                            if (first[nextSym].find("EPSILON") == first[nextSym].end()) {
                                betaNullable = false;
                                break;
                            }
                        }
                    }
                    for (const string &t : cfg.terminals) {
                        if (firstOfBeta.find(t) != firstOfBeta.end() && follow[curr].find(t) == follow[curr].end()) {
                            follow[curr].insert(t);
                            updated = true;
                        }
                    }
                    if ((i == prod.size() - 1) || betaNullable) {
                        if (follow[A].find("$") != follow[A].end() && follow[curr].find("$") == follow[curr].end()) {
                            follow[curr].insert("$");
                            updated = true;
                        }
                        for (const string &t : cfg.terminals) {
                            if (follow[A].find(t) != follow[A].end() && follow[curr].find(t) == follow[curr].end()) {
                                follow[curr].insert(t);
                                updated = true;
                            }
                        }
                    }
                }
            }
        }
    }

    for (const string &nt : cfg.nonTerminals) {
        cout << "FOLLOW(" << nt << ") = {";
        bool printed = false;
        if (follow[nt].find("$") != follow[nt].end()) {
            cout << " $";
            printed = true;
        }
        for (const string &t : cfg.terminals) {
            if (follow[nt].find(t) != follow[nt].end()) {
                if (printed)
                    cout << ",";
                cout << " " << t;
                printed = true;
            }
        }
        cout << " }" << endl;
    }
}

/*
 * Utility: Find the longest common prefix between two productions.
 */
vector<string> commonPrefix(const vector<string>& a, const vector<string>& b) {
    vector<string> prefix;
    size_t lim = min(a.size(), b.size());
    for (size_t i = 0; i < lim; i++) {
        if (a[i] == b[i])
            prefix.push_back(a[i]);
        else
            break;
    }
    return prefix;
}

/*
 * Utility: Check if a production begins with a given prefix.
 */
bool beginsWith(const vector<string>& prod, const vector<string>& prefix) {
    if (prefix.size() > prod.size())
        return false;
    for (size_t i = 0; i < prefix.size(); i++) {
        if (prod[i] != prefix[i])
            return false;
    }
    return true;
}

/*
 * Lexicographic comparison of two string sequences.
 */
bool lexicographicCompare(const vector<string>& seq1, const vector<string>& seq2) {
    size_t commonLength = min(seq1.size(), seq2.size());
    for (size_t i = 0; i < commonLength; i++) {
        if (seq1[i] < seq2[i])
            return true;
        else if (seq1[i] > seq2[i])
            return false;
    }
    return (seq1.size() < seq2.size());
}

/*
 * Operation 5: Perform left factoring on the grammar.
 */
void performLeftFactoring() {
    unordered_map<string, vector<vector<string>>> factored = cfg.ruleMap;
    queue<string> ntQueue;
    for (const string &nt : cfg.nonTerminals)
        ntQueue.push(nt);
    unordered_map<string, int> newNTCount;
    for (const string &nt : cfg.nonTerminals)
        newNTCount[nt] = 1;
    
    while (!ntQueue.empty()) {
        string currNT = ntQueue.front();
        ntQueue.pop();
        bool refactored = true;
        while (refactored) {
            refactored = false;
            vector<string> bestCommon;
            int total = factored[currNT].size();
            for (int i = 0; i < total; i++) {
                for (int j = i + 1; j < total; j++) {
                    vector<string> prefix = commonPrefix(factored[currNT][i], factored[currNT][j]);
                    if (!prefix.empty()) {
                        if (prefix.size() > bestCommon.size() ||
                           (prefix.size() == bestCommon.size() && lexicographicCompare(prefix, bestCommon))) {
                            bestCommon = prefix;
                        }
                    }
                }
            }
            if (bestCommon.empty())
                break;
            vector<vector<string>> group, remainder;
            for (const auto &prod : factored[currNT]) {
                if (beginsWith(prod, bestCommon))
                    group.push_back(prod);
                else
                    remainder.push_back(prod);
            }
            if (group.size() < 2)
                break;
            string newNT = currNT + to_string(newNTCount[currNT]++);
            ntQueue.push(newNT);
            vector<vector<string>> newGroup;
            for (const auto &prod : group) {
                vector<string> tail(prod.begin() + bestCommon.size(), prod.end());
                newGroup.push_back(tail);
            }
            factored[currNT] = remainder;
            vector<string> newProd = bestCommon;
            newProd.push_back(newNT);
            factored[currNT].push_back(newProd);
            factored[newNT] = newGroup;
            refactored = true;
        }
    }
    
    vector<ProductionRule> outRules;
    for (auto &entry : factored) {
        string head = entry.first;
        for (auto &body : entry.second) {
            ProductionRule pr;
            pr.head = head;
            pr.body = body;
            outRules.push_back(pr);
        }
    }
    
    auto ruleComparator = [&](const ProductionRule &r1, const ProductionRule &r2) {
        if (r1.head != r2.head)
            return r1.head < r2.head;
        size_t len = min(r1.body.size(), r2.body.size());
        for (size_t i = 0; i < len; i++) {
            if (r1.body[i] != r2.body[i])
                return r1.body[i] < r2.body[i];
        }
        return r1.body.size() < r2.body.size();
    };
    
    sort(outRules.begin(), outRules.end(), ruleComparator);
    
    for (const ProductionRule &pr : outRules) {
        cout << pr.head << " ->";
        if (!pr.body.empty()) {
            for (const string &s : pr.body)
                cout << " " << s;
        }
        cout << " #" << endl;
    }
}

/*
 * Operation 6: Eliminate left recursion from the grammar.
 * Assumes no epsilon rules and no cycles exist.
 */
void removeLeftRecursion() {
    unordered_map<string, vector<vector<string>>> P = cfg.ruleMap;
    vector<string> NTList = cfg.nonTerminals;
    sort(NTList.begin(), NTList.end());
    
    // Remove indirect left recursion.
    for (int i = 0; i < NTList.size(); i++) {
        string A = NTList[i];
        for (int j = 0; j < i; j++) {
            string B = NTList[j];
            vector<vector<string>> updated;
            for (auto &prod : P[A]) {
                if (!prod.empty() && prod[0] == B) {
                    for (auto &delta : P[B]) {
                        vector<string> newProd;
                        newProd.insert(newProd.end(), delta.begin(), delta.end());
                        newProd.insert(newProd.end(), prod.begin() + 1, prod.end());
                        updated.push_back(newProd);
                    }
                } else {
                    updated.push_back(prod);
                }
            }
            P[A] = updated;
        }
        
        // Eliminate immediate left recursion.
        vector<vector<string>> alpha, beta;
        for (auto &prod : P[A]) {
            if (!prod.empty() && prod[0] == A) {
                vector<string> remainder(prod.begin() + 1, prod.end());
                alpha.push_back(remainder);
            } else {
                beta.push_back(prod);
            }
        }
        if (!alpha.empty()) {
            string newNT = A + "1";
            vector<vector<string>> newBeta;
            for (auto &b : beta) {
                vector<string> temp = b;
                temp.push_back(newNT);
                newBeta.push_back(temp);
            }
            P[A] = newBeta;
            vector<vector<string>> newAlpha;
            for (auto &a : alpha) {
                vector<string> temp = a;
                temp.push_back(newNT);
                newAlpha.push_back(temp);
            }
            newAlpha.push_back(vector<string>());
            P[newNT] = newAlpha;
        }
    }
    
    vector<ProductionRule> finalRules;
    for (auto &entry : P) {
        string head = entry.first;
        for (auto &body : entry.second) {
            ProductionRule pr;
            pr.head = head;
            pr.body = body;
            finalRules.push_back(pr);
        }
    }
    
    auto finalComparator = [&](const ProductionRule &r1, const ProductionRule &r2) {
        if (r1.head != r2.head)
            return r1.head < r2.head;
        size_t len = min(r1.body.size(), r2.body.size());
        for (size_t i = 0; i < len; i++) {
            if (r1.body[i] != r2.body[i])
                return r1.body[i] < r2.body[i];
        }
        return r1.body.size() < r2.body.size();
    };
    
    sort(finalRules.begin(), finalRules.end(), finalComparator);
    
    for (const ProductionRule &pr : finalRules) {
        cout << pr.head << " ->";
        if (!pr.body.empty()) {
            for (const string &s : pr.body)
                cout << " " << s;
        }
        cout << " #" << endl;
    }
}

int main(int argc, char *argv[]) {
    int operation;
    if (argc < 2) {
        cout << "Error: missing argument\n";
        return 1;
    }
    operation = atoi(argv[1]);
    loadCFG(cfg);
    switch (operation) {
        case 1:
            printSymbols();
            break;
        case 2:
            showNullable();
            break;
        case 3:
            displayFirstSets();
            break;
        case 4:
            displayFollowSets();
            break;
        case 5:
            performLeftFactoring();
            break;
        case 6:
            removeLeftRecursion();
            break;
        default:
            cout << "Error: unrecognized task number " << operation << "\n";
            break;
    }
    return 0;
}
