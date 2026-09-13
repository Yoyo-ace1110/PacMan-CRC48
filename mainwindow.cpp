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
    // 建立小精靈與鬼魂
    pacman.init(this);
    blinky.init(this);
    pinky.init(this);
    inky.init(this);
    clyde.init(this);
    // 重置角色位置並進入等待玩家第一個移動鍵狀態
    reset_game_positions();
    this->update();
}

// 更新一次狀態
void MainWindow::main_loop() {
    // 1. 等待玩家按下第一個移動鍵 (開局或死亡重置後)
    if (is_waiting_start) {
        repaint();
        return;
    }
    // 2. 吃鬼得分定格處理 (僅暫停移動與邏輯，畫面角色維持正常繪製)
    if (is_frozen) [[unlikely]] {
        freeze_timer -= 1;
        if (freeze_timer <= 0) {
            is_frozen = false;
            eaten_ghost_ptr = nullptr;
        }
        repaint();
        return;
    }
    // 3. 正常遊戲邏輯
    handle_collision();     // 碰撞偵測
    handle_passed();        // 判斷是否過關
    update_behavior_mode(); // 更新鬼魂行為模式 (Scatter / Chase)
    update_ghosts();        // 更新鬼魂們
    pacman.update();        // 更新小精靈
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
    pacman.paint(painter);  // 繪製小精靈
    draw_scorebar(painter); // 繪製狀態列
    // 吃掉鬼魂且定格時繪製浮動得分
    if (is_frozen && eaten_ghost_ptr != nullptr) [[unlikely]] {
        paint_eaten_score(painter);
    }
}

// signals
void MainWindow::keyPressEvent(QKeyEvent *event) {
    bool move_key_pressed = false;
    switch (event->key()) {
        case Qt::Key_Left:
        case Qt::Key_A:
            pacman.turn_left();
            move_key_pressed = true;
            break;
        case Qt::Key_Right:
        case Qt::Key_D:
            pacman.turn_right();
            move_key_pressed = true;
            break;
        case Qt::Key_Up:
        case Qt::Key_W:
            pacman.turn_up();
            move_key_pressed = true;
            break;
        case Qt::Key_Down:
        case Qt::Key_S:
            pacman.turn_down();
            move_key_pressed = true;
            break;
        default: {
            QMainWindow::keyPressEvent(event);
            return;
        }
    }
    // 若處於等待按鍵開始狀態，按下第一個移動鍵即解鎖開始移動
    if (move_key_pressed && is_waiting_start) {
        is_waiting_start = false;
    }
}

