#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <stack>

struct PossibleNode {
    PossibleNode *prev, *next;
    int move;
};

using namespace std;

typedef unsigned char index;
typedef unsigned char value;

PossibleNode* constructPossibleLink(char *occupationBoard, index boardSize) {
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
    cur->next = NULL;
    return head;
}

void boardScore(value *valueBoard, char *occupationBoard, char player, index boardSize, int *totalScore, int *myScore, int *oppScore) {
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

void updateScore(int move, value *valueBoard, char *occupationBoard, char player, index boardSize, int *myScore, int *oppScore) {
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


int miniMax(value *valueBoard, char *occupationBoard, char player, index boardSize, int depth) {
    PossibleNode *head = constructPossibleLink(occupationBoard, boardSize);
    int myScore, oppScore, totalScore;
    boardScore(valueBoard, occupationBoard, player, boardSize, &myScore, &oppScore, &totalScore);
    stack<PossibleNode*> moves;
    int curDepth = 0;
    bool maxMove = true;
    PossibleNode *cur = head;
    moves.push(cur);
    while (true) {
        PossibleNode *cur = moves.top()->next;
        moves.pop();
        if (cur) {
            moves.push(cur);
            cur->prev->next = cur->next;

        }
    }

}

int main() {
    ifstream in("input.txt");
    index boardSize;
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


}