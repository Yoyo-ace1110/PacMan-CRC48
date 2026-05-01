#include "mainwindow.h"

#include <QApplication>

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    MainWindow window;
    window.show();
    return app.exec();
}

// root_folder = C:\Users\yosprout\Desktop\Codes\Github\PacMan-CRC48
// copy root_folder\Map.txt To root_folder\PacMan\Map.txt
// copy root_folder\yo_stylesheet.qss To root_folder\PacMan\yo_stylesheet.qss
// copy root_folder\build\Desktop_Qt_6_10_1_MinGW_64_bit-Release\CRC48-PacMan.exe To root_folder\PacMan\CRC48-PacMan.exe
// run C:\Users\yosprout\AppData\Roaming\Microsoft\Windows\Start Menu\Programs\Qt\6.10.1\MinGW 13.1.0 (64-bit)
// windeployqt root_folder\PacMan\CRC48-PacMan.exe
