#include "graphics.hpp"
#include <vector>
#include <cmath>
#include <string>

using namespace genv;
using namespace std;

const int width = 800;
const int height = 600;

enum GameState {MENU, GAME, GAME_OVER}; //jatekallapotok

struct Ball {
    Ball(double _speed, int _size) : speed(_speed), size(_size) {
        basic();
    }
    void basic() {
        x = width / 2 - size / 2;
        y = height / 2;
        vx = 0;
        vy = speed;
    }
    void draw() {
        gout << move_to(x, y) << color(255, 255, 255) << box(size, size);
    }
    bool moveing() {
        x += vx;
        y += vy;

        if (x <= 0) { //visszapattan a falról
            x = 0;
            vx = vx * -1;
        }
        if (x + size >= width) {
            x = width - size;
            vx = vx * -1;
        }
        if (y <= 0) {
            y = 0;
            vy = vy * -1;
        }

        if (y >= height) {
            return true;
        }

        return false;
    }

    void paddleHit(int px, int py, int p_width, int p_height) { //nekimegye az utonek
        if (vy > 0 && y + size >= py && y <= py + p_height && x + size >= px && x <= px + p_width) {
            double hitPoint = (x + size / 2) - (px + p_width / 2);
            double normalHit = hitPoint / (p_width / 2);

            if (normalHit > 0.85) {
                normalHit = 0.85;
            }
            if (normalHit < -0.85) {
                normalHit = -0.85;
            }

            vx = normalHit * speed;
            vy = -sqrt(speed * speed - vx * vx);
            y = py - size;
        }
    }
    bool brickHit(int bx, int by, int b_width, int b_height){
        if (x+size > bx && x < bx + b_width && y + size > by && y < by + b_height){
            double ball_cx = x + size / 2;
            double ball_cy = y + size / 2;
            double brick_cx = bx + b_width / 2;
            double brick_cy = by + b_height / 2;

            double dx = (ball_cx - brick_cx) / b_width;
            double dy = (ball_cy - brick_cy) / b_height;

            if (abs(dx) > abs(dy)) {
                vx = vx * -1;
            }
            else {
                vy = vy * -1;
            }
            return true; //eltalalt
        }
        return false;
    }

private:
    double x, y;
    double vx, vy;
    double speed;
    int size;
};

struct Paddle {

    Paddle(int x, int y, int size_x, int size_y):
        x(x), y(y), size_x(size_x), size_y(size_y) {}

    void draw() {
        gout << move_to(x, y) << color(255, 255, 255) << box(size_x, size_y);
    }

    void moveing(int mouse_x) {
        x = mouse_x - size_x / 2;
        if (x < 0) {
            x = 0;
        }
        if (x + size_x > width) {
            x = width - size_x;
        }
    }
    void ballHandle(Ball& ball) { //adatoroklodes
        ball.paddleHit(x, y, size_x, size_y);
    }

private:
    int x, y, size_x, size_y;
};
struct Brick {
    Brick(int x, int y, int size_x, int size_y, int r, int g, int b):
    x(x), y(y), size_x(size_x), size_y(size_y), r(r), g(g), b(b), active(true) {}

    void draw() {
        if (active) {
            gout << move_to(x, y) << color(30, 35, 40) << box(size_x, size_y);
            gout << move_to(x+1, y + 1) << color(r,g,b) << box(size_x - 2, size_y -2);
        }
    }

    void ballHandle(Ball& ball) {
        if (active) {
            if (ball.brickHit(x,y,size_x,size_y)) {
                active = false;
            }
        }
    }

private:
    int x, y, size_x, size_y;
    int r, g, b;
    bool active;
};

void midPrint(int y, string s, int r, int g, int b) {
    gout << color(r, g, b);
    int text_width = gout.twidth(s);
    gout << move_to(width / 2 - text_width / 2, y) << text(s);
}

int main()
{
    gout.open(width,height);
    gout << font("LiberationSans-Regular.ttf",30);
    gin.timer(20);

    Paddle paddle(width / 2 - 60, height - 50, 120, 15);
    Ball ball(8, 10);
    vector<Brick*> bricks;

    GameState state = MENU; //alapkepernyo
    int lives = 3;

    int rows = 6;
    int cols = 12;
    int brick_w = width / cols;
    int brick_h = 30;
    int colors[6][3] = {{255, 90, 90}, {255, 160, 0}, {255, 210, 0}, {20, 200, 80}, {80, 160, 255}, {180, 100,255}};

    auto mapGen = [&]() {
        for (Brick* b : bricks) {
            delete b;
        }
        bricks.clear();
        for (int i = 0; i < rows; i++) {
            for (int j = 0; j < cols; j++) {
                bricks.push_back(new Brick(j *brick_w, 50 + i * brick_h, brick_w, brick_h, colors[i][0], colors[i][1], colors[i][2]));
            }
        }
    };
    mapGen();

    event ev;
    while(gin >> ev)
    {
        gout << move_to(0,0) << color(30, 35, 40) << box(width, height);
        if (ev.type == ev_key && ev.keycode == key_escape) {
            break;
        }

        if (state == MENU) {
            midPrint(height / 2 - 40, "FALBONTÓ", 255,255,255);
            midPrint(height / 2 + 20, "Kattints a kezdéshez!", 150, 150, 150);

            if (ev.type == ev_mouse && ev.button == btn_left) {
                state = GAME;
                lives = 3;
                ball.basic();
                mapGen();
            }
        }
        else if (state == GAME) {
            if (ev.type == ev_mouse) {
                paddle.moveing(ev.pos_x);
            }

            if (ev.type == ev_timer) {
                if (ball.moveing()) {
                    lives--;

                    if (lives <= 0) {
                        state = GAME_OVER;
                    } else {
                        ball.basic();
                    }

                }

                paddle.ballHandle(ball);
                for (Brick* b : bricks) {
                    b->ballHandle(ball); //tégla elkezdi kezelnu a labdát
                }
            }

            for (Brick* b : bricks) {
                b->draw();
            }
            paddle.draw();
            ball.draw();

            gout << font("LiberationSans_Regular.ttf", 20);
            gout << move_to(10, 10) << color(255, 255, 255) << text("Életek: "+ to_string(lives));
            gout << font("LiberationSans_Regular.ttf", 30);
        }
        else if (state == GAME_OVER) {
            midPrint(height / 2 -40, "GAME OVER", 255, 50, 50);
            midPrint(height / 2 + 20, "Kattints a főmenűhöz!", 255, 255, 255);

            if (ev.type == ev_mouse && ev.button == btn_left) {
                state = MENU;
            }
        }
        gout << refresh;
    }
    for (Brick* b : bricks) {
        delete b;
    }
    return 0;
}
