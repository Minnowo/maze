
#ifndef WORLD_H
#define WORLD_H

#include "GLFW/glfw3.h"
#include "gl2d/gl2d.h"
#include <bitset>

#define NEWCOLOR(r, g, b) (gl2d::Color4f{(float)(r) / 255.0f, (float)(g) / 255.0f, (float)(b) / 255.0f, 1})
#define ColorBG NEWCOLOR(0x1f, 0x1f, 0x1f)
#define ColorWall NEWCOLOR(0xd4, 0xd4, 0xd4)
#define ColorPlayer NEWCOLOR(0xFf, 0xc1, 0xff)
#define ColorPath NEWCOLOR(0x81, 0xcf, 0xff)
#define ColorSearch NEWCOLOR(0xFF, 0x80, 0x80)

typedef enum { NORTH = 0, SOUTH, EAST, WEST } Direction;

Direction opposite_direction(Direction d);

struct Cell {

        gl2d::Color4f color;
        bool          walls[4];
        bool          visited;
        int           distance;

        void           addWall(Direction d);
        void           removeWall(Direction d);
        bool           wallAt(Direction d);
        bool           wallOpposite(Direction d);
        std::bitset<4> wallDirections(Direction d);
};

struct Player {

        int   x;
        int   y;
        float lastmoved;
        float movecooldown;
};

struct Map {

        glm::i32vec2 finishPos;
        Cell*        cells;

        int percentLessWalls;
        int width;
        int height;

        bool   canMove(int x, int y, Direction d);
        bool   canMove(int x, int y);
        Cell*  at(int x, int y);
        int    rawIndex(int x, int y);
        void   buildRandomMaze();
        size_t length();
};

struct World {

    public:
        gl2d::Renderer2D r2d;
        GLFWwindow*      glwin;

        Map    map;
        Player player;

        int    screenWidth;
        int    screenHeight;
        double lastTime;
        double deltaTime;

        int cellSize;
        int wallWidth;

        void initGLFW();
        void initGL2D();
        void updateTime();
        void beginFrame();
        void endFrame();

        void renderCell(Cell* cell, int x, int y);
        void renderMap();
        void renderPlayer();
};

#endif
