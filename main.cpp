#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <stack>
#include <chrono>

#define SCORE_MIN -70000
#define SCORE_MAX 70000

using namespace std;

//Global Variable
int boardSize;
char *occupationBoard;
int *valueBoard;


void printOccupationBoard(ofstream *out) {
    if (!out) {
        for (int i = 0; i < boardSize; i++) {
            for (int j = 0; j < boardSize; j++) {
                cout << occupationBoard[i * boardSize + j];
            }
            cout << endl;

        }
    } else {
        for (int i = 0; i < boardSize; i++) {
            for (int j = 0; j < boardSize; j++) {
                *out << occupationBoard[i * boardSize + j];
            }
            *out << endl;
        }
    }
}

struct PossibleNode {
    PossibleNode *prev, *next;
    int move;
};

struct SearchNode {
    PossibleNode *move;
    int changeSize;
    int changes[3];
    int result;
    int bestMove;
    int score;
};

struct SearchResult {
    bool raid;
    int move;
    int result;
};

void removePossibleNode(PossibleNode *node) {
    node->prev->next = node->next;
    node->next->prev = node->prev;
}

void rescuePossibleNode(PossibleNode *node) {
    node->next->prev = node;
    node->prev->next = node;
}

PossibleNode* constructPossibleLink(PossibleNode* previous) {
    if (previous) {
        return previous;
    }
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

int boardScore(char player) {
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

bool raidable(int move, char player) {
    bool raidable = false;
    bool raided = false;
    char oppPlayer = player == 'O' ? 'X' : 'O';
    if (move >= boardSize) {
        if (occupationBoard[move - boardSize] == player) {
            raidable = true;
        }
        if (occupationBoard[move - boardSize] == oppPlayer) {
            raided = true;
        }
    }
    if (move < boardSize * boardSize - boardSize) {
        if (occupationBoard[move + boardSize] == player) {
            raidable = true;
        }
        if (occupationBoard[move + boardSize] == oppPlayer) {
            raided = true;
        }
    }
    if (move % boardSize != 0) {
        if (occupationBoard[move - 1] == player) {
            raidable = true;
        }
        if (occupationBoard[move - 1] == oppPlayer) {
            raided = true;
        }
    }
    if (move % boardSize != boardSize - 1) {
        if (occupationBoard[move + 1] == player) {
            raidable = true;
        }
        if (occupationBoard[move + 1] == oppPlayer) {
            raided = true;
        }
    }
    return raidable && raided;
}

int updateScoreBoard(int move, char player, bool tryRaid, SearchNode *sn) {
    //Change occupation board only if sn is not NULL
    bool raid = false;
    int scoreGetStake = 0, scoreGetRaid = 0;
    int raidSize = 0;
    int raidIndex[4];
    scoreGetStake += valueBoard[move];
    if (move >= boardSize) {
        int value = valueBoard[move - boardSize];
        if (occupationBoard[move - boardSize] == player) {
            raid = true;
        } else if (occupationBoard[move - boardSize] != '.') {
            scoreGetRaid += 2 * value;
            raidIndex[raidSize++] = move - boardSize;
        }
    }
    if (move < boardSize * boardSize - boardSize) {
        int value = valueBoard[move + boardSize];
        if (occupationBoard[move + boardSize] == player) {
            raid = true;
        } else if (occupationBoard[move + boardSize] != '.') {
            scoreGetRaid += 2 * value;
            raidIndex[raidSize++] = move + boardSize;
        }
    }
    if (move % boardSize != 0) {
        int value = valueBoard[move - 1];
        if (occupationBoard[move - 1] == player) {
            raid = true;
        } else if (occupationBoard[move - 1] != '.') {
            scoreGetRaid += 2 * value;
            raidIndex[raidSize++] = move - 1;
        }
    }
    if (move % boardSize != boardSize - 1) {
        int value = valueBoard[move + 1];
        if (occupationBoard[move + 1] == player) {
            raid = true;
        } else if (occupationBoard[move + 1] != '.') {
            scoreGetRaid += 2 * value;
            raidIndex[raidSize++] = move + 1;
        }
    }
    if (raid && tryRaid) {
        if (sn) {
            sn->changeSize = raidSize;
            occupationBoard[move] = player;
            for (int i = 0; i < raidSize; i++) {
                occupationBoard[raidIndex[i]] = player;
                sn->changes[i] = raidIndex[i];
            }
        }
        return scoreGetRaid + scoreGetStake;
    }
    else {
        if (sn) {
            occupationBoard[move] = player;
            sn->changeSize = 0;
        }
        return scoreGetStake;
    }
}
void undoBoard(int move, char player, SearchNode &sn) {
    player = player == 'O' ? 'X' : 'O';
    occupationBoard[move] = '.';
    for (int i = 0; i < sn.changeSize; i++) {
        occupationBoard[sn.changes[i]] = player;
    }
}

int maxR(int, int, char, PossibleNode*, bool, int, int);

int minR(int depth, int score, char player, PossibleNode *prevHead, bool ab, int a, int b) {
    if (depth <= 0) return score;
    PossibleNode *head = constructPossibleLink(prevHead);
    PossibleNode *chosenNode = head->next;
    if (chosenNode->move < 0) {
        return score;
    }
    SearchNode sn;
    int nodeResult = SCORE_MAX;
    char oppPlayer = player == 'O' ? 'X' : 'O';
    //Consider stake only
    while (chosenNode->move >= 0) {
        removePossibleNode(chosenNode);
        int moveResult = maxR(depth - 1, score - updateScoreBoard(chosenNode->move, player, false, &sn), oppPlayer, head, ab, a, b);
        if (moveResult < nodeResult) {
            nodeResult = moveResult;
            b = moveResult;
        }
        undoBoard(chosenNode->move, player, sn);
        rescuePossibleNode(chosenNode);
        if (ab && nodeResult <= a) {
            return nodeResult;
        }
        chosenNode = chosenNode->next;
    }
    //Consider raid if possible
    chosenNode = head->next;
    while (chosenNode->move >= 0) {
        if (!raidable(chosenNode->move, player)) {
            chosenNode = chosenNode->next;
            continue;
        }
        removePossibleNode(chosenNode);
        int moveResult = maxR(depth - 1, score - updateScoreBoard(chosenNode->move, player, true, &sn), oppPlayer, head, ab, a, b);
        if (moveResult < nodeResult) {
            nodeResult = moveResult;
            b = moveResult;
        }
        undoBoard(chosenNode->move, player, sn);
        rescuePossibleNode(chosenNode);
        if (ab && nodeResult <= a) {
            return nodeResult;
        }
        chosenNode = chosenNode->next;
    }
    return nodeResult;
}

int maxR(int depth, int score, char player, PossibleNode *prevHead, bool ab, int a, int b) {
    if (depth <= 0) return score;
    PossibleNode *head = constructPossibleLink(prevHead);
    PossibleNode *chosenNode = head->next;
    if (chosenNode->move < 0) {
        return score;
    }
    SearchNode sn;
    int nodeResult = SCORE_MIN;
    char oppPlayer = player == 'O' ? 'X' : 'O';
    //Consider stake only
    while (chosenNode->move >= 0) {
        removePossibleNode(chosenNode);
        int moveResult = minR(depth - 1, score + updateScoreBoard(chosenNode->move, player, false, &sn), oppPlayer, head, ab, a, b);
        if (moveResult > nodeResult) {
            nodeResult = moveResult;
            a = moveResult;
        }
        undoBoard(chosenNode->move, player, sn);
        rescuePossibleNode(chosenNode);
        if (ab && nodeResult >= b) {
            return nodeResult;
        }
        chosenNode = chosenNode->next;
    }
    //Consider raid if possible
    chosenNode = head->next;
    while (chosenNode->move >= 0) {
        if (!raidable(chosenNode->move, player)) {
            chosenNode = chosenNode->next;
            continue;
        }
        removePossibleNode(chosenNode);
        int moveResult = minR(depth - 1, score + updateScoreBoard(chosenNode->move, player, true, &sn), oppPlayer, head, ab, a, b);
        if (moveResult > nodeResult) {
            nodeResult = moveResult;
            a = moveResult;
        }
        undoBoard(chosenNode->move, player, sn);
        rescuePossibleNode(chosenNode);
        if (ab && nodeResult >= b) {
            return nodeResult;
        }
        chosenNode = chosenNode->next;
    }
    return nodeResult;
}

SearchResult miniMaxR(int depth, char player, bool ab) {
    SearchResult rv;
    if (depth <= 0) return rv;
    int score = boardScore(player);
    PossibleNode *head = constructPossibleLink(NULL);
    PossibleNode *chosenNode = head->next;
    SearchNode sn;
    int nodeResult = SCORE_MIN;
    int a = SCORE_MIN, b = SCORE_MAX;
    char oppPlayer = player == 'O' ? 'X' : 'O';
    //Consider stake only
    while (chosenNode->move >= 0) {
        removePossibleNode(chosenNode);
        int moveResult = minR(depth - 1, score + updateScoreBoard(chosenNode->move, player, false, &sn), oppPlayer, head, ab, a, b);
        if (moveResult > nodeResult) {
            rv.move = chosenNode->move;
            nodeResult = moveResult;
            rv.raid = false;
            a = moveResult;
        }
        undoBoard(chosenNode->move, player, sn);
        rescuePossibleNode(chosenNode);
        chosenNode = chosenNode->next;
    }
    //Consider raid if possible
    chosenNode = head->next;
    while (chosenNode->move >= 0) {
        if (!raidable(chosenNode->move, player)) {
            chosenNode = chosenNode->next;
            continue;
        }
        removePossibleNode(chosenNode);
        int moveResult = minR(depth - 1, score + updateScoreBoard(chosenNode->move, player, true, &sn), oppPlayer, head, ab, a, b);
        if (moveResult > nodeResult) {
            rv.move = chosenNode->move;
            nodeResult = moveResult;
            rv.raid = true;
            a = moveResult;
        }
        undoBoard(chosenNode->move, player, sn);
        rescuePossibleNode(chosenNode);
        chosenNode = chosenNode->next;
    }
    rv.result = nodeResult;
    return rv;
}

int main(int argc, char *argv[]) {
    string input_file = argc >= 2 ? argv[1] : "input.txt";
    string output_file = argc >= 3 ? argv[2] : "output.txt";
    ifstream in(input_file);
    int  depth;
    char player;
    string mode;

    in >> boardSize;
    in >> mode;
    in >> player;
    in >> depth;

    valueBoard = new int[boardSize * boardSize];
    occupationBoard = new char[boardSize * boardSize];

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

    SearchResult result;
    if (mode == "ALPHABETA") {
        result = miniMaxR(depth, player, true);
    } else {
        result = miniMaxR(depth, player, false);
    }
    int row = result.move / boardSize + 1;
    char col = static_cast<char>(result.move % boardSize) + 'A';
    SearchNode sn;
    string moveType = result.raid ? "Raid" : "Stake";
    ofstream out;
    out.open(output_file);
    out << col << row << " " + moveType << endl;
    updateScoreBoard(result.move, player, result.raid, &sn);
    printOccupationBoard(&out);
    out.close();
}