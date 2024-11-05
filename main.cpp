#include "GoBang.h"
#include <QApplication>

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    GoBangWindow window;
    window.show();
    return app.exec();
}