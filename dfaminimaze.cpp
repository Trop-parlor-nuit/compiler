#include <iostream>
#include <vector>
#include <map>
#include <set>
#include <algorithm>

using namespace std;

class DFA {
public:
    set<int> states;             // All states.
    map<pair<int, char>, int> transition;  // Transition function.
    int startState;
    set<int> acceptStates;

    DFA(set<int> states, map<pair<int, char>, int> transition, int startState, set<int> acceptStates)
        : states(states), transition(transition), startState(startState), acceptStates(acceptStates) {}
};


set<set<int>> split(const set<int>& group, const DFA& dfa, const set<set<int>>& partitions, const vector<char>& alphabet) {
    map<int, int> signature;

    for (int state : group) {
        int sig = 0;
        for (char a : alphabet) {
            int to = dfa.transition.at({ state, a });
            int idx = 0;
            for (const set<int>& part : partitions) {
                if (part.find(to) != part.end()) {
                    sig |= (1 << idx);
                    break;
                }
                idx++;
            }
        }
        signature[state] = sig;
    }

    set<set<int>> newGroups;
    map<int, set<int>> groupBySig;
    for (auto [state, sig] : signature) {
        groupBySig[sig].insert(state);
    }
    for (auto [_, g] : groupBySig) {
        newGroups.insert(g);
    }

    return newGroups;
}

DFA minimizeDFA(const DFA& dfa, const vector<char>& alphabet) {
    set<set<int>> partitions;
    partitions.insert(dfa.acceptStates);
    set<int> nonAcceptStates;

    // Initialize nonAcceptStates as states - acceptStates
    std::set_difference(dfa.states.begin(), dfa.states.end(), dfa.acceptStates.begin(), dfa.acceptStates.end(), std::inserter(nonAcceptStates, nonAcceptStates.begin()));
    partitions.insert(nonAcceptStates);

    while (true) {
        set<set<int>> newPartitions;
        for (const set<int>& group : partitions) {
            set<set<int>> splitted = split(group, dfa, partitions, alphabet);
            newPartitions.insert(splitted.begin(), splitted.end());
        }
        if (newPartitions == partitions) break;
        partitions = newPartitions;
    }

    map<int, int> oldStateToNewState;
    int newState = 0;
    for (const set<int>& group : partitions) {
        for (int oldState : group) {
            oldStateToNewState[oldState] = newState;
        }
        newState++;
    }

    map<pair<int, char>, int> newTransition;
    for (auto [key, value] : dfa.transition) {
        int oldSource = key.first;
        char symbol = key.second;
        int oldTarget = value;
        newTransition[{oldStateToNewState[oldSource], symbol}] = oldStateToNewState[oldTarget];
    }

    set<int> newAcceptStates;
    for (int oldAcceptState : dfa.acceptStates) {
        newAcceptStates.insert(oldStateToNewState[oldAcceptState]);
    }

    return DFA({ 0, 1, 2, 3 }, newTransition, oldStateToNewState[dfa.startState], newAcceptStates);
}

int main() {
    set<int> states = { 0, 1, 2, 3 };
    map<pair<int, char>, int> transition = {
        {{0, 'a'}, 1},
        {{0, 'b'}, 2},
        {{1, 'a'}, 3},
        {{1, 'b'}, 2},
        {{2, 'a'}, 1},
        {{2, 'b'}, 3},
        {{3, 'a'}, 3},
        {{3, 'b'}, 3}
    };
    int startState = 0;
    set<int> acceptStates = { 3 };

    DFA dfa(states, transition, startState, acceptStates);

    DFA minimizedDfa = minimizeDFA(dfa, { 'a', 'b' });

    cout << "States: ";
    for (int state : minimizedDfa.states) {
        cout << state << " ";
    }
    cout << "\nAccept States: ";
    for (int acceptState : minimizedDfa.acceptStates) {
        cout << acceptState << " ";
    }

    return 0;
}

}