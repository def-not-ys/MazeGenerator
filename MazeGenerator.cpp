#include <iostream>
#include <stack>
#include <vector>
using namespace std;

#include "/CPP/lib/olcConsoleGameEngine.h"


class Maze : public olcConsoleGameEngine {

public:
    Maze() {
        m_sAppName = L"MAZE";
    }

private:
    int m_maze_width;
    int m_maze_height;
    int* m_maze;

    enum {
        CELL_PATH_N = 0x01,  // 0000 0001
        CELL_PATH_E = 0x02,  // 0000 0010
        CELL_PATH_S = 0x04,  // 0000 0100
        CELL_PATH_W = 0x08,  // 0000 1000
        CELL_VISITED = 0x10, // 0001 0000
    };

    int m_visited_cells;

    stack<pair<int, int>> m_stack;

    // for drawing path 
    int m_path_width;

protected:
    virtual bool OnUserCreate() {

        m_maze_width = 40;
        m_maze_height = 25;
        m_path_width = 3;

        m_maze = new int[m_maze_width * m_maze_height];

        // initialize cells with value 0
        memset(m_maze, 0x00, m_maze_width * m_maze_height * sizeof(int));

        // choose a starting cell 
        int x = rand() % m_maze_width;
        int y = rand() % m_maze_height;
        m_stack.push(make_pair(0 , 0));
        m_maze[0] = CELL_VISITED;
        //m_maze[y * m_maze_width + 1] = CELL_VISITED;
        m_visited_cells = 1;

        return true;
    }

    virtual bool OnUserUpdate(float fElaspsedTime) {

        constructMaze();

        // Drawing 
        // clear screen 
        Fill(0, 0, m_nScreenWidth, m_nScreenHeight, L' ');

        // draw maze
        for (int x = 0; x < m_maze_width; x++) {
            for (int y = 0; y < m_maze_height; y++) {
                // drawing the cells 
                for (int px = 0; px < m_path_width; px++) {
                    for (int py = 0; py < m_path_width; py++) {

                        if (m_maze[y * m_maze_width + x] & CELL_VISITED)
                            Draw(x * (m_path_width + 1) + px, y * (m_path_width + 1) + py, PIXEL_SOLID, FG_WHITE);
                        else
                            Draw(x * (m_path_width + 1) + px, y * (m_path_width + 1) + py, PIXEL_SOLID, FG_BLUE);
                    }
                }

                // drawing paths between cells 
                for (int p = 0; p < m_path_width; p++) {
                    // walls are shared between two cells so only need to check two directions 
                    if (m_maze[y * m_maze_width + x] & CELL_PATH_S)
                        Draw(x * (m_path_width + 1) + p, y * (m_path_width + 1) + m_path_width, PIXEL_SOLID, FG_WHITE);
                    if (m_maze[y * m_maze_width + x] & CELL_PATH_E)
                        Draw(x * (m_path_width + 1) + m_path_width, y * (m_path_width + 1) + p, PIXEL_SOLID, FG_WHITE);
                }
            }
        }

        // draw top of the stack 
        for (int px = 0; px < m_path_width; px++) 
            for (int py = 0; py < m_path_width; py++)
                Draw(m_stack.top().first * (m_path_width + 1) + px, m_stack.top().second * (m_path_width + 1) + py, 0x2588, FG_GREEN);

        return true;
    }

    void constructMaze() {
        
        auto offset = [&](int x, int y) {
            return (m_stack.top().second + y) * m_maze_width + (m_stack.top().first + x);
        };

        // construct maze algorithm 
        if (m_visited_cells < m_maze_width * m_maze_height) {

            vector<int> neighbours;

            // step 1: create a set of the unvisited neighbours
            // north neighbour (check if the cell is at the top row, cell.y > 0)
            if (m_stack.top().second > 0  && (m_maze[offset(0, -1)] & CELL_VISITED) == 0)
                neighbours.push_back(0); // using 0 to represent North neighbour

            // east neighbour (check if the cell is at the right column, cell.x < m_maze_width - 1 )
            if (m_stack.top().first < m_maze_width - 1 && (m_maze[offset(1, 0)] & CELL_VISITED) == 0)
                neighbours.push_back(1); // using 1 to represent East neighbour

            // south neighbour (check if the cell is at the bottom row, cell.y < m_maze_height - 1)
            if (m_stack.top().second < m_maze_height - 1 && (m_maze[offset(0, 1)] & CELL_VISITED) == 0)
                neighbours.push_back(2); // using 2 to represent South neighbour

            // west neighbour (check if the cell is at the right column, cell.x > 0)
            if (m_stack.top().first > 0 && (m_maze[offset(-1, 0)] & CELL_VISITED) == 0)
                neighbours.push_back(3); // using 3 to represent West neighbour

            // check if there is valid neighours
            if (!neighbours.empty()) {
                // choose a random direction 
                int next_cell_dir = neighbours[rand() % neighbours.size()];

                // create a path between the neighbour and current cell
                switch (next_cell_dir) {
                case 0: // North 
                    m_maze[offset(0, 0)] |= CELL_PATH_N;    // current cell has path to north cell
                    m_maze[offset(0, -1)] |= CELL_PATH_S;   // north neighbour has path to south 
                    m_stack.push(make_pair((m_stack.top().first + 0), (m_stack.top().second - 1))); // push neighbour to stack 
                    break;
                case 1: // East 
                    m_maze[offset(0, 0)] |= CELL_PATH_E;    // current cell has path to east cell
                    m_maze[offset(1, 0)] |= CELL_PATH_W;     // east neighbour has path to west
                    m_stack.push(make_pair((m_stack.top().first + 1), (m_stack.top().second + 0))); // push neighbour to stack 
                    break;
                case 2: // South 
                    m_maze[offset(0, 0)] |= CELL_PATH_S;    // current cell has path to south cell
                    m_maze[offset(0, 1)] |= CELL_PATH_N;    // south neighbour has path to north 
                    m_stack.push(make_pair((m_stack.top().first + 0), (m_stack.top().second + 1))); // push neighbour to stack 
                    break;
                case 3: // West 
                    m_maze[offset(0, 0)] |= CELL_PATH_W;    // current cell has path to west cell
                    m_maze[offset(-1, 0)] |= CELL_PATH_E;   // west neighbour has path to east
                    m_stack.push(make_pair((m_stack.top().first - 1), (m_stack.top().second + 0))); // push neighbour to stack 
                    break;
                }

                m_maze[offset(0, 0)] |= CELL_VISITED;
                m_visited_cells++;

            }
            else {
                // backtrack
                m_stack.pop();
            }
        }
    }
};

int main()
{
    srand(clock());

    Maze maze;
    maze.ConstructConsole(160, 100, 8, 8);
    maze.Start();

    return 0;
}
