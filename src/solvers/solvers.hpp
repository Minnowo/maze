
#ifndef SOLVERS_H
#define SOLVERS_H

#include <stack>
#include <queue>
#include "glm/fwd.hpp"

#include "../world.hpp"

typedef enum { DFS, FLOODFILL } SolveStrat;

void dfs_show_path(Map& map, Player& player, std::stack<glm::i32vec2>& history);
void dfs_solve_maze(Map& map, Player& player, std::stack<glm::i32vec2>& history, bool& isSolved);

void floodfill_show_path(Map& map, Player& player);
void floodfill_solve_maze(Map& map, Player& player, std::queue<glm::i32vec2>& history, bool& isSolved);

#endif
