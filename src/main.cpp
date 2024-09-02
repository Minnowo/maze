

#include <bitset>
#include <stack>

#include <GLFW/glfw3.h>
#include <gl2d/gl2d.h>
#include <glad/glad.h>

#include "glm/fwd.hpp"
#include "world.hpp"

constexpr size_t M_WIDTH = 30;
constexpr size_t M_HEIGHT = 30;
constexpr int TILE_SIZE = 15;
constexpr int WALL_WIDTH = 2;


void show_path(Map& map, Player& player, std::stack<glm::i32vec2>& history) {

    if (player.lastmoved < player.movecooldown) {
        return;
    }

    if (history.empty()) {
        return;
    }

    glm::i32vec2 pos = history.top();

    history.pop();

    Cell* cell = map.at(pos.x, pos.y);

    cell->color = ColorPath;
}

void solve_maze(Map& map, Player& player, std::stack<glm::i32vec2>& history, bool& isSolved) {

    if (player.lastmoved < player.movecooldown) {
        return;
    }

    int x = player.x;
    int y = player.y;

    if (isSolved || map.finishPos == map.rawIndex(x, y)) {

        isSolved = true;

        return;
    }

    Cell* cell = map.at(x, y);

    cell->visited = true;
    cell->color = ColorSearch;

    std::bitset<4> directions = {0b1111};

    if (cell->wallAt(Direction::WEST) || x <= 0) {
        directions.reset(Direction::WEST);
    }

    if (cell->wallAt(Direction::EAST) || x >= map.width - 1) {
        directions.reset(Direction::EAST);
    }

    if (cell->wallAt(Direction::NORTH) || y <= 0) {
        directions.reset(Direction::NORTH);
    }

    if (cell->wallAt(Direction::SOUTH) || y >= map.height - 1) {
        directions.reset(Direction::SOUTH);
    }

    for (;;) {

        if (directions.count() == 0) {

            if(history.empty())
                return;

            glm::i32vec2 pos = history.top();

            history.pop();

            player.x = pos.x;
            player.y = pos.y;

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

        if (cell->wallAt(move_to)) {

            directions.reset(move_to);

            continue;
        }

        Cell* newCell = map.at(newx, newy);

        if (newCell->visited || newCell->wallOpposite(move_to)) {

            directions.reset(move_to);

            continue;
        }

        directions.reset(move_to);

        player.x = newx;
        player.y = newy;

        history.push({x, y});

        return;
    }
}

void do_player_move(GLFWwindow* window, Map& map, Player& player) {

    if (player.lastmoved < player.movecooldown) {
        return;
    }

    if (glfwGetKey(window, GLFW_KEY_UP)) {

        if (map.canMove(player.x, player.y, Direction::NORTH)) {

            player.y--;
            player.lastmoved = 0;

            if (player.y < 0) {
                player.y = 0;
            }
            return;
        }
    }

    if (glfwGetKey(window, GLFW_KEY_DOWN)) {

        if (map.canMove(player.x, player.y, Direction::SOUTH)) {

            player.y++;
            player.lastmoved = 0;

            if (player.y >= map.height) {
                player.y = map.height - 1;
            }
            return;
        }
    }

    if (glfwGetKey(window, GLFW_KEY_LEFT)) {

        if (map.canMove(player.x, player.y, Direction::WEST)) {

            player.x--;
            player.lastmoved = 0;

            if (player.x < 0) {
                player.x = 0;
            }
            return;
        }
    }

    if (glfwGetKey(window, GLFW_KEY_RIGHT)) {

        if (map.canMove(player.x, player.y, Direction::EAST)) {

            player.x++;
            player.lastmoved = 0;

            if (player.x >= map.width) {
                player.x = map.width - 1;
            }
            return;
        }
    }
}

int main(void) {

    srand(time(NULL));

    World world;
    Cell cells[M_WIDTH * M_HEIGHT] = {};

    world.player = {
        .x = 0,
        .y = 0,
        .movecooldown = 0.1
    };

    world.map = { 
        .width = M_WIDTH,
        .height = M_HEIGHT,
        .cells = cells
    };
    world.screenWidth = 1024;
    world.screenHeight = 1024;
    world.cellSize = 25;
    world.wallWidth = 4;
    
    world.map.buildRandomMaze();

    bool isSolved = false;
    std::stack<glm::i32vec2> visitHistory;

    world.initGLFW();
    world.initGL2D();

    while (!glfwWindowShouldClose(world.glwin)) {

        world.beginFrame();

        world.renderMap();
        world.renderPlayer();

        world.endFrame();

        world.updateTime();

        if (world.screenHeight > world.screenWidth) {
            world.cellSize = world.screenWidth / world.map.width;
        } else {
            world.cellSize = world.screenHeight / world.map.width;
        }

        world.player.lastmoved += world.deltaTime;

        Cell* cell = world.map.at(world.player.x, world.player.y);

        cell->color = Colors_Gray;

        do_player_move(world.glwin, world.map, world.player);

        if (glfwGetKey(world.glwin, GLFW_KEY_SPACE)) {
            if (isSolved) {
                show_path(world.map, world.player, visitHistory);
            } else {
                solve_maze(world.map, world.player, visitHistory, isSolved);
            }
        }

        if (glfwGetKey(world.glwin, GLFW_KEY_R)) {

            isSolved = false;

            world.map.buildRandomMaze();

            while (!visitHistory.empty())
                visitHistory.pop();
        }
    }

    glfwDestroyWindow(world.glwin);

    glfwTerminate();

    return 0;
}
