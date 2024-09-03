
#include <stack>
#include "glm/fwd.hpp"

#include "solvers.hpp"

bool dfs_show_path(Map& map, Player& player, std::stack<glm::i32vec2>& history) {

    if (player.lastmoved < player.movecooldown) {
        return true;
    }

    if (history.empty()) {
        return true;
    }

    glm::i32vec2 pos = history.top();

    history.pop();

    player.x = pos.x;
    player.y = pos.y;

    Cell* cell = map.at(pos.x, pos.y);

    cell->color = ColorPath;

    return false;
}

void dfs_solve_maze(Map& map, Player& player, std::stack<glm::i32vec2>& history, bool& isSolved) {

    if (player.lastmoved < player.movecooldown) {
        return;
    }

    int x = player.x;
    int y = player.y;

    if (isSolved || map.finishPos.x == x && map.finishPos.y == y) {

        isSolved = true;

        return;
    }

    Cell* cell = map.at(x, y);

    cell->visited = true;
    cell->color   = ColorSearch;

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

    for (int i = 0; i < 4; i++) {

        if (!directions.test(i))
            continue;

        Direction move_to = Direction(i);

        if (cell->wallAt(move_to))
            continue;

        int nx = x;
        int ny = y;

        switch (move_to) {
        case NORTH: ny--; break;
        case SOUTH: ny++; break; 
        case EAST : nx++; break;
        case WEST : nx--; break;
        }

        Cell* newCell = map.at(nx, ny);

        if (newCell->visited)
            continue;

        if (newCell->wallOpposite(move_to))
            continue;

        player.x = nx;
        player.y = ny;

        history.push({x, y});

        return;
    }

    if (history.empty())
        return;

    glm::i32vec2 pos = history.top();

    history.pop();

    player.x = pos.x;
    player.y = pos.y;
}
