#include "GoBang.h"
#include <QVBoxLayout>
#include <QPushButton>
#include <QMessageBox>
#include <QCheckBox>
#include <random>
#include <algorithm>

using namespace std;

ChessBoard::ChessBoard(QWidget *parent) : QWidget(parent) {
    setMinimumSize(BOARD_SIZE * GRID_SIZE + 2 * MARGIN,
                   BOARD_SIZE * GRID_SIZE + 2 * MARGIN);
    setMaximumSize(minimumSize());

    board = std::vector<std::vector<int>>(BOARD_SIZE,
           std::vector<int>(BOARD_SIZE, 0));
    isBlackTurn = true;
    gameOver = false;
    aiEnabled = false;

    // 初始化AI定时器
    aiTimer = new QTimer(this);
    aiTimer->setSingleShot(true);
    connect(aiTimer, &QTimer::timeout, this, &ChessBoard::aiMoveDelayed);
}

// ... (保持原有的 paintEvent, drawBoard, drawPieces, getBoardPos 函数不变) ...

void ChessBoard::mousePressEvent(QMouseEvent *event) {
    if (gameOver || (!isBlackTurn && aiEnabled)) return;

    QPoint pos = getBoardPos(event->pos());
    int row = pos.y();
    int col = pos.x();

    if (row >= 0 && row < BOARD_SIZE && col >= 0 && col < BOARD_SIZE
        && board[row][col] == 0) {
        // 玩家落子
        board[row][col] = isBlackTurn ? 1 : 2;

        if (checkWin(row, col, board[row][col])) {
            gameOver = true;
            QString winner = isBlackTurn ? "黑棋" : "白棋";
            QMessageBox::information(this, "游戏结束", winner + "获胜！");
        } else {
            isBlackTurn = !isBlackTurn;
            update();

            // 如果启用AI且现在是AI回合，延迟执行AI移动
            if (aiEnabled && !isBlackTurn) {
                aiTimer->start(500); // 500ms延迟
            }
        }
    }
}

void ChessBoard::aiMoveDelayed() {
    if (!gameOver && aiEnabled && !isBlackTurn) {
        makeAIMove();
    }
}

void ChessBoard::makeAIMove() {
    auto [row, col] = findBestMove();

    if (row >= 0 && col >= 0) {
        board[row][col] = 2; // AI使用白子

        if (checkWin(row, col, 2)) {
            gameOver = true;
            QMessageBox::information(this, "游戏结束", "AI获胜！");
        } else {
            isBlackTurn = true;
        }
        update();
    }
}

std::pair<int, int> ChessBoard::findBestMove() {
    std::vector<std::pair<int, int>> validMoves = getValidMoves();
    int bestScore = -1;
    std::vector<std::pair<int, int>> bestMoves;

    // 评估所有可能的移动
    for (const auto& move : validMoves) {
        int score = evaluatePosition(move.first, move.second, 2);
        if (score > bestScore) {
            bestScore = score;
            bestMoves.clear();
            bestMoves.push_back(move);
        } else if (score == bestScore) {
            bestMoves.push_back(move);
        }
    }

    // 从最佳移动中随机选择一个
    if (!bestMoves.empty()) {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> dis(0, bestMoves.size() - 1);
        return bestMoves[dis(gen)];
    }

    return {-1, -1};
}

std::vector<std::pair<int, int>> ChessBoard::getValidMoves() {
    std::vector<std::pair<int, int>> moves;

    // 只考虑已有棋子周围的空位
    for (int i = 0; i < BOARD_SIZE; i++) {
        for (int j = 0; j < BOARD_SIZE; j++) {
            if (board[i][j] != 0) {
                // 检查周围8个方向
                for (int di = -1; di <= 1; di++) {
                    for (int dj = -1; dj <= 1; dj++) {
                        if (di == 0 && dj == 0) continue;

                        int ni = i + di;
                        int nj = j + dj;

                        if (ni >= 0 && ni < BOARD_SIZE &&
                            nj >= 0 && nj < BOARD_SIZE &&
                            board[ni][nj] == 0) {
                            moves.push_back({ni, nj});
                        }
                    }
                }
            }
        }
    }

    // 如果棋盘为空或没有找到合适的位置，返回中心位置
    if (moves.empty()) {
        moves.push_back({BOARD_SIZE/2, BOARD_SIZE/2});
    }

    // 去除重复位置
    std::sort(moves.begin(), moves.end());
    moves.erase(std::unique(moves.begin(), moves.end()), moves.end());

    return moves;
}

int ChessBoard::evaluatePosition(int row, int col, int player) {
    int score = 0;
    const int directions[4][2] = {{1,0}, {0,1}, {1,1}, {1,-1}};

    for (const auto& dir : directions) {
        std::vector<int> line;

        // 获取当前方向的棋子序列
        for (int i = -4; i <= 4; i++) {
            int newRow = row + dir[0] * i;
            int newCol = col + dir[1] * i;

            if (newRow >= 0 && newRow < BOARD_SIZE &&
                newCol >= 0 && newCol < BOARD_SIZE) {
                if (newRow == row && newCol == col) {
                    line.push_back(player);
                } else {
                    line.push_back(board[newRow][newCol]);
                }
            }
        }

        score += evaluateLine(line, player);
    }

    return score;
}

