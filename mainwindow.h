#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <array>       // std::array
#include <cstdint>     // size_t
#include <fstream>     // 操作檔案
// #include <iostream>    // std::cout
#include <QMainWindow> // 主視窗
#include <QPainter>    // 畫筆工具
#include <QKeyEvent>   // 鍵盤工具

using size_t  = std::size_t;

QT_BEGIN_NAMESPACE
namespace Ui {class MainWindow;}
QT_END_NAMESPACE

class MainWindow : public QMainWindow {
    Q_OBJECT
private:
    Ui::MainWindow *ui;
public:
    // constructor
    ~MainWindow();
    MainWindow(QWidget *parent = nullptr);
    void paintEvent(QPaintEvent *event) override;

    // 磚塊資料結構
    enum class Tile : uint8_t {
        flat = 0, // 可以通過的空地
        wall = 1  // 不可通過的牆壁
    };

    // 初始化地圖
    static constexpr int tile_size = 30;
    static constexpr size_t map_width = 18ULL, map_height = 15ULL;
    using MapType = std::array<std::array<Tile, map_width>, map_height>;
    MapType map;

    // 位置資料結構
    struct Pos {
        int x, y;
        MainWindow *parent = nullptr;
        inline Pos(const Pos& other) noexcept
            : x(other.x), y(other.y), parent(other.parent) {}
        inline Pos(MainWindow *_parent_, int _x_, int _y_) noexcept
            : x(_x_), y(_y_), parent(_parent_) {}
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
            return Pos(lhs.parent, lhs.x + rhs.x, lhs.y + rhs.y);
        }
        friend inline Pos operator - (const Pos& lhs, const Pos& rhs) noexcept {
            return Pos(lhs.parent, lhs.x - rhs.x, lhs.y - rhs.y);
        }
        // 利用 Pos 取得 Tile 位置
        inline Tile& tile() {
            bool is_out_of_range = (x < 0) || (y < 0) || (x >= map_width) || (y >= map_height);
            if (is_out_of_range) throw std::out_of_range("get_tile position out of range");
            return parent->map[y][x];
        }
    };

    // 物件: 小精靈
    class PacMan {
    private:
        enum class Direc : uint8_t {
            none  = 0,
            right = 1,
            left  = 2,
            up    = 3,
            down  = 4,
        };
        static constexpr int radius = tile_size * 0.8;
        // 成員變數
        MainWindow *parent = nullptr;
        Direc direction = Direc::none;
        Pos position = Pos(parent, 1, 1);
        const int max_angle = 45;
        int mouth_angle = 0;
        int angle_step  = 5;
    public:
        // 建構子
        inline PacMan(MainWindow *_parent_) noexcept
            : parent(_parent_) {}
        // 成員函數
        inline void set_direction(Direc direc) {
            direction = direc;
        }
        inline Pos get_move(Direc direc) {
            switch(direc) {
                case Direc::left:  return Pos(parent, -1, +0);
                case Direc::right: return Pos(parent, +1, +0);
                case Direc::up:    return Pos(parent, +0, -1);
                case Direc::down:  return Pos(parent, +0, +1);
                default:           return Pos(parent, +0, +0);
            }
        }
        // 繪製小精靈
        inline void draw(QPainter& painter, int angle_open) {
            painter.setRenderHint(QPainter::Antialiasing); // 避免鋸齒狀
            painter.setBrush(Qt::yellow);               // 黃色圓心
            painter.setPen(QPen(Qt::black, 1));         // 黑色外框
            // 計算張開嘴巴的角度
            mouth_angle += angle_step;
            if (mouth_angle >= max_angle || mouth_angle <= 0) {
                angle_step = -angle_step;
            }
            int start_angle = (mouth_angle / 2) * 16;    // 扇形始邊
            int span_angle  = (360 - mouth_angle) * 16;  // 扇形終邊
            // 繪製黃色扇形
            int x = position.x-radius;
            int y = position.y-radius;
            QRectF rect(x, y, radius*2, radius*2);
            painter.drawPie(rect, start_angle, span_angle);
            // 繪製眼睛
            x = position.x+radius/4;
            y = position.y+radius/4;
            painter.setBrush(Qt::black);
            painter.drawEllipse(x, y, radius/4, radius/4);
        }
        // 更新狀態並繪製
        inline void update() {
            Pos destination = position + get_move(direction);
            if (destination.tile() != Tile::wall) {
                position = destination;
            }
        }
    };

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
                painter.setBrush(map[row][col] == Tile::wall ? Qt::blue : Qt::black);
                painter.setPen(QPen(Qt::NoPen));
                painter.drawRect(x, y, tile_size, tile_size);
            }
        }
    }
};

#endif // MAINWINDOW_H
