
#include <bitset>
#include <iostream>
#include "openglErrorReporting.h"
#include "world.hpp"

Direction opposite_direction(Direction d) {
    switch(d){
    case NORTH:
        return Direction::SOUTH;
    case SOUTH:
        return Direction::NORTH;
    case EAST:
        return Direction::WEST;
    case WEST:
        return Direction::EAST;
    }
}

void Cell::addWall(Direction d) {
    walls[d] = 1;
}
void Cell::removeWall(Direction d) {
    walls[d] = 0;
}
bool Cell::wallAt(Direction d) {
    return walls[d];
}
bool Cell::wallOpposite(Direction d) {
    return wallAt(opposite_direction(d));
}

bool Map::canMove(int x, int y, Direction d) {

    if (this->at(x, y)->wallAt(d)) {
        return false;
    }

    switch (d) {
    case NORTH:
        return !this->at(x, y - 1)->wallOpposite(d);
    case SOUTH:
        return !this->at(x, y + 1)->wallOpposite(d);
    case EAST:
        return !this->at(x + 1, y)->wallOpposite(d);
    case WEST:
        return !this->at(x - 1, y)->wallOpposite(d);
    }
}

int Map::rawIndex(int x, int y) {
    return this->width * y + x;
}

Cell* Map::at(int x, int y) {

    assert(x > 0);
    assert(y > 0);

    assert(x < this.width);
    assert(y < this.height);

    return this->cells + this->width * y + x;
}

size_t Map::length() {
    return this->width * this->height;
}

static void build_maze_recur(Map* map, int x, int y) {

    Cell* cell = map->at(x, y);

    cell->visited = true;

    std::bitset<4> directions = {0b1111};

    if (x <= 0) {
        directions.reset(Direction::WEST);
    }

    if (x >= map->width - 1) {
        directions.reset(Direction::EAST);
    }

    if (y <= 0) {
        directions.reset(Direction::NORTH);
    }

    if (y >= map->height - 1) {
        directions.reset(Direction::SOUTH);
    }

    for (;;) {

        if (directions.count() == 0) {
            return;
        }

        Direction move_to = Direction(rand() % 4);

        if (!directions.test(move_to)) {
            continue;
        }

        int newx = x;
        int newy = y;

        switch (move_to) {
        case NORTH:
            newy--;
            break;
        case SOUTH:
            newy++;
            break;
        case EAST:
            newx++;
            break;
        case WEST:
            newx--;
            break;
        }

        if (!cell->wallAt(move_to)) {

            directions.reset(move_to);

            continue;
        }

        Cell* newCell = map->at(newx, newy);

        if (newCell->visited || !newCell->wallOpposite(move_to)) {

            directions.reset(move_to);

            continue;
        }

        directions.reset(move_to);

        cell->removeWall(move_to);

        newCell->removeWall(opposite_direction(move_to));

        build_maze_recur(map, newx, newy);
    }
}

void Map::buildRandomMaze() {

    size_t len = this->length();

    for(int i = 0; i < len; i++) {

        this->cells[i] = {.color = ColorBG, .walls = {1, 1, 1, 1}, .visited = false};
    }

    int start_x = rand() % this->width;
    int start_y = rand() % this->height;

    build_maze_recur(this, start_x, start_y);

    for (int i = 0; i < this->length(); i++) {
        this->cells[i].visited = false;
    }

    this->at(start_x, start_y)->color = Colors_Green;
    this->finishPos = this->rawIndex(start_x, start_y);
}




static void error_callback(int error, const char* description) {
    std::cout << "Error: " << error << " " << description << "\n";
}

void World::initGLFW(){

    glfwSetErrorCallback(error_callback);

    if (!glfwInit())
        exit(EXIT_FAILURE);

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);

    GLFWwindow* window = glfwCreateWindow(this->screenWidth, this->screenHeight, "Simple example", NULL, NULL);

    if (!window) {
        glfwTerminate();
        exit(EXIT_FAILURE);
    }

    this->glwin = window;

    glfwMakeContextCurrent(window);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        glfwTerminate();
        exit(EXIT_FAILURE);
    }

    enableReportGlErrors();
}

void World::initGL2D() {

    gl2d::init();

    this->r2d.create();
}


void World::updateTime(){

    double currentTime = glfwGetTime();

    this->deltaTime = currentTime - lastTime;
    this->lastTime  = currentTime;
}

void World::beginFrame() {

    glfwGetFramebufferSize(this->glwin, &this->screenWidth, &this->screenHeight);

    glViewport(0, 0, this->screenWidth, this->screenHeight);

    glClear(GL_COLOR_BUFFER_BIT);

    this->r2d.updateWindowMetrics(this->screenWidth, this->screenHeight);
}

void World::endFrame() {

    this->r2d.flush();

    glfwSwapBuffers(this->glwin);

    glfwPollEvents();
}

void World::renderCell(Cell* cell, int x, int y) {

    x *= this->cellSize;
    y *= this->cellSize;

    r2d.renderRectangle({x, y, this->cellSize, this->cellSize}, cell->color);

    if (cell->walls[Direction::NORTH]) {
        r2d.renderRectangle({x, y, this->cellSize, this->wallWidth}, ColorWall);
    }

    if (cell->walls[Direction::SOUTH]) {
        r2d.renderRectangle({x, y + this->cellSize - this->wallWidth, this->cellSize, this->wallWidth}, ColorWall);
    }

    if (cell->walls[Direction::WEST]) {
        r2d.renderRectangle({x, y, this->wallWidth, this->cellSize}, ColorWall);
    }

    if (cell->walls[Direction::EAST]) {
        r2d.renderRectangle({x + this->cellSize - this->wallWidth, y, this->wallWidth, this->cellSize}, ColorWall);
    }
}

void World::renderMap(){

    for (int y = 0; y < map.height; y++) {

        for (int x = 0; x < map.width; x++) {

            int i = y * map.width + x;

            renderCell(map.cells + i, x, y);
        }
    }
}

void World::renderPlayer(){

    r2d.renderRectangle({player.x * this->cellSize, player.y * this->cellSize, this->cellSize, this->cellSize}, ColorPlayer);
}
