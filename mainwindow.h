#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <algorithm>        // std::shuffle
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
#include <QTime>            // 獨立時間
#include <QRandomGenerator> // 亂數種子
using size_t = std::size_t;

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
    QTimer *timer;
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

    // 鬼魂行為模式
    enum class BehaviorMode : uint8_t {
        scatter = 0, // 散開模式
        chase   = 1  // 追逐模式
    };
    struct ModePhase {
        BehaviorMode mode;
        int duration_sec;
    };
    static inline const std::vector<ModePhase> phase_schedule = {
        { BehaviorMode::scatter, 7 },
        { BehaviorMode::chase,   20 },
        { BehaviorMode::scatter, 7 },
        { BehaviorMode::chase,   20 },
        { BehaviorMode::scatter, 5 },
        { BehaviorMode::chase,   20 },
        { BehaviorMode::scatter, 5 },
        { BehaviorMode::chase,   -1 } // 永久追逐
    };
    size_t phase_index = 0;
    // 預設第一個階段散開 7 秒
    int behavior_timer = 7 * fps;                       
    BehaviorMode behavior_mode = BehaviorMode::scatter;

    // 初始化變數
    int lives = 3;                  // 小精靈生命
    int count = 0;                  // 繪圖計時器
    int score = 0;                  // 累積的分數
    int dots_amount = 0;            // 剩餘點點的數量
    int freeze_timer = 0;           // 畫面定格剩餘時間
    int pellet_timer = 0;           // 藥丸生效剩餘時間
    bool is_frozen = false;         // 紀錄畫面是否定格
    bool is_waiting_start = true;   // 是否在等待玩家按下移動鍵
    int ghosts_eaten_count = 0;     // 已經吃掉的鬼魂數量
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
        int x = 0, y = 0;
        inline constexpr Pos() noexcept {}
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
    bool is_walkable(const Pos& pos) const noexcept {
        // 邊界外直接視為不可通行（避免拋出例外）
        if (pos.x < 0 || pos.x >= (int)map_width ||
            pos.y < 0 || pos.y >= (int)map_height) {
            return false;
        }
        bool is_not_wall = (map[pos.y][pos.x] != Tile::wall);
        bool is_not_gate = (map[pos.y][pos.x] != Tile::gate);
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

        // 當吃下能量豆時，強制鬼魂同步狀態、清除舊路徑並掉頭一次
        for (Ghost* ghost : ghosts) {
            ghost->update_status();
            ghost->clear_path();
            ghost->reverse_direction();
        }
    }
    void update_pellet_timer() noexcept {
        if (pellet_timer == 0) [[unlikely]] {
            // 時間結束恢復正常
            set_state_normal();
            return;
        }
        if (pellet_timer == 2*fps) [[unlikely]] {
            // 時間剩兩秒閃爍
            set_state_flashing();
        }
        --pellet_timer;
    }
    void update_behavior_mode() noexcept {
        if (behavior_timer < 0) return;         // -1 為永久追逐模式，不需更新
        if (state != GameState::normal) return; // 處於驚嚇/閃爍狀態時暫停模式計時器
        if (behavior_timer > 0) --behavior_timer;
        else {
            // 切換至下一個階段
            if (phase_index + 1 < phase_schedule.size()) {
                phase_index++;
                behavior_mode = phase_schedule[phase_index].mode;
                int duration = phase_schedule[phase_index].duration_sec;
                if (duration > 0) {
                    behavior_timer = duration * fps;
                } else {
                    behavior_timer = -1; // 永久追逐
                }
                // 每當切換模式時，幽靈都會強制掉頭轉向一次
                trigger_ghosts_u_turn();
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
        const int pacman_speed = 5; // 移動速度
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
        // 重置位置與方向
        inline constexpr void reset() noexcept {
            position = Pos(9, 14);
            direction = Direc::none;
            direc_buffer = Direc::none;
            mouth_angle = 0;
            angle_step = 24;
        }
        // 成員函數
        inline constexpr Pos get_position() const noexcept {
            return position;
        }
        inline constexpr Direc get_direction() const noexcept {
            return direction;
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
            // 完成一周期的循環
            if (pacman_count == 0) {
                Pos this_move = get_move(direction);
                Pos destination = position + this_move;

                if (parent->is_walkable(destination)) {
                    // 下一格可以通過 => 先前進, 後切換方向
                    position = destination;
                    parent->try_eat_dot(destination);
                    parent->try_eat_power_pellet(destination);
                    // 前進後確認 buffer 方向從新位置可走才轉向，否則保持原方向
                    if (direc_buffer != Direc::none) {
                        Pos buffer_dest = position + get_move(direc_buffer);
                        if (parent->is_walkable(buffer_dest)) {
                            direction = direc_buffer;
                        }
                    }
                } else {
                    // 下一格不可通過 => 停在原地, 直接轉向
                    if (direc_buffer != Direc::none) {
                        Pos buffer_dest = position + get_move(direc_buffer);
                        if (parent->is_walkable(buffer_dest)) {
                            direction = direc_buffer;
                        }
                    }
                }
            }
        }
        inline void paint(QPainter& painter) {
            this->draw(painter, get_move(direction));
        }
        // slots
        inline constexpr void turn_left () noexcept { this->_turn(Direc::left ); }
        inline constexpr void turn_right() noexcept { this->_turn(Direc::right); }
        inline constexpr void turn_up   () noexcept { this->_turn(Direc::up   ); }
        inline constexpr void turn_down () noexcept { this->_turn(Direc::down ); }
    } pacman;

    // 物件: 鬼魂
    class Ghost {
    public:
        enum class State : uint8_t {
            normal      = 0,
            scared      = 1,
            flashing    = 2,
            eaten       = 3,
            immune      = 4,
        };
    private:
        // 各種情況的顏色                                                  // 正常身體
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
        const int eaten_speed = 8;              // 鬼魂回家的速度 (24/8=3幀一格)
        const int normal_speed = 4;             // 鬼魂移動的速度 (24/4=6幀一格)
        const int scared_speed = 2;             // 鬼魂被追逐的速度 (24/2=12幀一格)
        int immune_timer = 0;                   // 免疫剩餘幀數
        static constexpr int immune_duration = 7 * fps; // 免疫時長（與藥丸效果相同）
        bool pending_u_turn = false;            // 延遲至走完格子後執行的掉頭標記
        // 虛擬函數
        inline virtual void init(
            const Pos& pos, 
            MainWindow *_parent_, 
            const QColor& body_color, 
            double delay_second,
            bool born_in_gate
        ) noexcept {
            // 亂數種子:  x=9~11, y=3
            int rand_x = 8 + QRandomGenerator::global()->bounded(3);
            spawn_pos = Pos(rand_x, 3);
            parent = _parent_;
            position = spawn_pos;
            direction = Direc::none;
            normal_body = body_color;
            gate_walkble = born_in_gate;
            delay_timer = fps * delay_second;
        }
        // 取得速度
        inline constexpr int get_speed() const noexcept {
            switch(status) {
                case State::eaten:      {return eaten_speed ;}
                case State::normal:     {return normal_speed;}
                case State::scared:     {return scared_speed;}
                case State::flashing:   {return scared_speed;}
                case State::immune:     {return normal_speed;}
                default:                {return 0;          }
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
            int count = parent->count % (fps/speed);
            const double ratio = count*(static_cast<double>(speed)/fps);
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
                case State::immune: {
                    eye_color   = normal_eye;
                    body_color  = normal_body;
                    pupil_color = normal_pupil;
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
            // 繪製瞳孔 (根據 move 微調位置，靜止/等待時眼睛向下看)
            painter.setBrush(pupil_color);
            int pupil_radius = radius * 0.18;
            Pos eye_move = move;
            if (eye_move.x == 0 && eye_move.y == 0) {
                eye_move = Pos(0, 1); // 靜止/等待時眼睛向下看
            }
            QPoint pupil_offset(eye_move.x * 2.5, eye_move.y * 2.5);
            painter.drawEllipse(left_eye  + pupil_offset, pupil_radius, pupil_radius);
            painter.drawEllipse(right_eye + pupil_offset, pupil_radius, pupil_radius);
        }
    public:
        // 虛擬函數
        inline Ghost() noexcept = default;
        inline virtual ~Ghost() noexcept = default;
        inline virtual void update_direction() noexcept {};
        inline virtual void reset_with_spawn_pos(const Pos& new_spawn_pos) noexcept {
            spawn_pos = new_spawn_pos;
            position = spawn_pos;
            direction = Direc::none;
            status = State::normal;
            best_path.clear();
            immune_timer = 0;
            pending_u_turn = false;
            gate_walkble = true;
        }
        inline virtual void reset() noexcept {
            // 亂數種子:  x=9~11, y=3
            int rand_x = 8 + QRandomGenerator::global()->bounded(3);
            reset_with_spawn_pos(Pos(rand_x, 3));
        }
        // 成員函數
        inline Pos get_position() const noexcept { return position; }
        inline Pos get_spawn_pos() const noexcept { return spawn_pos; }
        inline State get_status() const noexcept { return status; }
        inline void set_status(State _status_) noexcept { status = _status_; }
        inline void clear_path() noexcept { best_path.clear(); }
        inline void reverse_direction() noexcept {
            if (status == State::eaten) return; // 被吃掉回家中的眼球不掉頭
            pending_u_turn = true;              // 走完當前格子後再轉向
        }
        inline bool collides_with(const MainWindow::PacMan& pacman) const noexcept {
            if (position == pacman.get_position()) return true;
            Pos ghost_next = position + get_move(direction);
            Pos pacman_next = pacman.get_position() + pacman.get_move(pacman.get_direction());
            if (pacman.get_position() == ghost_next && position == pacman_next) {
                return true;
            }
            return false;
        }
        inline bool can_pass_through(const Pos& pos) const noexcept {
            if (parent->get_tile(pos) == Tile::gate) {
                // 被吃的眼球 (State::eaten) 恆可穿過鬼門返回鬼屋
                return gate_walkble || (status == State::eaten);
            }
            if (parent->get_tile(pos) == Tile::wall) return false;
            return true;
        } 
        inline void pass_position(const Pos& pos) noexcept {
            position = pos;
            if (parent->get_tile(pos) == Tile::gate) {
                gate_walkble = false;
            }
        }
        inline void BFS_path(const Pos& target) noexcept {
            best_path.clear();
            Pos safe_target = target;
            if (position == target) return;
            if (safe_target.x < 0)              safe_target.x = 0;
            if (safe_target.x >= map_width)     safe_target.x = map_width - 1;
            if (safe_target.y < 0)              safe_target.y = 0;
            if (safe_target.y >= map_height)    safe_target.y = map_height - 1;
            // 佇列
            std::queue<Pos> queue;
            std::unordered_map<int, Pos> parent_map;
            // 轉為 key in unordered_map
            auto to_key = [](const Pos& p) -> int {
                return (p.x << 16) | p.y;
            };
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
                if (curr == safe_target) {
                    found = true;
                    break;
                }
                // 四個方向
                for (Direc direc : check_dirs) {
                    Pos next_pos = curr + get_move(direc);
                    // 邊界檢查
                    bool valid_x = (next_pos.x >= 0 && next_pos.x < map_width);
                    bool valid_y = (next_pos.y >= 0 && next_pos.y < map_height);
                    if (!valid_x || !valid_y) continue;
                    int next_key = to_key(next_pos);
                    // 尋找下一個目標
                    if (parent_map.find(next_key) != parent_map.end()) continue;
                    if (!can_pass_through(next_pos)) continue;
                    parent_map[next_key] = curr;
                    queue.push(next_pos);
                }
            }
            // 找不到路徑
            if (!found) return;
            // 反向追蹤回起點
            Pos trace = safe_target;
            while (!(trace == position)) {
                best_path.push_back(trace);
                trace = parent_map[to_key(trace)];
            }
            std::reverse(best_path.begin(), best_path.end());
        }
        inline void update_status() noexcept {
            if (status == State::eaten) return;
            if (status == State::immune) return;
            // 將主視窗的 GameState 轉為 Ghost::State
            uint8_t temp = (uint8_t)parent->state;
            status = static_cast<State>(temp);
        }
        inline void handle_eaten_direction() noexcept {
            if (status != State::eaten) return;
            BFS_path(spawn_pos);
            if (best_path.empty()) [[unlikely]] {
                // 成功回到出生點，進入免疫狀態並重新獲得出門權限
                status = State::immune;
                immune_timer = immune_duration;
                direction = Direc::none;
                gate_walkble = true;
                return;
            }
            // 沿著 best_path 的方向走
            Pos next_step = best_path[0];
            if (next_step.x < position.x)      direction = Direc::left;
            else if (next_step.x > position.x) direction = Direc::right;
            else if (next_step.y < position.y) direction = Direc::up;
            else if (next_step.y > position.y) direction = Direc::down;
        }
        // 繪製和更新
        inline void paint(QPainter& painter) const {
            // 若該鬼魂剛被吃掉且處於 0.5 秒得分定格狀態，暫不繪製該鬼魂 (避免雙眼與分數文字重疊)
            if (parent->is_frozen && parent->eaten_ghost_ptr == this) return;
            draw(painter);
        }
        inline void update() noexcept {
            update_status();
            // 免疫倒計時
            if (status == State::immune) {
                if (immune_timer > 0) {
                    --immune_timer;
                } else {
                    status = State::normal;
                    update_status();
                }
            }
            // 開始移動
            if (delay_timer == 0) {
                int speed = get_speed();
                int count = (parent->count)%(fps/speed);
                if (count == 0) [[unlikely]] {
                    // 完成循環
                    Pos move = get_move(direction);
                    Pos destination = position + move;
                    if (can_pass_through(destination)) {
                        pass_position(destination);
                    }
                    // 走完一格抵達格子中心時，執行延遲掉頭轉向
                    if (pending_u_turn) {
                        pending_u_turn = false;
                        if (status != State::eaten) {
                            Direc opp = Direc::none;
                            switch(direction) {
                                case Direc::left:  opp = Direc::right; break;
                                case Direc::right: opp = Direc::left;  break;
                                case Direc::up:    opp = Direc::down;  break;
                                case Direc::down:  opp = Direc::up;    break;
                                default:           opp = Direc::none;  break;
                            }
                            if (opp != Direc::none) {
                                Pos opp_dest = position + get_move(opp);
                                if (can_pass_through(opp_dest)) {
                                    direction = opp;
                                    clear_path();
                                }
                            }
                        }
                    }
                    // 更新方向
                    if (status == State::eaten) {
                        handle_eaten_direction();
                    }
                    if (status != State::eaten) {
                        update_direction();
                    }
                }
            } else { --delay_timer; }
        }
    };

    // 繼承: 紅鬼
    class Blinky : public Ghost {
    private:
        size_t scatter_index = 0;
        static inline const std::vector<Pos> scatter_waypoints = {
            Pos(17, 1), // Line 2, Column 18 (右上)
            Pos(17, 8), // Line 9, Column 18 (右下)
            Pos(13, 8), // Line 9, Column 14 (左下)
            Pos(13, 1)  // Line 2, Column 14 (左上)
        };
    public:
        inline Blinky() noexcept : Ghost() {}
        inline virtual ~Blinky() noexcept override = default;
        inline void init(MainWindow *_parent_) noexcept {
            Ghost::init(Pos(9, 1), _parent_, QColor(255, 0, 0), 0.0, false);
            scatter_index = 0;
        }
        inline void reset_with_spawn_pos(const Pos& new_spawn_pos) noexcept override {
            Ghost::reset_with_spawn_pos(new_spawn_pos);
            scatter_index = 0;
        }
        inline void reset() noexcept override {
            Ghost::reset();
            scatter_index = 0;
        }
        inline void update_direction() noexcept override {
            if (status == State::eaten) return;
            
            // 驚嚇/閃爍狀態：隨機逃跑（不追逐 Pac-Man）
            if (status == State::scared || status == State::flashing) {
                if (!best_path.empty() && position == best_path[0]) {
                    best_path.erase(best_path.begin());
                }
                if (best_path.empty()) {
                    Pos escape_target = Pos(0, 0);
                    do {
                        escape_target.x = QRandomGenerator::global()->bounded(static_cast<int>(map_width));
                        escape_target.y = QRandomGenerator::global()->bounded(static_cast<int>(map_height));
                    } while (!can_pass_through(escape_target));
                    
                    BFS_path(escape_target);
                }
                if (!best_path.empty()) {
                    Pos next_step = best_path[0];
                    if (next_step.x < position.x)      direction = Direc::left;
                    else if (next_step.x > position.x) direction = Direc::right;
                    else if (next_step.y < position.y) direction = Direc::up;
                    else if (next_step.y > position.y) direction = Direc::down;
                }
                return;
            }

            // 正常 / 免疫狀態：根據行為模式選擇目標點
            Pos target;
            if (parent->behavior_mode == MainWindow::BehaviorMode::scatter) {
                // 散開模式 (Scatter Mode)：繞右上角 C 字形巡邏
                target = scatter_waypoints[scatter_index];
                if (position == target) {
                    scatter_index = (scatter_index + 1) % scatter_waypoints.size();
                    target = scatter_waypoints[scatter_index];
                }
            } else {
                // 追逐模式 (Chase Mode)：目標為小精靈當前位置
                target = parent->pacman.get_position();
            }

            BFS_path(target);
            // 散開模式抵達節點時自動切換至下一巡邏點
            if (best_path.empty() && parent->behavior_mode == MainWindow::BehaviorMode::scatter) {
                scatter_index = (scatter_index + 1) % scatter_waypoints.size();
                target = scatter_waypoints[scatter_index];
                BFS_path(target);
            }

            if (!best_path.empty()) {
                Pos next_step = best_path[0];
                if (next_step.x < position.x)      direction = Direc::left;
                else if (next_step.x > position.x) direction = Direc::right;
                else if (next_step.y < position.y) direction = Direc::up;
                else if (next_step.y > position.y) direction = Direc::down;
            } else { direction = Direc::none; }
        }
    } blinky;

    // 繼承: 粉鬼
    class Pinky : public Ghost {
    private:
        size_t scatter_index = 0;
        static inline const std::vector<Pos> scatter_waypoints = {
            Pos(1, 1), // Line 2, Column 2 (左上)
            Pos(5, 1), // Line 2, Column 6 (右上)
            Pos(5, 8), // Line 9, Column 6 (右下)
            Pos(1, 8)  // Line 9, Column 2 (左下)
        };
    public:
        inline Pinky() noexcept : Ghost() {}
        inline virtual ~Pinky() noexcept override = default;
        inline void init(MainWindow *_parent_) noexcept {
            Ghost::init(Pos(9, 3), _parent_, QColor(255, 182, 193), 2.0, true);
            scatter_index = 0;
        }
        inline void reset_with_spawn_pos(const Pos& new_spawn_pos) noexcept override {
            Ghost::reset_with_spawn_pos(new_spawn_pos);
            scatter_index = 0;
            delay_timer = static_cast<int>(fps * 2.0); // 出生定格 2.0 秒出門
        }
        inline void reset() noexcept override {
            Ghost::reset();
            scatter_index = 0;
            delay_timer = static_cast<int>(fps * 2.0); // 出生定格 2.0 秒出門
        }
        inline void update_direction() noexcept override {
            if (status == State::eaten) return;

            // 驚嚇/閃爍狀態：隨機逃跑（不追逐 Pac-Man）
            if (status == State::scared || status == State::flashing) {
                if (!best_path.empty() && position == best_path[0]) {
                    best_path.erase(best_path.begin());
                }
                if (best_path.empty()) {
                    Pos escape_target = Pos(0, 0);
                    do {
                        escape_target.x = QRandomGenerator::global()->bounded(static_cast<int>(map_width));
                        escape_target.y = QRandomGenerator::global()->bounded(static_cast<int>(map_height));
                    } while (!can_pass_through(escape_target));
                    
                    BFS_path(escape_target);
                }
                if (!best_path.empty()) {
                    Pos next_step = best_path[0];
                    if (next_step.x < position.x)      direction = Direc::left;
                    else if (next_step.x > position.x) direction = Direc::right;
                    else if (next_step.y < position.y) direction = Direc::up;
                    else if (next_step.y > position.y) direction = Direc::down;
                }
                return;
            }

            // 正常 / 免疫狀態：根據行為模式選擇目標點
            Pos target;
            if (parent->behavior_mode == MainWindow::BehaviorMode::scatter) {
                // 散開模式 (Scatter Mode)：繞左上角 C 字形巡邏
                target = scatter_waypoints[scatter_index];
                if (position == target) {
                    scatter_index = (scatter_index + 1) % scatter_waypoints.size();
                    target = scatter_waypoints[scatter_index];
                }
            } else {
                // 追逐模式 (Chase Mode)：伏擊 Pac-Man 前方 4 格
                Pos pac_pos = parent->pacman.get_position();
                Direc pac_dir = parent->pacman.get_direction();
                Pos offset = Pos(0, 0);
                switch (pac_dir) {
                    case Direc::left:  offset = Pos(-4,  0); break;
                    case Direc::right: offset = Pos( 4,  0); break;
                    case Direc::up:    offset = Pos( 0, -4); break;
                    case Direc::down:  offset = Pos( 0,  4); break;
                    default:           offset = Pos( 0,  0); break;
                }
                target = pac_pos + offset;
            }

            BFS_path(target);
            // 散開抵達目標時自動切換至下一巡邏點
            if (best_path.empty() && parent->behavior_mode == MainWindow::BehaviorMode::scatter) {
                scatter_index = (scatter_index + 1) % scatter_waypoints.size();
                target = scatter_waypoints[scatter_index];
                BFS_path(target);
            }

            if (!best_path.empty()) {
                Pos next_step = best_path[0];
                if (next_step.x < position.x)      direction = Direc::left;
                else if (next_step.x > position.x) direction = Direc::right;
                else if (next_step.y < position.y) direction = Direc::up;
                else if (next_step.y > position.y) direction = Direc::down;
            } else { direction = Direc::none; }
        }
    } pinky;

    // 繼承: 青鬼
    class Inky : public Ghost {
    private:
        int steps_counter = 0;
        size_t scatter_index = 0;
        static inline const std::vector<Pos> scatter_waypoints = {
            Pos(17, 8),  // Line 9, Column 18 (右上)
            Pos(17, 14), // Line 15, Column 18 (右下)
            Pos(9, 14),  // Line 15, Column 10 (左下)
            Pos(9, 8)    // Line 9, Column 10 (左上)
        };
    public:
        inline Inky() noexcept : Ghost() {}
        inline virtual ~Inky() noexcept override = default;
        inline void init(MainWindow *_parent_) noexcept {
            Ghost::init(Pos(9, 3), _parent_, QColor(0, 255, 255), 4.0, true);
            steps_counter = 0;
            scatter_index = 0;
        }
        inline void reset_with_spawn_pos(const Pos& new_spawn_pos) noexcept override {
            Ghost::reset_with_spawn_pos(new_spawn_pos);
            steps_counter = 0;
            scatter_index = 0;
            delay_timer = static_cast<int>(fps * 4.0); // 延遲 4 秒出門
        }
        inline void reset() noexcept override {
            Ghost::reset();
            steps_counter = 0;
            scatter_index = 0;
            delay_timer = static_cast<int>(fps * 4.0); // 延遲 4 秒出門
        }
        inline void update_direction() noexcept override {
            if (status == State::eaten) return;

            // 驚嚇/閃爍狀態：隨機逃跑
            if (status == State::scared || status == State::flashing) {
                if (!best_path.empty() && position == best_path[0]) {
                    best_path.erase(best_path.begin());
                }
                if (best_path.empty()) {
                    Pos escape_target = Pos(0, 0);
                    do {
                        escape_target.x = QRandomGenerator::global()->bounded(static_cast<int>(map_width));
                        escape_target.y = QRandomGenerator::global()->bounded(static_cast<int>(map_height));
                    } while (!can_pass_through(escape_target));
                    
                    BFS_path(escape_target);
                }
                if (!best_path.empty()) {
                    Pos next_step = best_path[0];
                    if (next_step.x < position.x)      direction = Direc::left;
                    else if (next_step.x > position.x) direction = Direc::right;
                    else if (next_step.y < position.y) direction = Direc::up;
                    else if (next_step.y > position.y) direction = Direc::down;
                }
                return;
            }

            // 散開模式 (Scatter Mode)：繞右下角 C 字形巡邏
            if (parent->behavior_mode == MainWindow::BehaviorMode::scatter) {
                Pos target = scatter_waypoints[scatter_index];
                if (position == target) {
                    scatter_index = (scatter_index + 1) % scatter_waypoints.size();
                    target = scatter_waypoints[scatter_index];
                }
                BFS_path(target);
                if (best_path.empty()) {
                    scatter_index = (scatter_index + 1) % scatter_waypoints.size();
                    target = scatter_waypoints[scatter_index];
                    BFS_path(target);
                }
                if (!best_path.empty()) {
                    Pos next_step = best_path[0];
                    if (next_step.x < position.x)      direction = Direc::left;
                    else if (next_step.x > position.x) direction = Direc::right;
                    else if (next_step.y < position.y) direction = Direc::up;
                    else if (next_step.y > position.y) direction = Direc::down;
                } else { direction = Direc::none; }
                return;
            }

            // 追逐模式 (Chase Mode)：移動一段距離，隨機轉向，但是不會迴轉
            Pos forward_pos = position + get_move(direction);
            bool is_forward_blocked = !can_pass_through(forward_pos);

            if (steps_counter > 0 && !is_forward_blocked && direction != Direc::none) {
                --steps_counter;
                return;
            }

            Direc current = direction;
            Direc opposite = Direc::none;
            switch (current) {
                case Direc::left:  opposite = Direc::right; break;
                case Direc::right: opposite = Direc::left;  break;
                case Direc::up:    opposite = Direc::down;  break;
                case Direc::down:  opposite = Direc::up;    break;
                default:           opposite = Direc::none;  break;
            }

            std::vector<Direc> valid_dirs;
            Direc check_dirs[] = { Direc::left, Direc::right, Direc::up, Direc::down };

            for (Direc d : check_dirs) {
                if (d == opposite && current != Direc::none) continue;
                Pos next_p = position + get_move(d);
                if (can_pass_through(next_p)) {
                    valid_dirs.push_back(d);
                }
            }

            if (valid_dirs.empty() && opposite != Direc::none) {
                Pos opp_p = position + get_move(opposite);
                if (can_pass_through(opp_p)) {
                    valid_dirs.push_back(opposite);
                }
            }

            if (!valid_dirs.empty()) {
                int idx = QRandomGenerator::global()->bounded(static_cast<int>(valid_dirs.size()));
                direction = valid_dirs[idx];
                steps_counter = 2 + QRandomGenerator::global()->bounded(5);
            } else {
                direction = Direc::none;
            }
        }
    } inky;

    // 繼承: 橘鬼
    class Clyde : public Ghost {
    private:
        size_t scatter_index = 0;
        int mimic_role = 0;   // 0: Blinky (右上), 1: Pinky (左上), 2: Inky (右下)
        int mimic_timer = 0;  // 扮演該鬼魂的剩餘幀數

        // 另外三隻鬼魂的散開巡邏路線
        static inline const std::vector<Pos> blinky_waypoints = {
            Pos(17, 1), Pos(17, 8), Pos(13, 8), Pos(13, 1)  // 右上
        };
        static inline const std::vector<Pos> pinky_waypoints = {
            Pos(1, 1), Pos(5, 1), Pos(5, 8), Pos(1, 8)      // 左上
        };
        static inline const std::vector<Pos> inky_waypoints = {
            Pos(17, 8), Pos(17, 14), Pos(9, 14), Pos(9, 8)  // 右下
        };

        inline void pick_new_role() noexcept {
            mimic_role = QRandomGenerator::global()->bounded(3); // 隨機選擇 0, 1, 2
            mimic_timer = (3 + QRandomGenerator::global()->bounded(5)) * fps; // 隨機 3~7 秒
            scatter_index = 0;
            best_path.clear();
        }
    public:
        inline Clyde() noexcept : Ghost() {}
        inline virtual ~Clyde() noexcept override = default;
        inline void init(MainWindow *_parent_) noexcept {
            Ghost::init(Pos(10, 3), _parent_, QColor(255, 165, 0), 6.0, true);
            pick_new_role();
        }
        inline void reset_with_spawn_pos(const Pos& new_spawn_pos) noexcept override {
            Ghost::reset_with_spawn_pos(new_spawn_pos);
            delay_timer = static_cast<int>(fps * 6.0); // 延遲 6 秒出門
            pick_new_role();
        }
        inline void reset() noexcept override {
            Ghost::reset();
            delay_timer = static_cast<int>(fps * 6.0); // 延遲 6 秒出門
            pick_new_role();
        }
        inline void update_direction() noexcept override {
            if (status == State::eaten) return;

            // 驚嚇/閃爍狀態：隨機逃跑
            if (status == State::scared || status == State::flashing) {
                if (!best_path.empty() && position == best_path[0]) {
                    best_path.erase(best_path.begin());
                }
                if (best_path.empty()) {
                    Pos escape_target = Pos(0, 0);
                    do {
                        escape_target.x = QRandomGenerator::global()->bounded(static_cast<int>(map_width));
                        escape_target.y = QRandomGenerator::global()->bounded(static_cast<int>(map_height));
                    } while (!can_pass_through(escape_target));
                    
                    BFS_path(escape_target);
                }
                if (!best_path.empty()) {
                    Pos next_step = best_path[0];
                    if (next_step.x < position.x)      direction = Direc::left;
                    else if (next_step.x > position.x) direction = Direc::right;
                    else if (next_step.y < position.y) direction = Direc::up;
                    else if (next_step.y > position.y) direction = Direc::down;
                }
                return;
            }

            // 散開模式 (Scatter Mode)：隨機一段時間扮演 Blinky(右上)、Pinky(左上) 或 Inky(右下) 進行移動
            if (parent->behavior_mode == MainWindow::BehaviorMode::scatter) {
                if (mimic_timer > 0) {
                    --mimic_timer;
                } else {
                    pick_new_role();
                }

                // 取得當前扮演鬼魂的散開巡邏點
                const std::vector<Pos>& waypoints = (mimic_role == 0) ? blinky_waypoints :
                                                    (mimic_role == 1) ? pinky_waypoints : inky_waypoints;
                
                Pos target = waypoints[scatter_index % waypoints.size()];
                if (position == target) {
                    scatter_index = (scatter_index + 1) % waypoints.size();
                    target = waypoints[scatter_index];
                }
                BFS_path(target);
                if (best_path.empty()) {
                    scatter_index = (scatter_index + 1) % waypoints.size();
                    target = waypoints[scatter_index];
                    BFS_path(target);
                }

                if (!best_path.empty()) {
                    Pos next_step = best_path[0];
                    if (next_step.x < position.x)      direction = Direc::left;
                    else if (next_step.x > position.x) direction = Direc::right;
                    else if (next_step.y < position.y) direction = Direc::up;
                    else if (next_step.y > position.y) direction = Direc::down;
                } else { direction = Direc::none; }
                return;
            }

            // 追逐模式 (Chase Mode)：距離 >= 8 格時追 Pac-Man，距離 < 8 格時退回左下角
            Pos pac_pos = parent->pacman.get_position();
            int distance = std::abs(position.x - pac_pos.x) + std::abs(position.y - pac_pos.y);
            Pos target;
            if (distance >= 8) {
                target = pac_pos;
            } else {
                target = Pos(1, 8); // 左下角 retreat 點
            }

            BFS_path(target);
            if (!best_path.empty()) {
                Pos next_step = best_path[0];
                if (next_step.x < position.x)      direction = Direc::left;
                else if (next_step.x > position.x) direction = Direc::right;
                else if (next_step.y < position.y) direction = Direc::up;
                else if (next_step.y > position.y) direction = Direc::down;
            } else { direction = Direc::none; }
        }
    } clyde;

    // 吃掉鬼魂相關資訊
    Pos collision_pos = Pos(0, 0);
    Ghost* eaten_ghost_ptr = nullptr;
    // 鬼魂指標陣列 (紅鬼/粉鬼/青鬼/橘鬼)
    std::array<Ghost*, 4> ghosts = {
        &blinky,
        &pinky,
        &inky,
        &clyde
    };

    void trigger_ghosts_u_turn() noexcept {
        for (Ghost* ghost : ghosts) ghost->reverse_direction();
    }

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
        // 繪製分數、生命數與模式資訊
        painter.setPen(Qt::white);
        painter.setRenderHint(QPainter::Antialiasing);
        painter.setFont(QFont("Arial", 11, QFont::Bold));
        QString mode_str;
        if (behavior_timer < 0) {
            mode_str = "PERMANENT CHASE";
        } else {
            mode_str = (behavior_mode == BehaviorMode::scatter) ? "SCATTER" : "CHASE";
            int sec_left = (behavior_timer + fps - 1) / fps;
            mode_str += " (" + QString::number(sec_left) + "s)";
        }
        QString lives_str = "LIVES: " + QString::number(lives);
        QString score_text = "SCORE: " + QString::number(score) + "   " + lives_str + "   MODE: " + mode_str;
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
    void update_ghosts() noexcept {
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
            if (ghost->collides_with(pacman) && !was_eaten) [[unlikely]] {
                return ghost;
            }
        }
        return nullptr;
    }
    // 重置所有角色與遊戲狀態
    void reset_game_positions() noexcept {
        pacman.reset();
        for (Ghost* ghost : ghosts) {
            ghost->reset(); // 每個鬼魂獨立隨機抽選擇重生點 (允許重疊)
        }
        state = GameState::normal;             // 清除恐慌/驚嚇狀態，恢復正常
        pellet_timer = 0;                      // 清除藥丸剩餘時間
        ghosts_eaten_count = 0;                // 清除連吃鬼魂得分倍數
        behavior_mode = BehaviorMode::scatter; // 恢復初始散開模式
        behavior_timer = 7 * fps;              // 恢復散開模式 7 秒計時
        phase_index = 0;                       // 恢復階段 0
        is_waiting_start = true;               // 進入等待玩家按鍵狀態
        is_frozen = false;                     // 清除定格狀態
        eaten_ghost_ptr = nullptr;             // 清除被吃鬼魂指標
    }
    // 處理碰撞邏輯
    void handle_collision() noexcept {
        Ghost* ghost = collided_ghost();
        if (!ghost) return;

        bool is_ghost_immune = (ghost->get_status() == Ghost::State::immune);
        bool is_power_pellet_active = (state == GameState::chasing || state == GameState::flashing);

        // 若處於藥丸恐慌狀態且鬼魂「非免疫」，Pac-Man 吃掉鬼魂
        if (is_power_pellet_active && !is_ghost_immune) [[likely]] {
            ghost->set_status(Ghost::State::eaten);
            ghost->handle_eaten_direction(); // 立即計算尋路至出生點
            collision_pos = pacman.get_pixel_pos();
            score += (200 << ghosts_eaten_count++);
            eaten_ghost_ptr = ghost;
            freeze_timer = 0.5*fps;
            is_frozen = true;
        } else [[unlikely]] {
            // 正常狀態 或 鬼魂處於免疫狀態 -> Pac-Man 被鬼魂吃掉，扣減生命
            lives -= 1;
            if (lives > 0) {
                // 還有剩餘生命：重置小精靈與鬼魂位置，短暫定格 1 秒
                reset_game_positions();
                freeze_timer = 1.0 * fps;
                is_frozen = true;
            } else {
                // 沒生命了：Game Over
                QMessageBox::information(this, "Game Over", "You lose");
                this->close();
            }
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
        QString gotten_score = QString::number(200 << (ghosts_eaten_count-1));
        QRect text_rect(point.x()-offset, point.y()-offset, rect_size, rect_size);
        painter.drawText(text_rect, Qt::AlignCenter, gotten_score);
    }
};

#endif // MAINWINDOW_H
