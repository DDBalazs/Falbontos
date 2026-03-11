#include "graphics.hpp"
#include <vector>
#include <cmath>
#include <cstdlib>

using namespace genv;

// Képernyő méretei
const int WIDTH = 800;
const int HEIGHT = 600;

// Téglák struktúrája
struct Brick {
    int x, y, w, h;
    int r, g, b;
    bool active;
};

// Ütő struktúrája
struct Paddle {
    int x, y, w, h;
};

// Labda struktúrája
struct Ball {
    double x, y;
    double vx, vy;
    int size;
};

int main()
{
    gout.open(WIDTH, HEIGHT);

    // Állítsunk be egy timert a folyamatos játékmenethez (~50 FPS)
    gin.timer(20);

    // Objektumok inicializálása
    Paddle paddle = {WIDTH / 2 - 60, HEIGHT - 50, 120, 15};

    double ball_speed = 8.0;
    Ball ball = {WIDTH / 2.0 - 5.0, HEIGHT / 2.0, 0.0, ball_speed, 10};

    // Téglák generálása a kép alapján (szivárvány színek)
    std::vector<Brick> bricks;
    int rows = 6;
    int cols = 12;
    int brick_w = WIDTH / cols;
    int brick_h = 30;

    // Színek soronként (Piros, Narancs, Sárga, Zöld, Kék, Lila)
    int colors[6][3] = {
        {255, 90, 90},
        {255, 160, 0},
        {255, 210, 0},
        {20, 200, 80},
        {80, 160, 255},
        {180, 100, 255}
    };

    for(int i = 0; i < rows; ++i) {
        for(int j = 0; j < cols; ++j) {
            bricks.push_back({
                j * brick_w,
                50 + i * brick_h,
                brick_w,
                brick_h,
                colors[i][0], colors[i][1], colors[i][2],
                true
            });
        }
    }

    event ev;
    while(gin >> ev)
    {
        // Kilépés ESC-re
        if (ev.type == ev_key && ev.keycode == key_escape) {
            break;
        }

        // Ütő irányítása egérrel (vízszintes mozgás)
        if (ev.type == ev_mouse) {
            paddle.x = ev.pos_x - paddle.w / 2;

            // Pályán tartás
            if (paddle.x < 0) paddle.x = 0;
            if (paddle.x > WIDTH - paddle.w) paddle.x = WIDTH - paddle.w;
        }

        // Játék logikájának frissítése a timerre
        if (ev.type == ev_timer) {

            // 1. Labda mozgatása
            ball.x += ball.vx;
            ball.y += ball.vy;

            // 2. Fal ütközések
            if (ball.x <= 0) { ball.x = 0; ball.vx *= -1; }
            if (ball.x + ball.size >= WIDTH) { ball.x = WIDTH - ball.size; ball.vx *= -1; }
            if (ball.y <= 0) { ball.y = 0; ball.vy *= -1; }

            // Ha leesik a labda (reset)
            if (ball.y >= HEIGHT) {
                ball.x = WIDTH / 2.0 - ball.size / 2.0;
                ball.y = HEIGHT / 2.0;
                ball.vx = 0;
                ball.vy = ball_speed;
            }

            // 3. Ütő ütközés (Speciális pattogás a kritérium szerint!)
            if (ball.vy > 0 &&
                ball.y + ball.size >= paddle.y && ball.y <= paddle.y + paddle.h &&
                ball.x + ball.size >= paddle.x && ball.x <= paddle.x + paddle.w)
            {
                // Kiszámoljuk, hogy az ütő közepéhez képest hol történt a találat
                double hitPoint = (ball.x + ball.size / 2.0) - (paddle.x + paddle.w / 2.0);

                // Normalizáljuk az értéket -1.0 (bal szél) és 1.0 (jobb szél) közé
                double normalizedHit = hitPoint / (paddle.w / 2.0);

                // Kicsit limitáljuk, hogy ne lehessen teljesen vízszintes a pattanás
                if (normalizedHit > 0.85) normalizedHit = 0.85;
                if (normalizedHit < -0.85) normalizedHit = -0.85;

                // Az X és Y sebesség beállítása az új szög alapján
                ball.vx = normalizedHit * ball_speed;
                // Pitagorasz-tétellel tartjuk a fix sebességet: vx^2 + vy^2 = speed^2
                ball.vy = -std::sqrt(ball_speed * ball_speed - ball.vx * ball.vx);

                // Hogy ne ragadjon bele az ütőbe
                ball.y = paddle.y - ball.size;
            }

            // 4. Tégla ütközések
            for(auto& b : bricks) {
                if (b.active) {
                    // Egyszerű négyszög alapú (AABB) ütközésvizsgálat
                    if (ball.x + ball.size > b.x && ball.x < b.x + b.w &&
                        ball.y + ball.size > b.y && ball.y < b.y + b.h)
                    {
                        b.active = false; // Tégla eltüntetése

                        // Kiszámoljuk, hogy melyik irányból pattanjon vissza (egyszerűsített geometria)
                        double ball_cx = ball.x + ball.size / 2.0;
                        double ball_cy = ball.y + ball.size / 2.0;
                        double brick_cx = b.x + b.w / 2.0;
                        double brick_cy = b.y + b.h / 2.0;

                        double dx = (ball_cx - brick_cx) / b.w;
                        double dy = (ball_cy - brick_cy) / b.h;

                        if (std::abs(dx) > std::abs(dy)) {
                            ball.vx *= -1; // Oldalról találta el
                        } else {
                            ball.vy *= -1; // Alulról/felülről találta el
                        }
                        break; // Egy frame-ben csak egy téglát törjünk
                    }
                }
            }

            // ---- RAJZOLÁS ----

            // Háttér törlése (sötétszürke, mint a képen)
            gout << move_to(0, 0) << color(30, 35, 40) << box(WIDTH, HEIGHT);

            // Téglák rajzolása
            for(const auto& b : bricks) {
                if (b.active) {
                    // Fekete keret
                    gout << move_to(b.x, b.y) << color(30, 35, 40) << box(b.w, b.h);
                    // Belső szín
                    gout << move_to(b.x + 1, b.y + 1) << color(b.r, b.g, b.b) << box(b.w - 2, b.h - 2);
                }
            }

            // Ütő rajzolása (fehér)
            gout << move_to(paddle.x, paddle.y) << color(255, 255, 255) << box(paddle.w, paddle.h);

            // Labda rajzolása (fehér négyzet, mivel kör rajzolás nem mindig alapértelmezett a genv-ben)
            gout << move_to(ball.x, ball.y) << color(255, 255, 255) << box(ball.size, ball.size);

            gout << refresh;
        }
    }
    return 0;
}