#ifndef GOBANG_H
#define GOBANG_H

#include <QMainWindow>
#include <QWidget>
#include <QPainter>
#include <QMouseEvent>
#include <vector>
#include <QTimer>

class ChessBoard : public QWidget {
    Q_OBJECT

public:
    explicit ChessBoard(QWidget *parent = nullptr);
    void restart();
    void setAIEnabled(bool enabled) { aiEnabled = enabled; }

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;

private:
    static const int BOARD_SIZE = 15;
    static const int GRID_SIZE = 40;
    static const int MARGIN = 30;
    static const int PIECE_RADIUS = 15;

    std::vector<std::vector<int>> board;
    bool isBlackTurn;
    bool gameOver;
    bool aiEnabled;  // 新增：AI开关
    QTimer *aiTimer; // 新增：AI思考定时器

    void drawBoard(QPainter& painter);
    void drawPieces(QPainter& painter);
    bool checkWin(int row, int col, int player);
    QPoint getBoardPos(QPoint pos);

    // 新增：AI相关函数
    void makeAIMove();
    int evaluatePosition(int row, int col, int player);
    std::pair<int, int> findBestMove();
    int evaluateLine(const std::vector<int>& line, int player);
    std::vector<std::pair<int, int>> getValidMoves();

    private slots:
        void aiMoveDelayed();  // 新增：延迟AI落子
};

class GoBangWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit GoBangWindow(QWidget *parent = nullptr);

private:
    ChessBoard *chessBoard;
    void setupUI();

    private slots:
        void restartGame();
    void toggleAI(bool enabled);  // 新增：切换AI开关
};

#endif // GOBANG_H