#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <map>         // std::map
#include <array>       // std::array
#include <cstdint>     // size_t
#include <fstream>     // 操作檔案
// #include <iostream>    // std::cout
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

static constexpr int tile_size = 30;
// 初始化地圖
static constexpr size_t map_width = 18ULL, map_height = 15ULL;
using MapType = std::array<std::array<Tile, map_width>, map_height>;
inline MapType map;

struct Pos {
    int x, y;
    inline Pos(const Pos& other) noexcept {
        x = other.x;
        y = other.y;
    }
    inline Pos(int _x_, int _y_) noexcept : x(_x_), y(_y_) {}
    inline Pos& operator = (const Pos& other) noexcept {
        x = other.x;
        y = other.y;
        return (*this);
    }
    // 算術運算子
    inline Pos& operator += (const Pos& other) noexcept {
        x += other.x;
        y += other.y;
        return (*this);
    }
    inline Pos& operator -= (const Pos& other) noexcept {
        x -= other.x;
        y -= other.y;
        return (*this);
    }
    friend inline Pos operator + (const Pos& lhs, const Pos& rhs) noexcept {
        return Pos(lhs.x + rhs.x, lhs.y + rhs.y);
    }
    friend inline Pos operator - (const Pos& lhs, const Pos& rhs) noexcept {
        return Pos(lhs.x - rhs.x, lhs.y - rhs.y);
    }
};

class PacMan {
private:
    enum class Direc : uint8_t {
        right = 0,
        left = 1,
        up = 2,
        down = 3
    };
    Direc direction = Direc::right;
    Pos position{1, 1};
public:
    std::map<Direc, Pos> moves = {
        {Direc::left , Pos(-1, +0)},
        {Direc::right, Pos(+1, +0)},
        {Direc::up   , Pos(+0, +1)},
        {Direc::down , Pos(+0, -1)},
    };
    inline PacMan() noexcept {}
    inline void set_direction(Direc direc) {
        direction = direc;
    }
    inline Tile& get_tile(Pos pos) {
        if (pos.x < 0 || pos.y < 0) throw std::out_of_range("get_tile position out of range");
        return map[pos.y][pos.x];
    }
    inline void update() {
        Pos destination = position + moves.at(direction);
        if (get_tile(destination) != Tile::wall) {
            position = destination;
        }
    }
};

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    ~MainWindow();
    MainWindow(QWidget *parent = nullptr);
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
private:
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
