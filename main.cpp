#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <stack>
#include <chrono>

#define SCORE_MIN -70000
#define SCORE_MAX 70000

using namespace std;

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

int boardScore(int *valueBoard, char *occupationBoard, char player, int boardSize) {
    int myScore = 0;
    int oppScore = 0;
    for (int i = 0; i < boardSize * boardSize; i++) {
        if (occupationBoard[i] == player) {
            myScore += valueBoard[i];
        } else if (occupationBoard[i] != '.') {
            oppScore += valueBoard[i];
        }
    }
    return myScore - oppScore;
}

int updateScore(int move, int *valueBoard, char *occupationBoard, char player, int boardSize) {
    bool raid = false;
    int scoreGetStake = 0, scoreGetRaid = 0;
    scoreGetStake += valueBoard[move];
    if (move >= boardSize) {
        int value = valueBoard[move - boardSize];
        if (occupationBoard[move - boardSize] == player) {
            raid = true;
        } else if (occupationBoard[move - boardSize] != '.') {
            scoreGetRaid += 2 * value;
        }
    }
    if (move < boardSize * boardSize - boardSize) {
        int value = valueBoard[move + boardSize];
        if (occupationBoard[move + boardSize] == player) {
            raid = true;
        } else if (occupationBoard[move + boardSize] != '.') {
            scoreGetRaid += 2 * value;
        }
    }
    if (move % boardSize != 0) {
        int value = valueBoard[move - 1];
        if (occupationBoard[move - 1] == player) {
            raid = true;
        } else if (occupationBoard[move - 1] != '.') {
            scoreGetRaid += 2 * value;
        }
    }
    if (move % boardSize != boardSize - 1) {
        int value = valueBoard[move + 1];
        if (occupationBoard[move + 1] == player) {
            raid = true;
        } else if (occupationBoard[move + 1] != '.') {
            scoreGetRaid += 2 * value;
        }
    }
    if (raid) return scoreGetRaid + scoreGetStake;
    else return scoreGetStake;
}


int miniMax(int *valueBoard, char *occupationBoard, char player, int boardSize, int depth) {
    int score = boardScore(valueBoard, occupationBoard, player, boardSize);
    PossibleNode *head = constructPossibleLink(occupationBoard, boardSize);
    stack<PossibleNode*> moves;
    stack<int> scores;
    stack<int> results;

    int scoreOffset;
    int curDepth = 1;
    bool maxMove = true;
    PossibleNode *cur = head->next;
    while (true) {
        if (curDepth < depth) {
            if (cur->move >= 0) {
                curDepth++;
                moves.push(cur);
                scoreOffset = updateScore(cur->move, valueBoard, occupationBoard, player, boardSize);
                occupationBoard[cur->move] = player;
                score = maxMove ? score + scoreOffset : score - scoreOffset;
                scores.push(score);
                if (maxMove) results.push(SCORE_MIN);
                else results.push(SCORE_MAX);
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
                occupationBoard[cur->move] = '.';
                score = scores.top();
                moves.pop();
                scores.pop();
                cur = cur->next;
            }
        } else {
            int maxLeaf = SCORE_MIN;
            while (cur->move >= 0) {
                scoreOffset = updateScore(cur->move, valueBoard, occupationBoard, player, boardSize);
                maxLeaf = scoreOffset > maxLeaf ? scoreOffset : maxLeaf;
                cur = cur->next;
            }
            maxLeaf = maxMove ? maxLeaf : -maxLeaf;
            if (moves.empty()) {
                return maxLeaf;
            }
            cur = moves.top();
            cur->prev->next = cur;
            cur->next->prev = cur;
            curDepth--;
            moves.pop();
            score = scores.top();
            scores.pop();
            cur = cur->next;
        }
        player = player == 'X' ? 'O' : 'X';
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

    int *valueBoard = new int[boardSize * boardSize];
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
    miniMax(valueBoard, occupationBoard, player, boardSize, depth);


}