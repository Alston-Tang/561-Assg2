#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <stack>
#include <chrono>

using namespace std;
typedef unsigned char value;

class BenchMark{
    chrono::high_resolution_clock t;
    chrono::high_resolution_clock::time_point start, end;
    int b;
    int d;
public:
    BenchMark(int _b, int _d) : b(_b), d(_d){};
    void startTimer(){
        start = chrono::high_resolution_clock::now();
    }
    void endTimer() {
        end = chrono::high_resolution_clock::now();
    }
    double stepsPerS() {
        long long totalSteps = 1;
        for (int i = b; i > b - d; i--) {
            totalSteps *= i;
        }
        cout << totalSteps / chrono::duration_cast<chrono::seconds>(end - start).count() << endl;
        return 0;
    }
};

struct PossibleNode {
    PossibleNode *prev, *next;
    int move;
};

PossibleNode* constructPossibleLink(char *occupationBoard, int boardSize) {
    PossibleNode *head = new PossibleNode;
    PossibleNode *cur = head;
    for (int i = 0; i < boardSize * boardSize; i++) {
        if (occupationBoard[i] == '.') {
            PossibleNode *newNode = new PossibleNode;
            cur->next = newNode;
            newNode->prev = cur;
            newNode->move = i;
            cur = newNode;
        }
    }
    PossibleNode *end = new PossibleNode;
    cur->next = end;
    end->prev = cur;
    end->move = -1;
    return head;
}

void boardScore(value *valueBoard, char *occupationBoard, char player, int boardSize, int *totalScore, int *myScore, int *oppScore) {
    *myScore = 0;
    *totalScore = 0;
    *oppScore = 0;
    for (int i = 0; i < boardSize * boardSize; i++) {
        if (occupationBoard[i] == player) {
            *myScore += valueBoard[i];
        } else if (occupationBoard[i] != '.') {
            *oppScore += valueBoard[i];
        }
        *totalScore += valueBoard[i];
    }
}

void updateScore(int move, value *valueBoard, char *occupationBoard, char player, int boardSize, int *myScore, int *oppScore) {
    bool raid = false;
    int scoreGetStake = 0, scoreGetRaid = 0;
    scoreGetStake += valueBoard[move];
    if (move >= boardSize) {
        int value = valueBoard[move - boardSize];
        if (occupationBoard[move - boardSize] == player) {
            raid = true;
        } else if (occupationBoard[move - boardSize] != '.') {
            scoreGetRaid += value;
        }
    }
    if (move < boardSize * boardSize - boardSize) {
        int value = valueBoard[move + boardSize];
        if (occupationBoard[move + boardSize] == player) {
            raid = true;
        } else if (occupationBoard[move + boardSize] != '.') {
            scoreGetRaid += value;
        }
    }
    if (move % boardSize != 0) {
        int value = valueBoard[move - 1];
        if (occupationBoard[move - 1] == player) {
            raid = true;
        } else if (occupationBoard[move - 1] != '.') {
            scoreGetRaid += value;
        }
    }
    if (move % boardSize != boardSize - 1) {
        int value = valueBoard[move + 1];
        if (occupationBoard[move + 1] == player) {
            raid = true;
        } else if (occupationBoard[move + 1] != '.') {
            scoreGetRaid += value;
        }
    }
    *myScore = raid ? *myScore + scoreGetRaid + scoreGetStake : *myScore + scoreGetStake;
    *oppScore = raid ? *oppScore - scoreGetRaid : *oppScore;
}


int miniMax(value *valueBoard, char *occupationBoard, char player, int boardSize, int depth) {
    PossibleNode *head = constructPossibleLink(occupationBoard, boardSize);
    int myScore, oppScore, totalScore;
    boardScore(valueBoard, occupationBoard, player, boardSize, &myScore, &oppScore, &totalScore);
    stack<PossibleNode*> moves;
    stack<int> score;

    char oppPlayer = player == 'X' ? 'O' : 'X';
    int curDepth = 1;
    bool maxMove = true;
    PossibleNode *cur = head->next;
    while (true) {
        if (curDepth < depth) {
            if (cur->move >= 0) {
                curDepth++;
                moves.push(cur);
                cur->prev->next = cur->next;
                cur->next->prev = cur->prev;
                cur = head->next;
            } else {
                if (moves.empty()) {
                    break;
                }
                curDepth--;
                cur = moves.top();
                cur->prev->next = cur;
                cur->next->prev = cur;
                moves.pop();
                cur = cur->next;
            }
        } else {
            while (cur->move >= 0) {
                cur = cur->next;
            }
            if (moves.empty()) {
                break;
            }
            cur = moves.top();
            cur->prev->next = cur;
            cur->next->prev = cur;
            curDepth--;
            moves.pop();
            cur = cur->next;
        }
        char temp = player;
        player = oppPlayer;
        oppPlayer = temp;
        maxMove ^= true;
    }
    return 0;
}

int main() {
    ifstream in("input.txt");
    int boardSize;
    int  depth;
    char player;
    string mode;

    in >> boardSize;
    in >> mode;
    in >> player;
    in >> depth;

    value *valueBoard = new value[boardSize * boardSize];
    char *occupationBoard = new char[boardSize * boardSize];

    for (int i = 0; i < boardSize * boardSize; i++) {
        in >> valueBoard[i];
    }
    for (int i = 0; i < boardSize * boardSize; i++) {
        char c;
        do {
            in >> c;
        } while (c != '.' && c != 'X' && c != 'O');
        occupationBoard[i] = c;
    }
    in.close();
    BenchMark b = BenchMark(36, 6);
    b.startTimer();
    miniMax(valueBoard, occupationBoard, player, boardSize, depth);
    b.endTimer();
    cout << b.stepsPerS();


}