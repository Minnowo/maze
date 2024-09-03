

#include <iostream>
#include <ostream>
#include <stack>
#include <queue>

#include <GLFW/glfw3.h>
#include <gl2d/gl2d.h>
#include <glad/glad.h>
#include "glm/fwd.hpp"

#include "solvers/solvers.hpp"
#include "world.hpp"

constexpr size_t M_WIDTH = 12;
constexpr size_t M_HEIGHT = 12;

#define DIE(fmt, ...)                                                                                                  \
    do {                                                                                                               \
        fprintf(stderr, (fmt), ##__VA_ARGS__);                                                                         \
        putc('\n', stderr);                                                                                            \
        fflush(stderr);                                                                                                \
        exit(1);                                                                                                       \
    } while (0)

#define DIE_IF_NULL(i, fmt, ...)                                                                                       \
    do {                                                                                                               \
        if ((i) == NULL) {                                                                                             \
            DIE((fmt), ##__VA_ARGS__);                                                                                 \
        }                                                                                                              \
    } while (0)

struct Args {
    int width;
    int height;
    int percentLessWalls;
};

std::string pad_left(std::string const& str, size_t s)
{
    if ( str.size() < s )
        return std::string(s-str.size(), ' ') + str;
    else
        return str;
}

void do_player_move(GLFWwindow* window, Map& map, Player& player) {

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



int handle_flags( Args& args, const char* flag_str, const char* flag_value) {

    size_t flen = strlen(flag_str);

    if (flen < 2 || flag_str[0] != '-')
        return 0;

    for (size_t i = 1; i < flen; ++i)

        switch (flag_str[i]) {

            case '-':

                if (strcasecmp(flag_str + i, "-width") == 0) {

                    DIE_IF_NULL(flag_value, "--width requires a width value > 0");

                    args.width = atoi(flag_value);

                    if (args.width == 0)
                        DIE("--width requires a width value > 0");

                    return 1;
                }

                if (strcasecmp(flag_str + i, "-height") == 0) {

                    DIE_IF_NULL(flag_value, "--height requires a height value > 0");

                    args.height = atoi(flag_value);

                    if (args.height == 0)
                        DIE("--height requires a height value > 0");
                }

                return 0;

        case 'l':

            DIE_IF_NULL(flag_value, "less-walls requires a number from 0-100");

            args.percentLessWalls = atoi(flag_value);

            if (args.percentLessWalls < 0 || args.percentLessWalls > 100)
                DIE("less-walls requires a number from 0-100");

            return 1;
        case 'w':

            DIE_IF_NULL(flag_value, "--width requires a width value > 0");

            args.width = atoi(flag_value);

            if (args.width == 0)
                DIE("--width requires a width value > 0");

            return 1;

        case 'h':

            DIE_IF_NULL(flag_value, "--height requires a height value > 0");

            args.height = atoi(flag_value);

            if (args.height == 0)
                DIE("--height requires a height value > 0");

            return 1;

        }

    return 0;
}

void handle_start_args(Args& config, int argc, char* argv[]) {

    for (int i = 1; i < argc; i++) {

        if (i + 1 < argc) {
            i += handle_flags(config, argv[i], argv[i + 1]);
        } else {
            i += handle_flags(config, argv[i], NULL);
        }
    }
}

int main(int argc, char* argv[]) {

    Args args = {0, 0, 0};

    handle_start_args(args, argc, argv);

    if(args.width <= 1) 
        args.width = M_WIDTH;

    if(args.height <= 1) 
        args.height = M_HEIGHT;

    srand(time(NULL));

    World world;
    Cell  cells[args.width * args.height];

    world.player = {
        .x = 0,
        .y = 0,
        .movecooldown = 0.1
    };

    world.map = {
        .cells = cells,
        .percentLessWalls = args.percentLessWalls,
        .width = args.width,
        .height = args.height,
    };

    world.screenWidth  = 1024;
    world.screenHeight = 1024;
    world.cellSize     = 25;
    world.wallWidth    = 4;

    world.map.buildRandomMaze();

    bool reset   = false;
    bool autoRun = false;

    float resetAfter = 2;
    float resetAt = 0;

    float solveSpeed = 0;
    float solveLastTime = 0;
    bool       isSolved = false;
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

        // scale the map
        if (world.map.width > world.map.height) {
            world.cellSize = world.screenWidth / world.map.width;
        } else {
            world.cellSize = world.screenHeight / world.map.width;
        }

        world.player.lastmoved += world.deltaTime;
        solveLastTime += world.deltaTime;
        resetAt += world.deltaTime;

        // player input
        if (world.player.lastmoved > world.player.movecooldown) {

            do_player_move(world.glwin, world.map, world.player);

            if (glfwGetKey(world.glwin, GLFW_KEY_A)) {

                world.player.lastmoved = 0;

                autoRun = !autoRun;
            }

            if(glfwGetKey(world.glwin, GLFW_KEY_I)) {

                std::cout << "==============================" << std::endl;

                for (int y = 0; y < world.map.height; y++) {

                    for (int x = 0; x < world.map.width; x++) {

                        std::cout << pad_left(std::to_string(world.map.at(x, y)->distance), 4) << " ";

                    }

                    std::cout << std::endl;
                }
            }
        }

        // auto solving
        if (solveLastTime > solveSpeed) {

            solveLastTime = 0;

            if (autoRun || glfwGetKey(world.glwin, GLFW_KEY_SPACE)) {

                switch (strategy) {

                case DFS:

                    if (isSolved) {
                        reset = dfs_show_path(world.map, world.player, visitHistory);
                    } else {
                        dfs_solve_maze(world.map, world.player, visitHistory, isSolved);
                    }

                    break;

                case FLOODFILL:

                    if (isSolved) {
                        reset = floodfill_show_path(world.map, world.player);
                    } else {
                        floodfill_solve_maze(world.map, world.player, floodnext, isSolved);
                    }

                    break;
                }
            }

            if (!reset) {
                resetAt = 0;
            }
        }

        // map reset
        if ((resetAt > resetAfter) && autoRun && reset || glfwGetKey(world.glwin, GLFW_KEY_R)) {

            reset    = false;
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
