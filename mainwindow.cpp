#include "mainwindow.h"
#include "./ui_mainwindow.h"

#include <string>    // std::string
#include <QFile>     // QFile
#include <QTimer>    // QTimer
#include <windows.h> // system

MainWindow::~MainWindow() {delete ui;}

// 主視窗的初始化
MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow) {
    ui->setupUi(this);
    ui->centralwidget->setAttribute(Qt::WA_TranslucentBackground);
    ui->centralwidget->setStyleSheet("background:transparent;");
    // 嘗試載入地圖
    std::string map_path = "Map.txt";
    if (!load_map(map_path)) throw std::runtime_error("Cannot load map from file: "+map_path);
    // 初始化視窗
    setWindowTitle("PacMan-CRC48"); // 設定視窗標題
    system("chcp 65001 > nul");     // 避免亂碼
    int width = map_width*tile_size, height = map_height*tile_size; // 設定視窗大小
    ui->centralwidget->setFixedSize(width, height);                 // 套用至中央畫布
    this->adjustSize();                                             // 微調大小
    // 載入深色主題
    QFile file("yo_stylesheet.qss");
    if (file.open(QFile::ReadOnly | QFile::Text)) {
        QTextStream temp(&file);
        setStyleSheet(temp.readAll());
        file.close();
    } else throw std::runtime_error("qss not found!");
    // 設定計時器
    timer = new QTimer(this); // 連接到視窗更新
    connect(timer, &QTimer::timeout, this, QOverload<>::of(&MainWindow::update));
    timer->start(1000 / fps); // 每 1/fps 秒更新一次畫面
    // 建立小精靈 & 繪製到螢幕視窗
    player.init(this);
    this->update();
}

// 繪製一幀的畫面
void MainWindow::paintEvent(QPaintEvent *event) {
    QPainter painter(this);
    draw_map(painter);
    player.update(painter);
}
