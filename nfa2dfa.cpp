#include <stdio.h>
#include <stdlib.h>

// NFA��״̬�ṹ
typedef struct NFAState {
    int id; // ״̬ID
    int isFinal; // �Ƿ�Ϊ��̬
    struct Transition* transitions; // ת�ƹ�ϵ����
    struct NFAState* next; // ����״̬���ϵ�����
} NFAState;

// ת�ƹ�ϵ�ṹ
typedef struct Transition {
    NFAState* to; // Ŀ��״̬
    char symbol;  // ת�Ʒ��ţ�0����epsilon
    struct Transition* next; // ��һ��ת��
} Transition;

// DFA��״̬�ṹ
typedef struct DFAState {
    NFAState* nfaStates; // NFA״̬����
    int marked; // �Ƿ��ѱ��
    struct DFAState* next; // ��һ��DFA״̬
    struct DFATransition* transitions; // DFAת�ƹ�ϵ
} DFAState;

// DFA��ת�ƹ�ϵ�ṹ
typedef struct DFATransition {
    DFAState* to; // Ŀ��״̬
    char symbol; // ת�Ʒ���
    struct DFATransition* next; // ��һ��ת��
} DFATransition;

// ��ջ�ṹ�����ڼ���e-closure
typedef struct Stack {
    NFAState* state;
    struct Stack* next;
} Stack;

// ��ջ����������
void push(Stack** s, NFAState* state) {
    Stack* newNode = malloc(sizeof(Stack));
    newNode->state = state;
    newNode->next = *s;
    *s = newNode;
}

// ��ջ����������
NFAState* pop(Stack** s) {
    if (!*s) return NULL;
    NFAState* ret = (*s)->state;
    Stack* temp = *s;
    *s = (*s)->next;
    free(temp);
    return ret;
}

// ����e-closure
NFAState* eClosure(NFAState* T) {
    Stack* s = NULL;
    NFAState* eClosureResult = NULL;
    NFAState* current = T;

    // ��T�е�����״̬�����ջ
    while (current) {
        push(&s, current);
        current = current->next;
    }

    while (s) {
        NFAState* t = pop(&s);
        // ���t����eClosureResult�У������
        if (!inStateList(eClosureResult, t)) {
            addToStateList(&eClosureResult, t);
            Transition* trans = t->transitions;
            while (trans) {
                if (trans->symbol == 0 && !inStateList(eClosureResult, trans->to)) { // epsilonת��
                    push(&s, trans->to);
                }
                trans = trans->next;
            }
        }
    }
    return eClosureResult;
}

// NFA��DFA���Ӽ����취
DFAState* subsetConstruction(NFAState* start) {
    // ��ʼ��
    DFAState* Dstates = NULL;
    addToDFAStateList(&Dstates, eClosure(start));

    DFAState* currentDFA = Dstates;
    while (currentDFA && !currentDFA->marked) {
        currentDFA->marked = 1;

        // ����ÿ���������a
        for (char a = 'a'; a <= 'z'; a++) {  // ������ż�Ϊa-z
            NFAState* U = move(currentDFA->nfaStates, a);
            U = eClosure(U);

            if (!inDFAStateList(Dstates, U)) {
                addToDFAStateList(&Dstates, U);
            }
            addDFATransition(currentDFA, U, a);
        }
        currentDFA = currentDFA->next;
    }
    return Dstates;
}

// ������������

// ���״̬��״̬����
void addToStateList(NFAState** list, NFAState* state) {
    NFAState* newNode = malloc(sizeof(NFAState));
    *newNode = *state; // ǳ����
    newNode->next = *list;
    *list = newNode;
}

// ���״̬�Ƿ���״̬������
int inStateList(NFAState* list, NFAState* state) {
    while (list) {
        if (list->id == state->id) return 1;
        list = list->next;
    }
    return 0;
}

// �����ڸ��������µ�ת��״̬
NFAState* move(NFAState* T, char symbol) {
    NFAState* result = NULL;
    while (T) {
        Transition* trans = T->transitions;
        while (trans) {
            if (trans->symbol == symbol && !inStateList(result, trans->to)) {
                addToStateList(&result, trans->to);
            }
            trans = trans->next;
        }
        T = T->next;
    }
    return result;
}

// DFA״̬������

// ���DFA״̬��״̬����
void addToDFAStateList(DFAState** list, NFAState* nfaStates) {
    DFAState* newDFAState = malloc(sizeof(DFAState));
    newDFAState->nfaStates = nfaStates;
    newDFAState->marked = 0;
    newDFAState->transitions = NULL;
    newDFAState->next = *list;
    *list = newDFAState;
}

// ���DFA״̬�Ƿ���״̬������
int inDFAStateList(DFAState* list, NFAState* nfaStates) {
    while (list) {
        NFAState* stateList1 = list->nfaStates;
        NFAState* stateList2 = nfaStates;
        while (stateList1 && stateList2 && stateList1->id == stateList2->id) {
            stateList1 = stateList1->next;
            stateList2 = stateList2->next;
        }
        if (!stateList1 && !stateList2) return 1; // �ҵ���ͬ��DFA״̬
        list = list->next;
    }
    return 0;
}

// ���DFAת��
void addDFATransition(DFAState* from, DFAState* to, char symbol) {
    DFATransition* newTrans = malloc(sizeof(DFATransition));
    newTrans->to = to;
    newTrans->symbol = symbol;
    newTrans->next = from->transitions;
    from->transitions = newTrans;
}

void printDFA(DFAState* start) {
    if (!start) return;

    printf("DFA States and their transitions:\n");
    DFAState* curr = start;
    while (curr) {
        NFAState* nfaState = curr->nfaStates;
        printf("DFA State derived from NFA states: ");
        while (nfaState) {
            printf("%d ", nfaState->id);
            nfaState = nfaState->next;
        }
        printf("\n");
        DFATransition* trans = curr->transitions;
        while (trans) {
            NFAState* nfaToState = trans->to->nfaStates;
            printf("  On symbol %c to DFA state from NFA states: ", trans->symbol);
            while (nfaToState) {
                printf("%d ", nfaToState->id);
                nfaToState = nfaToState->next;
            }
            printf("\n");
            trans = trans->next;
        }
        curr = curr->next;
    }
}

int main() {
    // ����һ���򵥵�NFA��a|b
    NFAState* start = malloc(sizeof(NFAState));
    NFAState* aState = malloc(sizeof(NFAState));
    NFAState* bState = malloc(sizeof(NFAState));
    NFAState* finalState = malloc(sizeof(NFAState));

    start->id = 0; aState->id = 1; bState->id = 2; finalState->id = 3;
    start->isFinal = 0; aState->isFinal = 0; bState->isFinal = 0; finalState->isFinal = 1;
    start->transitions = NULL; aState->transitions = NULL; bState->transitions = NULL; finalState->transitions = NULL;
    start->next = aState; aState->next = bState; bState->next = finalState; finalState->next = NULL;

    // ���NFA��ת��
    addTransition(&start->transitions, start, aState, 'a');
    addTransition(&start->transitions, start, bState, 'b');
    addTransition(&aState->transitions, aState, finalState, 'e');  // epsilon transition
    addTransition(&bState->transitions, bState, finalState, 'e');  // epsilon transition

    // ����subsetConstruction����NFA��DFA��ת��
    DFAState* dfaStart = subsetConstruction(start);

    // ��ӡDFA
    printDFA(dfaStart);

    return 0;
}