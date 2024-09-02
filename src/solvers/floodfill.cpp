
#include "solvers.hpp"

void floodfill_show_path(Map& map, Player& player) {

    if (player.lastmoved < player.movecooldown) {
        return;
    }

    int x = player.x;
    int y = player.y;

    Cell* cell = map.at(x, y);

    int maxDistance = cell->distance;

    for (int i = 0; i < 4; i++) {

        Direction dir = Direction(i);

        if (!map.canMove(x, y, dir))
            continue;

        int nx = x;
        int ny = y;

        switch (dir) {
        case NORTH: ny--; break;
        case SOUTH: ny++; break;
        case EAST : nx++; break;
        case WEST : nx--; break;
        }

        Cell* cell = map.at(nx, ny);

        if (cell->distance == maxDistance - 1) {

            x = nx;
            y = ny;

            maxDistance--;

            player.x = x;
            player.y = y;

            cell->color = ColorPath;

            return;
        }
    }
}

void floodfill_solve_maze(Map& map, Player& player, std::queue<glm::i32vec2>& history, bool& isSolved) {

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

        int nx = x;
        int ny = y;

        switch (move_to) {
        case NORTH: ny--; break;
        case SOUTH: ny++; break;
        case EAST : nx++; break;
        case WEST : nx--; break;
        }

        if (cell->wallAt(move_to))
            continue;

        Cell* newCell = map.at(nx, ny);

        if (newCell->visited || newCell->wallOpposite(move_to))
            continue;

        newCell->distance = cell->distance + 1;

        history.push({nx, ny});
    }

    if (history.empty())
        return;

    glm::i32vec2 pos = history.front();

    history.pop();

    player.x = pos.x;
    player.y = pos.y;
}
