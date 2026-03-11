#include "graphics.hpp"
#include <vector>
#include <cmath>

using namespace genv;

const int width = 800;
const int height = 600;

struct Brick {
    int x,y,size_x,size_y;
    int r,g,b;
    bool active;
};

struct Paddle {
    int x,y,size_x,size_y;
};
struct Ball {
    double x,y,size_x,size_y;
    double speed;
    int size;
};

int main()
{
    gout.open(width,height);
    gout << font("LiberationSans-Regular.ttf",20);
    gout << text("hello world");
    gout << refresh;
    event ev;
    while(gin >> ev)
    {

    }
    return 0;
}
