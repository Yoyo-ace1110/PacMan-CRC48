#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <queue>            // std::queue
#include <array>            // std::array
#include <vector>           // std::vector
#include <string>           // string
#include <cstdint>          // size_t
#include <fstream>          // 操作檔案
#include <unordered_map>    // BFS
#include <QMainWindow>      // 主視窗
#include <QCoreApplication> // 刷新畫面
#include <QPainterPath>     // 進階畫筆
#include <QMessageBox>      // 彈出視窗
#include <QKeyEvent>        // 鍵盤工具
#include <QPainter>         // 畫筆工具
#include <QThread>          // 暫停時間
#include <QTimer>           // 計時工具
#include <QPoint>           // 位置資訊
#include <QTime>            // 獨立時間
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
    int count = 0;              // 繪圖計時器
    int score = 0;              // 累積的分數
    int dots_amount = 0;        // 剩餘點點的數量
    int freeze_timer = 0;       // 畫面定格剩餘時間
    int pellet_timer = 0;       // 藥丸生效剩餘時間
    bool is_frozen = false;     // 紀錄畫面是否定格
    int ghosts_eaten_count = 0; // 已經吃掉的鬼魂數量
    GameState state = GameState::normal;  // 遊戲狀態
    static constexpr int tile_size = 30;  // 磁磚大小
    static constexpr int dot_radius = 2;  // 點點半徑
    static constexpr int pill_radius = 8; // 藥丸半徑
    int scorebar_height = 30;                          // 橫條高度
    int width  = map_width*tile_size;                  // 視窗寬度
    int height = map_height*tile_size+scorebar_height; // 視窗高度

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
        inline constexpr Pos(const Pos& other) noexcept : x(other.x), y(other.y) {}
        inline constexpr Pos(int _x_, int _y_) noexcept : x(_x_),     y(_y_) {}
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
            return Pos(lhs.x + rhs.x, lhs.y + rhs.y);
        }
        friend inline Pos operator - (const Pos& lhs, const Pos& rhs) noexcept {
            return Pos(lhs.x - rhs.x, lhs.y - rhs.y);
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
        score += 10;
    }
    void try_eat_power_pellet(const Pos& pos) {
        if (get_tile(pos) != Tile::power_pellet) return;
        score += 50;
        dots_amount -= 1;
        set_state_chasing();
        ghosts_eaten_count = 0;
        pellet_timer = 7 * fps;
        get_tile(pos) = Tile::flat;
    }
    void update_pellet_timer() noexcept {
        if (pellet_timer > 0) {
            --pellet_timer;
            if (pellet_timer  <= 2*fps) [[unlikely]] {
                set_state_flashing();
            } else if (pellet_timer == 0) [[unlikely]] {
                set_state_normal();
            }
        }
    }

    // 物件: 小精靈
    class PacMan {
    private:
        // 成員變數
        MainWindow *parent = nullptr;
        Direc direction = Direc::none;      // 現在的移動方向
        Direc direc_buffer = Direc::none;   // 移動方向緩衝區
        Pos position = Pos(9, 14);          // 預設出生點位置
        static constexpr int radius = tile_size * 0.4;
        const int pacman_speed = 6; // 移動速度
        const int max_angle = 72;   // 最大張嘴角度
        int mouth_angle = 0;        // 當前張嘴角度
        int angle_step  = 24;       // 每次張嘴的角位移
        // 轉動移動方向
        inline constexpr void _turn(Direc new_direc) noexcept {
            direc_buffer = new_direc;
        }
    public:
        // 建構子
        inline constexpr PacMan() noexcept {}
        inline constexpr void init(MainWindow *_parent_) noexcept {
            parent = _parent_;
        }
        // 成員函數
        inline constexpr Pos get_position() const noexcept {
            return position;
        }
        inline constexpr Pos get_move(Direc direc) const noexcept {
            switch(direc) {
            case Direc::left:  return Pos(-1, +0);
            case Direc::right: return Pos(+1, +0);
            case Direc::up:    return Pos(+0, -1);
            case Direc::down:  return Pos(+0, +1);
            default:           return Pos(+0, +0);
            }
        }
        // 繪製小精靈
        inline Pos get_pixel_pos() const noexcept {
            // 格子座標
            Pos move = get_move(direction);
            Pos destination = position + move;
            if (!parent->is_walkable(destination)) {
                move = Pos(0, 0);
            }
            // 像素座標
            int count = parent->count % (fps/pacman_speed);
            double ratio = count * (static_cast<double>(pacman_speed)/fps);
            int offset_x = static_cast<double>((move.x * tile_size) * ratio);
            int offset_y = static_cast<double>((move.y * tile_size) * ratio);
            int center_x = position.x * tile_size + (tile_size / 2);
            int center_y = position.y * tile_size + (tile_size / 2);
            return Pos(center_x + offset_x, center_y + offset_y);
        }
        inline void draw(QPainter& painter, const Pos& move) {
            // 渲染相關設定
            painter.setRenderHint(QPainter::Antialiasing);  // 避免鋸齒狀
            painter.setBrush(Qt::yellow);                   // 黃色圓心
            painter.setPen(QPen(Qt::black, 1));             // 黑色外框
            // 計算張開嘴巴的角度
            int base_angle;
            mouth_angle += angle_step;
            if (mouth_angle >= max_angle || mouth_angle <= 0) {
                angle_step = -angle_step;
            }
            if (move.x == 0 && move.y == 0)     base_angle = 0;     // 預設
            else if (move.x > 0 && move.y == 0) base_angle = 0;     // 右
            else if (move.x < 0 && move.y == 0) base_angle = 180;   // 左
            else if (move.y < 0 && move.x == 0) base_angle = 90;    // 上
            else if (move.y > 0 && move.x == 0) base_angle = 270;   // 下
            else throw std::invalid_argument("wrong move");
            // Qt 的繪圖角度是 1/16 度
            int start_angle = (base_angle + mouth_angle/2) * 16;
            int span_angle  = (360 - mouth_angle) * 16;
            // 繪製黃色扇形
            Pos pixel = get_pixel_pos();
            QRectF rect(pixel.x-radius, pixel.y-radius, radius*2, radius*2);
            painter.drawPie(rect, start_angle, span_angle);
        }
        // 更新狀態&繪製
        inline void update() {
            // 計算相對於小精靈移動速度的計數器
            int pacman_count = parent->count % (fps/pacman_speed);
            // 取得目的地
            Pos this_move = get_move(direction);
            Pos destination = position + this_move;
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
                bool buffer_is_not_none = (direc_buffer != Direc::none);
                bool buffer_walkable = parent->is_walkable(get_move(direc_buffer));
                bool destination_walkable = parent->is_walkable(parent->get_tile(destination));
                if (buffer_is_not_none && (buffer_walkable || !destination_walkable)) {
                    direction = direc_buffer;
                }
            }
        }
        inline void paint(QPainter& painter) {
            // 沒吃到鬼魂就繪製
            if (parent->is_frozen) return;
            this->draw(painter, get_move(direction));
        }
        // slots
        inline constexpr void turn_left () noexcept { this->_turn(Direc::left ); }
        inline constexpr void turn_right() noexcept { this->_turn(Direc::right); }
        inline constexpr void turn_up   () noexcept { this->_turn(Direc::up   ); }
        inline constexpr void turn_down () noexcept { this->_turn(Direc::down ); }
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
        // 各種情況的顏色(實作在下方)                                      // 正常身體
        static inline const QColor normal_eye   = QColor(255, 255, 255); // 正常眼睛
        static inline const QColor normal_pupil = QColor(33, 33, 255);   // 正常瞳孔
        static inline const QColor scared_body  = QColor(33, 33, 255);   // 驚嚇身體
        static inline const QColor scared_eye   = QColor(216, 216, 200); // 驚嚇眼睛
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
        Pos position = Pos(0, 0);               // 自身位置
        Pos spawn_pos = Pos(0, 0);              // 出生位置
        Direc direction = Direc::none;          // 移動方向
        int delay_timer = 0;                    // 延遲出鬼門
        bool gate_walkble = true;               // 能否通過鬼門
        std::vector<Pos> best_path = {};        // 紀錄最短路徑
        QColor normal_body = QColor(0, 0, 0);   // 正常身體顏色
        const int eaten_speed = 4;              // 鬼魂回家的速度
        const int normal_speed = 6;             // 鬼魂移動的速度
        const int scared_speed = 4;             // 鬼魂被追逐的速度
        // 虛擬函數
        inline virtual void init(
            const Pos& pos, 
            MainWindow *_parent_, 
            const QColor& body_color, 
            double delay_second,
            bool born_in_gate
        ) noexcept {
            spawn_pos = pos;
            parent = _parent_;
            position = spawn_pos;
            normal_body = body_color;
            gate_walkble = born_in_gate;
            delay_timer = fps * delay_second;
            direction = Direc::left; // 預設方向
        }
        // 取得速度
        inline constexpr int get_speed() const noexcept {
            switch(status) {
                case State::eaten:      {return eaten_speed ;}
                case State::normal:     {return normal_speed;}
                case State::scared:     {return scared_speed;}
                case State::flashing:   {return scared_speed;}
                default                 {return 0;          }
            };
        }
        // 方向向量
        inline constexpr Pos get_move(Direc direc) const noexcept {
            switch(direc) {
                case Direc::left:  return Pos(-1, +0);
                case Direc::right: return Pos(+1, +0);
                case Direc::up:    return Pos(+0, -1);
                case Direc::down:  return Pos(+0, +1);
                default:           return Pos(+0, +0);
            }
        }
        // 渲染鬼魂
        inline void draw(QPainter& painter) const {
            // 計算平滑移動比例
            const int speed = get_speed();
            const double ratio = parent->count % (fps/speed);
            // 計算像素位置
            const Pos move = get_move(direction);
            int offset_x = static_cast<double>((move.x * tile_size) * ratio);
            int offset_y = static_cast<double>((move.y * tile_size) * ratio);
            int center_x = position.x * tile_size + (tile_size / 2);
            int center_y = position.y * tile_size + (tile_size / 2);
            Pos pixel = Pos(center_x + offset_x, center_y + offset_y);
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
            default: throw std::runtime_error("Unkown status");
            }
            // 繪製身體與眼睛
            painter.setRenderHint(QPainter::Antialiasing);
            int radius = tile_size * 0.45;
            // 繪製鬼魂身體
            if (body_color != Qt::transparent) {
                QPainterPath bodyPath;
                // 上半身: 繪製一個半圓弧
                bodyPath.arcMoveTo(pixel.x-radius, pixel.y-radius, radius*2, radius*2, 180);
                bodyPath.arcTo(pixel.x-radius, pixel.y-radius, radius*2, radius*2, 180, -180);
                // 往下畫到右下角的身體邊緣
                bodyPath.lineTo(pixel.x+radius, pixel.y+radius);
                // 下半身: 3個波浪裙擺
                int h_offset = 3;
                double wave = radius * 2.0 / 3.0;
                bodyPath.lineTo(pixel.x + radius - wave * 0.5, pixel.y + radius - h_offset);
                bodyPath.lineTo(pixel.x + radius - wave,       pixel.y + radius);
                bodyPath.lineTo(pixel.x + radius - wave * 1.5, pixel.y + radius - h_offset);
                bodyPath.lineTo(pixel.x + radius - wave * 2.0, pixel.y + radius);
                bodyPath.lineTo(pixel.x + radius - wave * 2.5, pixel.y + radius - h_offset);
                bodyPath.lineTo(pixel.x - radius,              pixel.y + radius);
                bodyPath.closeSubpath();
                // 同時繪製上去
                painter.setBrush(body_color);
                painter.setPen(QPen(Qt::NoPen));
                painter.drawPath(bodyPath);
            }
            // 繪製眼白
            painter.setBrush(eye_color);
            int eye_width = radius * 0.35;
            int eye_height = radius * 0.35;
            int eye_offset_x = radius * 0.4;
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
        // 虛擬函數
        inline Ghost() noexcept = default;
        inline virtual ~Ghost() noexcept = default;
        inline virtual update_direction() noexcept;
        // 成員函數
        inline State get_status() const noexcept { return status; }
        inline void set_status(State _status_) noexcept { status = _status_; }
        inline bool collides_with(const MainWindow::PacMan& pacman) const noexcept {
            return (position == pacman.get_position());
        }
        inline bool can_pass_through(const Pos& pos) const noexcept {
            if (parent->get_tile(pos) == Tile::gate) return gate_walkble;
            if (parent->get_tile(pos) == Tile::wall) return false;
            return true;
        } 
        inline void pass_position(const Pos& pos) noexcept {
            if (parent->get_tile(pos) == Tile::gate) {
                gate_walkble = false;
            }
            position = pos;
        }
        inline void BFS_path(const Pos& target) const noexcept {
            best_path.clear();
            if (position == target) return;
            // 轉為 key in unordered_map
            auto to_key = [](const Pos& p) -> int {
                return (p.x << 16) | p.y;
            };
            // 佇列
            std::queue<Pos> queue;
            std::unordered_map<int, Pos> parent_map;
            // 初始化起點
            bool found = false;
            queue.push(position);
            parent_map[to_key(position)] = position;
            Direc check_dirs[] = { Direc::left, Direc::right, Direc::up, Direc::down };
            // 開始 BFS 
            while (!queue.empty()) {
                Pos curr = queue.front();
                queue.pop();
                // 找到目標提早結束
                if (curr == target) {
                    found = true;
                    break;
                }
                // 四個方向
                for (Direc direc : check_dirs) {
                    Pos next_pos = curr + get_move(direc);
                    int next_key = to_key(next_pos);
                    if (parent_map.find(next_key) == parent_map.end()) {
                        if (can_pass_through(next_pos)) {
                            parent_map[next_key] = curr;
                            queue.push(next_pos);
                        }
                    }
                }
            }
            // 找不到路徑
            if (!found) return;
            // 反向追蹤回起點
            Pos trace = target;
            while (!(trace == position)) {
                best_path.push_back(trace);
                trace = parent_map[to_key(trace)];
            }
            std::reverse(best_path.begin(), best_path.end());
        }
        inline void update_status() noexcept {
            if (status == State::eaten) return;
            // 將主視窗的 GameState 轉為 Ghost::State
            uint8_t temp = (uint8_t)parent->state;
            status = static_cast<State>(temp);
        }
        inline void handle_eaten_direc() noexcept {
            if (status != State::eaten) return;
            // 沿著 best_path 的方向走
            Pos next_step = best_path[0];
            if (next_step.x < position.x)      direction = Direc::left;
            else if (next_step.x > position.x) direction = Direc::right;
            else if (next_step.y < position.y) direction = Direc::up;
            else if (next_step.y > position.y) direction = Direc::down;
        }
        // 繪製和更新
        inline void paint(QPainter& painter) {
            if (parent->eaten_ghost_ptr == this) return;
            draw(painter);
        }
        inline void update() noexcept {
            update_status();
            // 開始移動
            if (delay_timer == 0) {
                int speed = get_speed();
                Pos move = get_move(direction);
                // 完成循環
                int count = parent->count%(fps/speed);
                Pos destination = position + move;
                if ((count == 0) && can_pass_through(destination)) {
                    pass_position(destination);
                }
            } else { --delay_timer; }
        }
    };

    // 繼承: 紅鬼
    class Blinky : public Ghost {
    private:
        // 計算兩點之間的距離平方
        inline int distance_squared(const Pos& p1, const Pos& p2) const noexcept {
            int dx = p1.x - p2.x;
            int dy = p1.y - p2.y;
            return dx * dx + dy * dy;
        }
    public:
        inline Blinky() noexcept : Ghost() {}
        inline void init(MainWindow *_parent_) noexcept {
            Ghost::init(_parent_, Pos(9, 1), QColor(255, 0, 0));
        }
        inline void update() noexcept override {
            update_status();
        }
    } blinky;

    // 繼承: 粉鬼
    class Pinky : public Ghost {
    public:
        inline Pinky() noexcept : Ghost() {}
        inline void init(MainWindow *_parent_) noexcept {
            Ghost::init(_parent_, Pos(8, 3), QColor(255, 184, 255));
        }
    } pinky;

    // 繼承: 青鬼
    class Inky : public Ghost {
    public:
        inline Inky() noexcept : Ghost() {}
        inline void init(MainWindow *_parent_) noexcept {
            Ghost::init(_parent_, Pos(9, 3), QColor(0, 255, 255));
        }
    } inky;

    // 繼承: 橘鬼
    class Clyde : public Ghost {
    public:
        inline Clyde() noexcept : Ghost() {}
        inline void init(MainWindow *_parent_) noexcept {
            Ghost::init(_parent_, Pos(10, 3), QColor(255, 184, 82));
        }
    } clyde;

    // 吃掉鬼魂相關資訊
    Pos collision_pos = Pos(0, 0);
    Ghost* eaten_ghost_ptr = nullptr;
    // 鬼魂指標陣列
    std::array<Ghost*, 4> ghosts = {
        &blinky,
        &pinky,
        &inky,
        &clyde
    };

    // 宣告主迴圈
    void main_loop();
    // 讀取地圖
    void load_map(const std::string& file_path) {
        // 嘗試讀取地圖檔案
        dots_amount = 0;
        std::ifstream file(file_path);
        if (!file.is_open()) throw std::runtime_error("Cannot open the map");
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
                 * 'O' 視為藥丸
                 * '=' 視為鬼屋的門
                 */
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
                    Pos center = Pos(x + tile_size/2, y + tile_size/2);
                    painter.drawEllipse(get_point(center), dot_radius, dot_radius);
                }
                // 繪製小藥丸(3Hz)
                int current_msec = QTime::currentTime().msec();
                bool is_visible = (current_msec % 333) < 166;
                if (map[row][col] == Tile::power_pellet && is_visible) {
                    painter.setBrush(QColor(255, 204, 184));
                    painter.setRenderHint(QPainter::Antialiasing);
                    Pos center = Pos(x + tile_size/2, y + tile_size/2);
                    painter.drawEllipse(get_point(center), pill_radius, pill_radius);
                }
            }
        }
    }
    // 繪製狀態列
    void draw_scorebar(QPainter& painter) const noexcept {
        int start_y = map_height*tile_size;
        // 繪製背景
        painter.setPen(Qt::NoPen);
        painter.setBrush(QColor(0, 0, 0));
        painter.drawRect(0, start_y, width, scorebar_height);
        // 繪製分數
        painter.setPen(Qt::white);
        painter.setRenderHint(QPainter::Antialiasing);
        painter.setFont(QFont("Arial", 11, QFont::Bold));
        QString score_text = "SCORE: " + QString::number(score);
        QRect text_rect(10, start_y, width - 20, scorebar_height);
        painter.drawText(text_rect, Qt::AlignLeft | Qt::AlignVCenter, score_text);
    }
    // 繪製鬼魂們
    void paint_ghosts(QPainter& painter) const noexcept {
        for (Ghost* ghost : ghosts) {
            ghost->paint(painter);
        }
    }
    // 更新鬼魂們
    void update_ghosts() const noexcept {
        for (Ghost* ghost : ghosts) {
            ghost->update();
        }
    }
    // 處理過關邏輯
    void handle_passed() noexcept {
        // 確保地上小點點被吃光
        if (dots_amount != 0) [[likely]] return;
        QMessageBox::information(this, "Game Over", "You Win!");
        this->close();
    }
    // 找出相撞鬼魂
    Ghost* collided_ghost() noexcept {
        for (Ghost* ghost : ghosts) {
            bool was_eaten = (ghost->get_status() == Ghost::State::eaten);
            if (ghost->collides_with(player) && !was_eaten) [[unlikely]] {
                return ghost;
            }
        }
        return nullptr;
    }
    // 處理碰撞邏輯
    void handle_collision() noexcept {
        if (!collided_ghost()) return;
        Ghost* ghost = collided_ghost();
        bool normal_state = (state == GameState::normal);
        if (normal_state) [[unlikely]] {
            // 正常情況撞到 -> 輸了
            QMessageBox::information(this, "Game Over", "You lose");
            this->close();
        } else [[likely]] {
            // 小精靈把鬼魂吃掉 -> 得分
            ghost->set_status(Ghost::State::eaten);
            score += (200 << ghosts_eaten_count++);
            collision_pos = player.get_pixel_pos();
            eaten_ghost_ptr = ghost;
            freeze_timer = 0.5*fps;
            is_frozen = true;
        }
    }
    // 吃掉鬼魂得分畫面
    void paint_eaten_score(QPainter& painter) noexcept {
        painter.setPen(QColor(216, 216, 200));
        painter.setRenderHint(QPainter::Antialiasing);
        painter.setFont(QFont("Arial", 11, QFont::Bold));
        int offset = tile_size * 0.75;
        int rect_size = tile_size * 1.5;
        QPoint point = get_point(collision_pos);
        QRect text_rect(point.x()-offset, point.y()-offset, rect_size, rect_size);
        painter.drawText(text_rect, Qt::AlignCenter, QString::number(200<<ghosts_eaten_count));
    }
};

#endif // MAINWINDOW_H
