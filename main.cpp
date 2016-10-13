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
    int raidIndex[3];
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

/*
int miniMax(int *valueBoard, char *occupationBoard, char player, int boardSize, int depth) {
    int curScore = boardScore(valueBoard, occupationBoard, player, boardSize);
    int curResult = SCORE_MIN;
    int curBestMove = -1;
    int curDepth = 1;
    bool isMaxMove = true;
    char curPlayer = player;

    PossibleNode *head = constructPossibleLink(occupationBoard, boardSize);
    PossibleNode *curMove = head->next;
    stack<SearchNode> path;

    SearchNode sn, curNode;

    while (true) {
        if (isMaxMove) {
            if (curDepth < depth) {
                if (curMove->move >= 0) {
                    sn.move = curMove;
                    sn.score = curScore;
                    sn.result = SCORE_MIN;
                    sn.bestMove = -1;
                    curScore += updateScoreBoard(curMove->move, valueBoard, occupationBoard, curPlayer, boardSize, &sn);

                    path.push(sn);

                    curBestMove = -1;
                    curDepth++;
                    curMove->prev->next = curMove->next;
                    curMove->next->prev = curMove->prev;
                    curMove = head->next;
                    curResult = SCORE_MAX;
                    printOccupationBoard(occupationBoard, boardSize);
                } else {
                    if (path.empty()) {
                        return curBestMove;
                    }
                    sn = path.top();
                    path.pop();

                    if (sn.result < curResult) {
                        curResult = sn.result;
                        curBestMove = sn.bestMove;
                    }
                    curDepth--;
                    for (int i = 0; i < sn.changeSize; i++) {
                        occupationBoard[sn.changes[i]] = curPlayer;
                    }
                    occupationBoard[sn.move->move] = '.';
                    sn.move->prev->next = sn.move;
                    sn.move->next->prev = sn.move;
                    curMove = sn.move->next;
                    curScore = sn.score;
                }
            } else {
                int bestLeaf = 0;
                int leafOffset;
                while (curMove->move >= 0) {
                    leafOffset = updateScoreBoard(curMove->move, valueBoard, occupationBoard, player, boardSize, NULL);
                    if (leafOffset > bestLeaf) {
                        bestLeaf = leafOffset;
                        curBestMove = curMove->move;
                    }
                    curMove = curMove->next;
                }
                curResult = curScore + bestLeaf;

                if (path.empty()) {
                    return curBestMove;
                }

                sn = path.top();
                path.pop();

                if (sn.result < curResult) {
                    curResult = sn.result;
                    curBestMove = sn.bestMove;
                }
                curDepth--;
                for (int i = 0; i < sn.changeSize; i++) {
                    occupationBoard[sn.changes[i]] = curPlayer;
                }
                occupationBoard[sn.move->move] = '.';
                sn.move->prev->next = sn.move;
                sn.move->next->prev = sn.move;
                curMove = sn.move->next;
                curScore = sn.score;
            }
        } else {
            if (curDepth < depth) {
                if (curMove->move >= 0) {
                    sn.move = curMove;
                    sn.score = curScore;
                    sn.result = SCORE_MAX;
                    sn.bestMove = -1;
                    curScore -= updateScoreBoard(curMove->move, valueBoard, occupationBoard, curPlayer, boardSize, &sn);

                    path.push(sn);

                    curBestMove = -1;
                    curDepth++;
                    curMove->prev->next = curMove->next;
                    curMove->next->prev = curMove->prev;
                    curMove = head->next;
                    curResult = SCORE_MIN;
                    printOccupationBoard(occupationBoard, boardSize);
                } else {
                    if (path.empty()) {
                        return curBestMove;
                    }
                    sn = path.top();
                    path.pop();

                    if (sn.result > curResult) {
                        curResult = sn.result;
                        curBestMove = sn.bestMove;
                    }
                    curDepth--;
                    for (int i = 0; i < sn.changeSize; i++) {
                        occupationBoard[sn.changes[i]] = curPlayer;
                    }
                    occupationBoard[sn.move->move] = '.';
                    sn.move->prev->next = sn.move;
                    sn.move->next->prev = sn.move;
                    curMove = sn.move->next;
                    curScore = sn.score;
                }
            } else {
                int bestLeaf = 0;
                int leafOffset;
                while (curMove->move >= 0) {
                    leafOffset = updateScoreBoard(curMove->move, valueBoard, occupationBoard, player, boardSize, NULL);
                    if (leafOffset > bestLeaf) {
                        bestLeaf = leafOffset;
                        curBestMove = curMove->move;
                    }
                    curMove = curMove->next;
                }
                curResult = curScore - bestLeaf;

                if (path.empty()) {
                    return curBestMove;
                }

                sn = path.top();
                path.pop();

                if (sn.result > curResult) {
                    curResult = sn.result;
                    curBestMove = sn.bestMove;
                }
                curDepth--;
                for (int i = 0; i < sn.changeSize; i++) {
                    occupationBoard[sn.changes[i]] = curPlayer;
                }
                occupationBoard[sn.move->move] = '.';
                sn.move->prev->next = sn.move;
                sn.move->next->prev = sn.move;
                curMove = sn.move->next;
                curScore = sn.score;
            }
        }
        isMaxMove ^= true;
        curPlayer = curPlayer == 'O' ? 'X' : 'O';
    }
    return -1;
}


int _miniMax(int *valueBoard, char *occupationBoard, char player, int boardSize, int depth) {
    int score = boardScore(valueBoard, occupationBoard, player, boardSize);
    PossibleNode *head = constructPossibleLink(occupationBoard, boardSize);
    stack<SearchNode> path;
    int scoreOffset;
    int curDepth = 1;
    int curResult = SCORE_MIN;
    int curBestMove = -1;
    bool maxMove = true;
    PossibleNode *cur = head->next;
    SearchNode sn;
    while (true) {
        if (curDepth < depth) {
            if (cur->move >= 0) {
                curDepth++;
                sn.move = cur;
                scoreOffset = updateScoreBoard(cur->move, valueBoard, occupationBoard, player, boardSize, &sn);
                sn.score = score;
                sn.result = curResult;
                sn.bestMove = curBestMove;
                path.push(sn);
                cur->prev->next = cur->next;
                cur->next->prev = cur->prev;
                cur = head->next;
                curBestMove = -1;
                curResult = maxMove ? SCORE_MAX : SCORE_MIN;
                score = maxMove ? score + scoreOffset : score - scoreOffset;
                printOccupationBoard(occupationBoard, boardSize);
            } else {
                if (path.empty()) {
                    break;
                }
                curDepth--;
                sn = path.top();
                cur = sn.move;
                cur->prev->next = cur;
                cur->next->prev = cur;
                occupationBoard[cur->move] = '.';
                for (int i = 0; i < sn.changeSize; i++) {
                    occupationBoard[sn.changes[i]] = player;
                }
                score = sn.score;
                if ((maxMove && curResult < sn.result) || (!maxMove && curResult > sn.result)) {
                    curResult = sn.result;
                    curBestMove = sn.bestMove;
                }
                path.pop();
                cur = cur->next;
            }
        } else {
            int maxLeaf = SCORE_MIN;
            while (cur->move >= 0) {
                scoreOffset = updateScoreBoard(cur->move, valueBoard, occupationBoard, player, boardSize, NULL);
                if (scoreOffset > maxLeaf) {
                    maxLeaf = scoreOffset;
                    curBestMove = cur->move;
                }
                cur = cur->next;
            }
            curResult = maxMove ? score + maxLeaf : score - maxLeaf;
            if (path.empty()) {
                return curBestMove;
            }
            sn = path.top();
            cur = sn.move;
            cur->prev->next = cur;
            cur->next->prev = cur;
            occupationBoard[cur->move] = '.';
            if ((maxMove && curResult < sn.result) || (!maxMove && curResult > sn.result)) {
                curResult = sn.result;
                curBestMove = sn.bestMove;
            }
            for (int i = 0; i < sn.changeSize; i++) {
                occupationBoard[sn.changes[i]] = player;
            }
            score = sn.score;
            curDepth--;
            path.pop();
            cur = cur->next;
        }
        player = player == 'X' ? 'O' : 'X';
        maxMove ^= true;
    }
    return 0;
}
*/
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

int main() {
    ifstream in("input.txt");
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
    out.open("output.txt");
    out << col << row << " " + moveType << endl;
    updateScoreBoard(result.move, player, true, &sn);
    printOccupationBoard(&out);
    out.close();
}