int ChessBoard::evaluateLine(const std::vector<int>& line, int player) {
    int score = 0;
    int opponent = (player == 1) ? 2 : 1;

    // 评分规则
    for (int i = 0; i < line.size() - 4; i++) {
        std::vector<int> window(line.begin() + i, line.begin() + i + 5);

        int playerCount = std::count(window.begin(), window.end(), player);
        int emptyCount = std::count(window.begin(), window.end(), 0);
        int oppCount = std::count(window.begin(), window.end(), opponent);

        // 进攻评分
        if (playerCount == 4 && emptyCount == 1) score += 10000;  // 快要赢了
        else if (playerCount == 3 && emptyCount == 2) score += 1000;
        else if (playerCount == 2 && emptyCount == 3) score += 100;

        // 防守评分
        if (oppCount == 4 && emptyCount == 1) score += 5000;  // 必须防守
        else if (oppCount == 3 && emptyCount == 2) score += 500;
    }

    return score;
}

void ChessBoard::restart() {
    for (auto& row : board) {
        std::fill(row.begin(), row.end(), 0);
    }
    isBlackTurn = true;
    gameOver = false;
    update();
}

// GoBangWindow类的实现
GoBangWindow::GoBangWindow(QWidget *parent) : QMainWindow(parent) {
    setupUI();
}

void GoBangWindow::setupUI() {
    setWindowTitle("五子棋");

    auto centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    auto layout = new QVBoxLayout(centralWidget);

    chessBoard = new ChessBoard(this);
    layout->addWidget(chessBoard);

    auto controlLayout = new QHBoxLayout();

    auto restartButton = new QPushButton("重新开始", this);
    controlLayout->addWidget(restartButton);

    auto aiCheckBox = new QCheckBox("启用AI对手", this);
    controlLayout->addWidget(aiCheckBox);

    layout->addLayout(controlLayout);

    connect(restartButton, &QPushButton::clicked,
            this, &GoBangWindow::restartGame);
    connect(aiCheckBox, &QCheckBox::toggled,
            this, &GoBangWindow::toggleAI);
}

void GoBangWindow::restartGame() {
    chessBoard->restart();
}

void GoBangWindow::toggleAI(bool enabled) {
    chessBoard->setAIEnabled(enabled);
}

void ChessBoard::paintEvent(QPaintEvent *) {
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    drawBoard(painter);
    drawPieces(painter);
}

QPoint ChessBoard::getBoardPos(QPoint pos) {
    int x = (pos.x() - MARGIN + GRID_SIZE / 2) / GRID_SIZE;
    int y = (pos.y() - MARGIN + GRID_SIZE / 2) / GRID_SIZE;
    return QPoint(x, y);
}

bool ChessBoard::checkWin(int row, int col, int player) {
    // 方向数组：水平、垂直、对角线、反对角线
    const int directions[4][2] = {{1,0}, {0,1}, {1,1}, {1,-1}};

    for (const auto& dir : directions) {
        int count = 1;  // 当前位置已经有一个棋子

        // 正向检查
        for (int i = 1; i < 5; i++) {
            int newRow = row + dir[0] * i;
            int newCol = col + dir[1] * i;

            if (newRow < 0 || newRow >= BOARD_SIZE ||
                newCol < 0 || newCol >= BOARD_SIZE ||
                board[newRow][newCol] != player) {
                break;
            }
            count++;
        }

        // 反向检查
        for (int i = 1; i < 5; i++) {
            int newRow = row - dir[0] * i;
            int newCol = col - dir[1] * i;

            if (newRow < 0 || newRow >= BOARD_SIZE ||
                newCol < 0 || newCol >= BOARD_SIZE ||
                board[newRow][newCol] != player) {
                break;
            }
            count++;
        }

        if (count >= 5) {
            return true;
        }
    }
    return false;
}

void ChessBoard::drawBoard(QPainter& painter) {
    // 绘制棋盘背景
    painter.fillRect(rect(), QColor(240, 200, 150));

    // 绘制网格线
    QPen pen(Qt::black, 1);
    painter.setPen(pen);

    for (int i = 0; i < BOARD_SIZE; i++) {
        // 横线
        painter.drawLine(MARGIN, MARGIN + i * GRID_SIZE,
                        MARGIN + (BOARD_SIZE - 1) * GRID_SIZE,
                        MARGIN + i * GRID_SIZE);
        // 竖线
        painter.drawLine(MARGIN + i * GRID_SIZE, MARGIN,
                        MARGIN + i * GRID_SIZE,
                        MARGIN + (BOARD_SIZE - 1) * GRID_SIZE);
    }
}

void ChessBoard::drawPieces(QPainter& painter) {
    for (int i = 0; i < BOARD_SIZE; i++) {
        for (int j = 0; j < BOARD_SIZE; j++) {
            if (board[i][j] != 0) {
                QColor color = (board[i][j] == 1) ? Qt::black : Qt::white;
                painter.setPen(Qt::NoPen);
                painter.setBrush(color);

                // 如果是白子，添加边框以便于在浅色背景上显示
                if (board[i][j] == 2) {
                    painter.setPen(QPen(Qt::black, 1));
                }

                painter.drawEllipse(QPoint(MARGIN + j * GRID_SIZE,
                                         MARGIN + i * GRID_SIZE),
                                  PIECE_RADIUS, PIECE_RADIUS);
            }
        }
    }
}