

#include <stack>
#include <queue>

#include <GLFW/glfw3.h>
#include <gl2d/gl2d.h>
#include <glad/glad.h>
#include "glm/fwd.hpp"

#include "solvers/solvers.hpp"
#include "world.hpp"

constexpr size_t M_WIDTH = 30;
constexpr size_t M_HEIGHT = 30;
constexpr int TILE_SIZE = 15;
constexpr int WALL_WIDTH = 2;


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
        // .movecooldown = 0.5
    };

    bool a = 0;
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
    SolveStrat strategy = SolveStrat::FLOODFILL;
    std::stack<glm::i32vec2> visitHistory;
    std::queue<glm::i32vec2> floodnext;

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

        do_player_move(world.glwin, world.map, world.player);

        if (glfwGetKey(world.glwin, GLFW_KEY_SPACE)) {

            switch (strategy) {

            case DFS:
                if (isSolved) {
                    dfs_show_path(world.map, world.player, visitHistory);
                } else {
                    dfs_solve_maze(world.map, world.player, visitHistory, isSolved);
                }
                break;

            case FLOODFILL:
                if (isSolved) {
                    floodfill_show_path(world.map, world.player);
                } else {
                    floodfill_solve_maze(world.map, world.player, floodnext, isSolved);
                }
                break;
            }
        }

        if (glfwGetKey(world.glwin, GLFW_KEY_R)) {

            isSolved = false;

            world.map.buildRandomMaze();

            while (!visitHistory.empty())
                visitHistory.pop();

            while (!floodnext.empty())
                floodnext.pop();
        }
    }

    glfwDestroyWindow(world.glwin);

    glfwTerminate();

    return 0;
}
