#include <stdio.h>
#include <stdlib.h>

// NFA的状态结构
typedef struct NFAState {
    int id; // 状态ID
    int isFinal; // 是否为终态
    struct Transition* transitions; // 转移关系链表
    struct NFAState* next; // 用于状态集合的链表
} NFAState;

// 转移关系结构
typedef struct Transition {
    NFAState* to; // 目标状态
    char symbol;  // 转移符号，0代表epsilon
    struct Transition* next; // 下一个转移
} Transition;

// DFA的状态结构
typedef struct DFAState {
    NFAState* nfaStates; // NFA状态集合
    int marked; // 是否已标记
    struct DFAState* next; // 下一个DFA状态
    struct DFATransition* transitions; // DFA转移关系
} DFAState;

// DFA的转移关系结构
typedef struct DFATransition {
    DFAState* to; // 目标状态
    char symbol; // 转移符号
    struct DFATransition* next; // 下一个转移
} DFATransition;

// 堆栈结构，用于计算e-closure
typedef struct Stack {
    NFAState* state;
    struct Stack* next;
} Stack;

// 堆栈操作：推入
void push(Stack** s, NFAState* state) {
    Stack* newNode = malloc(sizeof(Stack));
    newNode->state = state;
    newNode->next = *s;
    *s = newNode;
}

// 堆栈操作：弹出
NFAState* pop(Stack** s) {
    if (!*s) return NULL;
    NFAState* ret = (*s)->state;
    Stack* temp = *s;
    *s = (*s)->next;
    free(temp);
    return ret;
}

// 计算e-closure
NFAState* eClosure(NFAState* T) {
    Stack* s = NULL;
    NFAState* eClosureResult = NULL;
    NFAState* current = T;

    // 将T中的所有状态推入堆栈
    while (current) {
        push(&s, current);
        current = current->next;
    }

    while (s) {
        NFAState* t = pop(&s);
        // 如果t不在eClosureResult中，则添加
        if (!inStateList(eClosureResult, t)) {
            addToStateList(&eClosureResult, t);
            Transition* trans = t->transitions;
            while (trans) {
                if (trans->symbol == 0 && !inStateList(eClosureResult, trans->to)) { // epsilon转移
                    push(&s, trans->to);
                }
                trans = trans->next;
            }
        }
    }
    return eClosureResult;
}

// NFA到DFA的子集构造法
DFAState* subsetConstruction(NFAState* start) {
    // 初始化
    DFAState* Dstates = NULL;
    addToDFAStateList(&Dstates, eClosure(start));

    DFAState* currentDFA = Dstates;
    while (currentDFA && !currentDFA->marked) {
        currentDFA->marked = 1;

        // 对于每个输入符号a
        for (char a = 'a'; a <= 'z'; a++) {  // 假设符号集为a-z
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

// 辅助函数部分

// 添加状态到状态链表
void addToStateList(NFAState** list, NFAState* state) {
    NFAState* newNode = malloc(sizeof(NFAState));
    *newNode = *state; // 浅复制
    newNode->next = *list;
    *list = newNode;
}

// 检查状态是否在状态链表中
int inStateList(NFAState* list, NFAState* state) {
    while (list) {
        if (list->id == state->id) return 1;
        list = list->next;
    }
    return 0;
}

// 计算在给定符号下的转移状态
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

// DFA状态管理函数

// 添加DFA状态到状态链表
void addToDFAStateList(DFAState** list, NFAState* nfaStates) {
    DFAState* newDFAState = malloc(sizeof(DFAState));
    newDFAState->nfaStates = nfaStates;
    newDFAState->marked = 0;
    newDFAState->transitions = NULL;
    newDFAState->next = *list;
    *list = newDFAState;
}

// 检查DFA状态是否在状态链表中
int inDFAStateList(DFAState* list, NFAState* nfaStates) {
    while (list) {
        NFAState* stateList1 = list->nfaStates;
        NFAState* stateList2 = nfaStates;
        while (stateList1 && stateList2 && stateList1->id == stateList2->id) {
            stateList1 = stateList1->next;
            stateList2 = stateList2->next;
        }
        if (!stateList1 && !stateList2) return 1; // 找到相同的DFA状态
        list = list->next;
    }
    return 0;
}

// 添加DFA转移
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
    // 创建一个简单的NFA：a|b
    NFAState* start = malloc(sizeof(NFAState));
    NFAState* aState = malloc(sizeof(NFAState));
    NFAState* bState = malloc(sizeof(NFAState));
    NFAState* finalState = malloc(sizeof(NFAState));

    start->id = 0; aState->id = 1; bState->id = 2; finalState->id = 3;
    start->isFinal = 0; aState->isFinal = 0; bState->isFinal = 0; finalState->isFinal = 1;
    start->transitions = NULL; aState->transitions = NULL; bState->transitions = NULL; finalState->transitions = NULL;
    start->next = aState; aState->next = bState; bState->next = finalState; finalState->next = NULL;

    // 添加NFA的转移
    addTransition(&start->transitions, start, aState, 'a');
    addTransition(&start->transitions, start, bState, 'b');
    addTransition(&aState->transitions, aState, finalState, 'e');  // epsilon transition
    addTransition(&bState->transitions, bState, finalState, 'e');  // epsilon transition

    // 调用subsetConstruction进行NFA到DFA的转换
    DFAState* dfaStart = subsetConstruction(start);

    // 打印DFA
    printDFA(dfaStart);

    return 0;
}