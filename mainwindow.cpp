#include "mainwindow.h"
#include "./ui_mainwindow.h"

#include <string>    // std::string
#include <QFile>     // QFile
#include <QTimer>    // QTimer

MainWindow::~MainWindow() {delete ui;}

// 主視窗的初始化
MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow) {
    ui->setupUi(this);
    ui->centralwidget->setAttribute(Qt::WA_TranslucentBackground);
    ui->centralwidget->setStyleSheet("background:transparent;");
    // 載入地圖
    load_map("Map.txt");
    // 初始化視窗
    setWindowTitle("PacMan-CRC48");                 // 設定視窗標題
    ui->centralwidget->setFixedSize(width, height); // 套用至畫布
    this->adjustSize();                             // 微調大小
    // 載入深色主題
    QFile file("yo_stylesheet.qss");
    if (file.open(QFile::ReadOnly | QFile::Text)) {
        QTextStream temp(&file);
        setStyleSheet(temp.readAll());
        file.close();
    } else throw std::runtime_error("qss not found!");
    // 設定計時器
    timer = new QTimer(this); // 連接到視窗更新
    connect(timer, &QTimer::timeout, this, &MainWindow::main_loop);
    timer->start(1000 / fps); // 每 1/fps 秒更新一次畫面
    // 建立小精靈
    player.init(this);
    // 建立鬼魂
    blinky.init(this);
    pinky.init(this);
    inky.init(this);
    clyde.init(this);
    // 繪製到螢幕視窗
    this->update();
}

// 更新一次狀態
void MainWindow::main_loop() {
    // 定格處理
    if (is_frozen) [[unlikely]] {
        freeze_timer -= 1;
        if (freeze_timer <= 0) {
            is_frozen = false;
        }
        repaint();
        return;
    }
    // 正常情況
    handle_collision();     // 碰撞偵測
    handle_passed();        // 判斷是否過關
    update_ghosts();        // 更新鬼魂們
    player.update();        // 更新小精靈
    repaint();              // 確保繪製
    count += 1;             // 計數器遞增
    count %= (fps*3600);    // 循環以避免溢位
    update_pellet_timer();  // 更新藥丸生效時間
}

// 繪製一幀的畫面
void MainWindow::paintEvent(QPaintEvent *event) {
    QPainter painter(this);
    draw_map(painter);      // 繪製地圖
    paint_ghosts(painter);  // 繪製鬼魂們
    player.paint(painter);  // 繪製小精靈
    draw_scorebar(painter); // 繪製狀態列
    // 定格畫面繪製 (得分)
    if (is_frozen) [[unlikely]] paint_eaten_score(painter);
}

// signals
void MainWindow::keyPressEvent(QKeyEvent *event) {
    switch (event->key()) {
        case Qt::Key_Left:
        case Qt::Key_A:
            player.turn_left();
            return;
        case Qt::Key_Right:
        case Qt::Key_D:
            player.turn_right();
            return;
        case Qt::Key_Up:
        case Qt::Key_W:
            player.turn_up();
            return;
        case Qt::Key_Down:
        case Qt::Key_S:
            player.turn_down();
            return;
        default: {
            QMainWindow::keyPressEvent(event);
            return;
        }
    }
}
