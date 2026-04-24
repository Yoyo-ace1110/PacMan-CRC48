#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <array>       // std::array
#include <cstdint>     // size_t
#include <fstream>     // 操作檔案
#include <iostream>    // std::cout
#include <QMainWindow> // 主視窗
#include <QPainter>    // 畫筆工具
#include <QKeyEvent>   // 鍵盤工具

using size_t  = std::size_t ;

QT_BEGIN_NAMESPACE
namespace Ui {class MainWindow;}
QT_END_NAMESPACE

enum class Tile : uint8_t {
    flat = 0, // 可以通過的空地
    wall = 1  // 不可通過的牆壁
};

class PacMan {
private:
    enum class Direc : uint8_t {
        right = 0,
        left = 1,
        up = 2,
        down = 3
    }
    Direc direction = Direc::right;
    size_t x = 1ULL, y = 1ULL;
public:
    inline PacMan() {}

    inline void set_direction(Direc x) {
        direction = x;
    }

    inline void update() {
        if (direction == Direc::right) {
            x += 1;
        }
        else if (direction == Direc::left) {
            x -= 1;
        }
        else if (direction == Direc::up) {
            y += 1;
        }
        else if (direction == Direc::down) {
            y -= 1;
        }
    }
};

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    ~MainWindow();
    MainWindow(QWidget *parent = nullptr);

    // 初始化地圖
    static constexpr size_t map_width = 18ULL, map_height = 15ULL;
    using MapType = std::array<std::array<Tile, map_width>, map_height>;
    MapType map;

    // 成員變數
    int tile_size = 30;

    void paintEvent(QPaintEvent *event) override;
    // 讀取檔案 file_path 作為地圖
    bool load_map(const std::string& file_path) {
        // 嘗試讀取地圖檔案
        std::ifstream file(file_path);
        if (!file.is_open()) return false;
        // 解析地圖檔案
        std::string line = "";
        for (size_t row = 0; std::getline(file, line) && row < map_height; ++row) {
            // 跑遍每一行, 每一列
            for (size_t col = 0; (col < line.length()) && (col < map_width); ++col) {
                /* 讀取一個個字元並解析成 Tile
                 * '1' 視為牆壁, ' '視為空地
                 * 若前兩者都不符合則補為空地
                */
                char character = line[col];
                map[row][col] = Tile::flat;
                if (character == '1') map[row][col] = Tile::wall;
            }
        }
        // 關閉地圖檔案(讀取)
        file.close();
        return true;
    }
    // 繪製地圖
    void draw_map(QPainter& painter) {
        for (int row = 0; row < map_height; ++row) {
            for (int col = 0; col < map_width; ++col) {
                // 計算每一格的繪製位置
                int x = col * tile_size;
                int y = row * tile_size;
                // 繪製該格
                if (map[row][col] == Tile::wall) {
                    // 畫牆壁
                    painter.setBrush(Qt::blue); // 深藍填充
                } else {
                    // 畫平地
                    painter.setBrush(Qt::black);
                }
                painter.setPen(QPen(Qt::NoPen));
                painter.drawRect(x, y, tile_size, tile_size);
            }
        }
    }
    void debug_print_map() {
        std::cout << "--- Debug Map Content ---" << std::endl;
        for (size_t r = 0; r < map_height; ++r) {
            for (size_t c = 0; c < map_width; ++c) {
                std::cout << (map[r][c] == Tile::wall ? "1" : " ");
            }
            std::cout << "|" << std::endl; // 行末加個邊界符號
        }
    }
private:
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
