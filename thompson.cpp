#include <stdio.h>
#include <stdlib.h>

typedef struct State {
    int isFinal;  // �Ƿ�Ϊ��ֹ״̬��1��ʾ�ǣ�0��ʾ��
    struct Transition* trans; // Pointing to transitions list
} State;

typedef struct Transition {
    State* from;    // ��ʼ״̬
    State* to;      // ��ֹ״̬
    char symbol;    // ��������״̬֮��ת��ʱҪ��ȡ���ַ�
    struct Transition* next;  // ���ӵ���һ��ת����ָ��
} Transition;

typedef struct NFA {
    State* start;
    State* end;
    Transition* transitions; // ת�����б�
} NFA;

void addTransition(Transition** transitions, State* from, State* to, char symbol) {
    Transition* t = malloc(sizeof(Transition));
    t->from = from;
    t->to = to;
    t->symbol = symbol;
    t->next = *transitions;
    *transitions = t;
}

NFA createNFAWithSymbol(char symbol) {
    NFA newNFA;
    State* start = malloc(sizeof(State));
    State* end = malloc(sizeof(State));

    start->isFinal = 0;
    start->trans = NULL;
    end->isFinal = 1;
    end->trans = NULL;

    addTransition(&(newNFA.transitions), start, end, symbol);

    newNFA.start = start;
    newNFA.end = end;
    return newNFA;
}

NFA unionOp(NFA nfa1, NFA nfa2) {
    NFA newNFA;
    State* newStart = malloc(sizeof(State));
    State* newEnd = malloc(sizeof(State));

    newStart->isFinal = 0;
    newStart->trans = NULL;
    newEnd->isFinal = 1;
    newEnd->trans = NULL;

    addTransition(&(newNFA.transitions), newStart, nfa1.start, 0);
    addTransition(&(newNFA.transitions), newStart, nfa2.start, 0);
    addTransition(&(newNFA.transitions), nfa1.end, newEnd, 0);
    addTransition(&(newNFA.transitions), nfa2.end, newEnd, 0);

    newNFA.start = newStart;
    newNFA.end = newEnd;
    return newNFA;
}

NFA concatOp(NFA nfa1, NFA nfa2) {
    addTransition(&(nfa1.transitions), nfa1.end, nfa2.start, 0);
    nfa1.end = nfa2.end;
    return nfa1;
}

NFA closureOp(NFA nfa) {
    NFA newNFA;
    State* newStart = malloc(sizeof(State));
    State* newEnd = malloc(sizeof(State));

    newStart->isFinal = 0;
    newStart->trans = NULL;
    newEnd->isFinal = 1;
    newEnd->trans = NULL;

    addTransition(&(newNFA.transitions), newStart, nfa.start, 0);
    addTransition(&(newNFA.transitions), newStart, newEnd, 0);
    addTransition(&(newNFA.transitions), nfa.end, nfa.start, 0);
    addTransition(&(newNFA.transitions), nfa.end, newEnd, 0);

    newNFA.start = newStart;
    newNFA.end = newEnd;
    return newNFA;
}



NFA thompsonConstruct(char* regex) {
    return expression(&regex);
}


int main() {
    char* regex = "a|b*";
    NFA result = thompsonConstruct(regex);
    printf("Constructed NFA for regex: %s\n", regex);

    return 0;
}
