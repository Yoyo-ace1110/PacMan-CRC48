#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <array>        // std::array
#include <fstream>      // 操作檔案
#include <cstdint>      // size_t
#include <string>       // string
#include <QMainWindow>  // 主視窗
#include <QPainter>     // 畫筆工具
#include <QPainterPath> // 進階畫筆
#include <QKeyEvent>    // 鍵盤工具
#include <QTimer>       // 計時工具
#include <QPoint>       // 位置資訊
#include <QTime>        // 獨立時間
#include <QMessageBox>  // 彈出視窗
using size_t = std::size_t;

/* TODO:
 * 實作鬼魂出生點
 * 讓四個鬼魂繼承 Ghost
 * 碰撞偵測(小精靈/鬼魂) */

QT_BEGIN_NAMESPACE
namespace Ui {class MainWindow;}
QT_END_NAMESPACE

static constexpr int fps = 24;

// 方向列舉
enum class Direc : uint8_t {
    none  = 0,
    right = 1,
    left  = 2,
    up    = 3,
    down  = 4,
};

// 遊戲狀態
enum class GameState : uint8_t {
    normal   = 0,
    chasing  = 1,
    flashing = 2,
};

class MainWindow : public QMainWindow {
    Q_OBJECT
private:
    Ui::MainWindow *ui;
    QTimer *timer; // 計時器
public:
    // Constructor
    ~MainWindow();
    MainWindow(QWidget *parent = nullptr);
    void paintEvent(QPaintEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;

    // 磚塊資料結構
    enum class Tile : uint8_t {
        flat          = 0,  // 空地
        wall          = 1,  // 牆壁
        dot           = 2,  // 點點
        gate          = 3,  // 鬼屋的門
        power_pellet  = 4,  // 神奇藥丸
    };

    // 初始化變數
    inline int count = 0;                 // 計時器
    inline int score = 0;                 // 累積分數
    inline int dots_amount = 0;           // 點點數量
    GameState state = GameState::normal;  // 遊戲狀態
    static constexpr int tile_size = 30;  // 磁磚大小
    static constexpr int dot_radius = 2;  // 點點半徑
    static constexpr int pill_radius = 8; // 藥丸半徑
    // 初始化地圖
    static constexpr size_t map_width  = 19ULL;
    static constexpr size_t map_height = 16ULL;
    using row_type = std::array<Tile, map_width>;
    std::array<row_type, map_height> map;

    // 設定遊戲狀態
    constexpr void set_state_normal   () noexcept { state = GameState::normal; }
    constexpr void set_state_chasing  () noexcept { state = GameState::chasing; }
    constexpr void set_state_flashing () noexcept { state = GameState::flashing; }

    // 位置資料結構
    struct Pos {
        int x, y;
        MainWindow *parent = nullptr;
        inline constexpr Pos(const Pos& other) noexcept
            : x(other.x), y(other.y), parent(other.parent) {}
        inline constexpr Pos(MainWindow *_parent_, int _x_, int _y_) noexcept
            : x(_x_), y(_y_), parent(_parent_) {}
        inline constexpr Pos& operator = (const Pos& other) noexcept {
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
        // 比較運算子
        friend inline bool operator == (const Pos& lhs, const Pos& rhs) noexcept {
            return (lhs.x == rhs.x && lhs.y == rhs.y);
        }
        friend inline bool operator != (const Pos& lhs, const Pos& rhs) noexcept {
            return (lhs.x != rhs.x || lhs.y != rhs.y);
        }
    };

    // 一般函數
    void map_boundary_check(const Pos& pos) const {
        bool x_out_of_range = (pos.x < 0) || (pos.x >= map_width) ;
        bool y_out_of_range = (pos.y < 0) || (pos.y >= map_height);
        if (x_out_of_range || y_out_of_range) {
            throw std::out_of_range("Position out of map");
        }
    }
    QPoint get_point(const Pos& pos) const {
        return QPoint(pos.x, pos.y);
    }
    Tile   get_tile (const Pos& pos) const {
        map_boundary_check(pos);
        return map[pos.y][pos.x];
    }
    Tile&  get_tile (const Pos& pos)       {
        map_boundary_check(pos);
        return map[pos.y][pos.x];
    }
    bool is_walkable(const Pos& pos) const {
        bool is_not_wall = (get_tile(pos) != Tile::wall);
        bool is_not_gate = (get_tile(pos) != Tile::gate);
        return (is_not_gate && is_not_wall);
    }
    void try_eat_dot(const Pos& pos) {
        if (get_tile(pos) != Tile::dot) return;
        map[pos.y][pos.x] = Tile::flat;
        dots_amount -= 1;
    }
    void try_eat_power_pellet(const Pos& pos) {
        if (get_tile(pos) != Tile::power_pellet) return;
        dots_amount -= 1;
        set_state_chasing();
        get_tile(pos) = Tile::flat;
        QTimer::singleShot(5000, this, &MainWindow::set_state_flashing);
        QTimer::singleShot(7000, this, &MainWindow::set_state_normal);
    }

    // 物件: 小精靈
    class PacMan {
    private:
        // 成員變數
        MainWindow *parent = nullptr;
        Direc direction = Direc::none;      // 現在的移動方向
        Direc direc_buffer = Direc::none;   // 移動方向緩衝區
        Pos position = Pos(parent, 10, 14); // 預設出生點位置
        static constexpr int radius = tile_size * 0.4;
        const int pacman_speed = 6; // 小精靈移動的速度
        const int max_angle = 72;   // 最大張嘴角度
        int mouth_angle = 0;        // 當前張嘴角度
        int angle_step  = 24;       // 每次張嘴的角位移
        // 轉動移動方向
        inline void _turn(Direc new_direc) {
            if (direc_buffer == new_direc) return;
            // 計算移動的向量
            Pos this_move = get_move(direction);
            Pos next_move = get_move(new_direc);
            Pos destination = position+this_move;
            Pos next_destination = position+this_move;
            if (this_move != next_move) {
                next_destination += next_move;
            }
            // 撞牆時可以改變方向
            bool move_is_valid = parent->is_walkable(destination);
            // 轉方向之後不能撞牆
            bool next_move_is_valid = parent->is_walkable(next_destination);
            if (!move_is_valid || next_move_is_valid) {
                direc_buffer = new_direc;
            }
        }
    public:
        // 建構子
        inline PacMan() noexcept {}
        inline void init(MainWindow *_parent_) {
            parent = _parent_;
            position.parent = _parent_;
        }
        // 成員函數
        inline Pos get_position() const {
            return position;
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
        inline void draw(QPainter& painter, const Pos& move, int count) {
            // 設定前進比例
            double ratio = count * (static_cast<double>(pacman_speed)/fps);
            // 渲染相關設定
            painter.setRenderHint(QPainter::Antialiasing);  // 避免鋸齒狀
            painter.setBrush(Qt::yellow);                   // 黃色圓心
            painter.setPen(QPen(Qt::black, 1));             // 黑色外框
            // 計算張開嘴巴的角度
            mouth_angle += angle_step;
            if (mouth_angle >= max_angle || mouth_angle <= 0) {
                angle_step = -angle_step;
            }
            int base_angle;
            if (move.x == 0 && move.y == 0)     base_angle = 0;     // 預設向右
            else if (move.x > 0 && move.y == 0) base_angle = 0;     // 右
            else if (move.x < 0 && move.y == 0) base_angle = 180;   // 左
            else if (move.y < 0 && move.x == 0) base_angle = 90;    // 上
            else if (move.y > 0 && move.x == 0) base_angle = 270;   // 下
            else throw std::invalid_argument("wrong move");
            // Qt 的繪圖角度是 1/16 度
            int start_angle = (base_angle + mouth_angle/2) * 16;
            int span_angle  = (360 - mouth_angle) * 16;
            // 繪製黃色扇形
            int offset_x = static_cast<double>((move.x * tile_size) * ratio);
            int offset_y = static_cast<double>((move.y * tile_size) * ratio);
            int center_x = position.x * tile_size + (tile_size / 2);
            int center_y = position.y * tile_size + (tile_size / 2);
            Pos pixel = Pos(parent, center_x + offset_x, center_y + offset_y);
            QRectF rect(pixel.x-radius, pixel.y-radius, radius*2, radius*2);
            painter.drawPie(rect, start_angle, span_angle);
        }
        // 更新狀態並繪製
        inline void update(QPainter& painter) {
            // 計算相對於小精靈移動速度的計數器
            int pacman_count = parent->count % (fps/pacman_speed);
            // 取得目的地
            Pos move = get_move(direction);
            Pos destination = position + move;
            if (!parent->is_walkable(destination)) {
                pacman_count = 0;
            }
            // 完成一周期的循環
            if (pacman_count == 0) {
                // 嘗試前進一格
                if (parent->is_walkable(destination)) {
                    position = destination;
                    // 嘗試把小點點吃掉
                    parent->try_eat_dot(destination);
                    // 嘗試把小藥丸吃掉
                    parent->try_eat_power_pellet(destination);
                }
                // 更新方向
                if (direc_buffer != Direc::none) {
                    direction = direc_buffer;
                }
            }
            this->draw(painter, move, pacman_count);
        }
        // slots
        inline void turn_left () { this->_turn(Direc::left ); }
        inline void turn_right() { this->_turn(Direc::right); }
        inline void turn_up   () { this->_turn(Direc::up   ); }
        inline void turn_down () { this->_turn(Direc::down ); }
    } player;

    // 物件: 鬼魂
    class Ghost {
    public:
        enum class State : uint8_t {
            normal      = 0,
            scared      = 1,
            flashing    = 2,
            eaten       = 3,
        };
    private:
        // 各種情況的顏色(實作在下方)                                        // 正常身體
        static inline const QColor normal_eye   = QColor(255, 255, 255); // 正常眼睛
        static inline const QColor normal_pupil = QColor(33, 33, 255);   // 正常瞳孔
        static inline const QColor scared_body  = QColor(33, 33, 255);   // 驚嚇身體
        static inline const QColor scared_eye   = QColor(255, 184, 174); // 驚嚇眼睛
        static inline const QColor scared_pupil = scared_eye;            // 驚嚇瞳孔
        static inline const QColor flash_body   = QColor(255, 255, 255); // 閃爍身體
        static inline const QColor flash_eye    = QColor(255, 0, 0);     // 閃爍眼睛
        static inline const QColor flash_pupil  = flash_eye;             // 閃爍瞳孔
        static inline const QColor eaten_body   = QColor(0, 0, 0);       // 被吃身體
        static inline const QColor eaten_eye    = normal_eye;            // 被吃眼睛
        static inline const QColor eaten_pupil  = normal_pupil;          // 被吃瞳孔
    protected:
        // 成員變數
        State status = State::normal;           // 狀態
        MainWindow *parent = nullptr;           // 主視窗
        Pos position = Pos(parent, 0, 0);       // 自身位置
        QColor normal_body = QColor(0, 0, 0);   // 正常身體
        const int ghost_speed = 6;              // 鬼魂移動的速度
        const int rghost_speed = 4;             // 鬼魂被抓時的移速
        // 虛擬函數
        virtual Pos get_move();
        inline Ghost() noexcept {};
        inline virtual ~Ghost() noexcept = default;
        inline virtual void init(MainWindow *_parent_, const Pos& init_pos, const QColor& body_color) noexcept {
            parent = _parent_;
            position = init_pos;
            normal_body = body_color;
            position.parent = _parent_;
        }
        // 渲染鬼魂
        inline void draw(QPainter& painter, const Pos& move) const noexcept {
            // 計算平滑移動比例
            bool reverse = (parent->state != GameState::normal);
            const int speed = (reverse ? rghost_speed : ghost_speed);
            const double ratio = parent->count * (static_cast<double>(speed)/fps);
            // 計算像素位置
            int offset_x = static_cast<double>((move.x * tile_size) * ratio);
            int offset_y = static_cast<double>((move.y * tile_size) * ratio);
            int center_x = position.x * tile_size + (tile_size / 2);
            int center_y = position.y * tile_size + (tile_size / 2);
            Pos pixel = Pos(parent, center_x + offset_x, center_y + offset_y);
            // 判斷顏色
            QColor body_color, eye_color, pupil_color;
            switch (status) {
                // 正常狀態
                case State::normal: {
                    eye_color   = normal_eye;
                    body_color  = normal_body;
                    pupil_color = normal_pupil;
                    break;
                }
                // 驚嚇狀態
                case State::scared: {
                    eye_color   = scared_eye;
                    body_color  = scared_body;
                    pupil_color = scared_pupil;
                    break;
                }
                // 閃爍狀態
                case State::flashing: {
                    int current_msec = QTime::currentTime().msec();
                    bool toggle = (current_msec % 333) < 166;
                    if (toggle) {
                        body_color  = scared_body;
                        eye_color   = scared_eye;
                        pupil_color = scared_pupil;
                    } else {
                        body_color  = flash_body;
                        eye_color   = flash_eye;
                        pupil_color = flash_pupil;
                    }
                    break;
                }
                // 被吃掉了
                case State::eaten: {
                    body_color  = eaten_body;
                    eye_color   = eaten_eye;
                    pupil_color = eaten_pupil;
                    break;
                }
                default: {
                    throw std::runtime_error("Unkown status");
                    break;
                }
            }
            // 繪製身體與眼睛
            painter.setRenderHint(QPainter::Antialiasing);
            int radius = tile_size * 0.4;
            // 繪製鬼魂身體
            if (body_color != Qt::transparent) {
                QPainterPath bodyPath;
                // 上半身: 繪製一個半圓弧
                bodyPath.arcMoveTo(pixel.x-radius, pixel.y-radius, radius*2, radius*2, 180);
                bodyPath.arcTo(pixel.x-radius, pixel.y-radius, radius*2, radius*2, 180, -180);
                // 往下畫到右下角的身體邊緣
                bodyPath.lineTo(pixel.x+radius, pixel.y+radius);
                // 下半身: 3個波浪裙擺
                int wave = (radius*2)/3;
                bodyPath.lineTo(pixel.x + radius - wave * 0.5, pixel.y + radius - 4);
                bodyPath.lineTo(pixel.x + radius - wave,       pixel.y + radius);
                bodyPath.lineTo(pixel.x - radius + wave,       pixel.y + radius);
                bodyPath.lineTo(pixel.x - radius,              pixel.y + radius);
                bodyPath.closeSubpath();
                painter.setBrush(body_color);
                painter.setPen(QPen(Qt::NoPen));
                painter.drawPath(bodyPath);
            }
            // 繪製眼白
            painter.setBrush(eye_color);
            int eye_width = radius * 0.4;
            int eye_height = radius * 0.6;
            int eye_offset_x = radius * 0.35;
            QPoint left_eye (pixel.x - eye_offset_x, pixel.y - radius * 0.1);
            QPoint right_eye(pixel.x + eye_offset_x, pixel.y - radius * 0.1);
            painter.drawEllipse(left_eye,  eye_width, eye_height);
            painter.drawEllipse(right_eye, eye_width, eye_height);
            // 繪製瞳孔 (根據 move 微調位置)
            painter.setBrush(pupil_color);
            int pupil_radius = radius * 0.18;
            QPoint pupil_offset(move.x * 2, move.y * 2);
            painter.drawEllipse(left_eye  + pupil_offset, pupil_radius, pupil_radius);
            painter.drawEllipse(right_eye + pupil_offset, pupil_radius, pupil_radius);
        }
    public:
        inline void update_status() noexcept {
            if (status == State::eaten) return;
            // 將主視窗的 GameState 轉為 Ghost::State
            uint8_t temp = (uint8_t)parent->state;
            status = static_cast<State>(temp);
        }
        inline State get_status() const noexcept {
            return status;
        }
        inline void set_status(State _status_) noexcept { status = _status_; }
        inline bool collides_with(const MainWindow::PacMan& pacman) const noexcept {
            return (position == pacman.get_position());
        }
    };

    std::array<Ghost, 4> ghosts;

    // 讀取地圖
    bool load_map(const std::string& file_path) {
        // 嘗試讀取地圖檔案
        dots_amount = 0;
        std::ifstream file(file_path);
        if (!file.is_open()) return false;
        // 解析地圖檔案
        std::string line = "";
        for (size_t row = 0; std::getline(file, line) && row < map_height; ++row) {
            // 跑遍每一行, 每一列
            for (size_t col = 0; (col < line.length()) && (col < map_width); ++col) {
                char character = line[col];
                /* 讀取一個個字元並解析成 Tile
                 * '1' 視為牆壁
                 * ' ' 視為點點
                 * 'x' 視為空地
                 * 若前兩者都不符合則補為空地 */
                switch (character) {
                    // 牆壁
                    case '1': {
                        map[row][col] = Tile::wall;
                        break;
                    }
                    // 點點
                    case ' ': {
                        map[row][col] = Tile::dot;
                        dots_amount += 1;
                        break;
                    }
                    // 藥丸
                    case 'O': {
                        map[row][col] = Tile::power_pellet;
                        dots_amount += 1;
                        break;
                    }
                    // 空地
                    case 'x': {
                        map[row][col] = Tile::flat;
                        break;
                    }
                    // 鬼屋的門
                    case '=': {
                        map[row][col] = Tile::gate;
                        break;
                    }
                    // 未知符號
                    default: {
                        std::string msg = "Error: Unknown character ";
                        throw std::invalid_argument(msg + character);
                    }
                };
            }
        }
        // 關閉地圖檔案(讀取)
        file.close();
        return true;
    }
    // 繪製地圖
    void draw_map(QPainter& painter) const noexcept {
        for (int row = 0; row < map_height; ++row) {
            for (int col = 0; col < map_width; ++col) {
                // 計算每一格的繪製位置
                int x = col * tile_size;
                int y = row * tile_size;
                // 繪製該格背景顏色
                QColor color;
                switch (map[row][col]) {
                    case Tile::wall: { color = QColor(0  , 0  , 255); break; }
                    case Tile::gate: { color = QColor(216, 216, 200); break; }
                    default:         { color = QColor(0  , 0  , 0  ); break; }
                };
                painter.setBrush(color);
                painter.setPen(QPen(Qt::NoPen));
                painter.drawRect(x, y, tile_size, tile_size);
                // 繪製小點點
                if (map[row][col] == Tile::dot) {
                    painter.setBrush(QColor(255, 184, 174));
                    painter.setRenderHint(QPainter::Antialiasing);
                    Pos center = Pos(this, x + tile_size/2, y + tile_size/2);
                    painter.drawEllipse(get_point(center), dot_radius, dot_radius);
                }
                // 繪製小藥丸(3Hz)
                int current_msec = QTime::currentTime().msec();
                bool is_visible = (current_msec % 333) < 166;
                if (map[row][col] == Tile::power_pellet && is_visible) {
                    painter.setBrush(QColor(255, 204, 184));
                    painter.setRenderHint(QPainter::Antialiasing);
                    Pos center = Pos(this, x + tile_size/2, y + tile_size/2);
                    painter.drawEllipse(get_point(center), pill_radius, pill_radius);
                }
            }
        }
    }
    // 判斷是否過關
    bool handle_passed() noexcept {
        // 確保地上小點點被吃光
        if (dots_amount != 0) [[likely]] return;
        QMessageBox::information(this, "Game Over", "You Win!");
        this->close();
    }
    // 判斷小精靈是否被抓到
    void handle_collision() noexcept {
        bool normal_state = (state == GameState::normal);
        for (Ghost& ghost : ghosts) {
            // 避免重複被吃兩次
            bool was_eaten = (ghost.get_status() == Ghost::State::eaten);
            if (ghost.collides_with(player) && !was_eaten) [unlikely] {
                if (normal_state) [[unlikely]] {
                    // 正常情況撞到 -> 輸了
                    QMessageBox::information(this, "Game Over", "You lose");
                    this->close();
                } else [[likely]] {
                    // TODO: 小精靈把鬼魂吃掉
                    ghost.set_status(Ghost::State::eaten);
                }
            }
        }
    }
};

#endif // MAINWINDOW_H
