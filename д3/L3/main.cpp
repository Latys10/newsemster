#include <QApplication>
#include "MainWindow.h"

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);   // создаёт приложение Qt
    MainWindow w;
    w.show();                       // показать окно
    return app.exec();              // запуск цикла обработки событий
}
