#include <array>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <ostream>
#include <string>

#include "raylib.h"


enum CellType {
    EMPTY,
    STONE,
    SAND,
    WATER,
    ACID,
    LAVA,
    STEAM
};

enum CellCatagory {
    NONE,
    SOLID,
    LIQUID
};

struct Particle {
    CellType type = EMPTY;
    CellCatagory catagory = NONE;
    Color color = WHITE;
    int dirX = 0;
    int dirY = 0;
    bool onFloor = false;
    int fallingTime = 0;
    bool updatedThisFrame = false;
};

struct Button {
    CellType type;
    Color color;
};


// define particles
const Color stoneColor = DARKGRAY;
const Color sandColor = BEIGE;
const Color waterColor = BLUE;
const Color acidColor = GREEN;

Particle emptyParticle;
Particle sandParticle = {.type = SAND, .catagory = SOLID, .color = sandColor};
Particle waterParticle = {.type = WATER, .catagory = LIQUID, .color = waterColor};
Particle stoneParticle = {.type = STONE, .catagory = SOLID, .color = stoneColor};
Particle acidParticle = {.type = ACID, .catagory = LIQUID, .color = acidColor};


const int buttonSize = 32;

const int screenWidth = 1024;
const int screenHeight = 1024;
const int cellSize = 8;
const int gridWidth = screenWidth / cellSize;
const int gridHeight = screenHeight / cellSize - buttonSize / cellSize;
Particle grid[gridWidth][gridHeight];

int brushSize = 2;

bool clearMode = false;
int clearModeSpeed = 3;


// helper functions
bool inBounds(int x, int y) {
    return x >= 0 && y >= 0 && x < gridWidth && y < gridHeight;
}

Color randomColorTint(Color baseColor, int bottomRange, int topRange) {
    Vector3 color = ColorToHSV(baseColor);
    color.z = (float) GetRandomValue(bottomRange, topRange) / 100.0f;
    return ColorFromHSV(color.x, color.y, color.z);
}


// update functions
void updateSand(int x, int y, Particle particle) {
    int rand = GetRandomValue(1, 100) < 50 ? -1 : 1;

    if (inBounds(x, y+1) && grid[x][y+1].type == EMPTY) {
        grid[x][y] = emptyParticle;
        grid[x][y+1] = particle;
    } else if (inBounds(x+rand, y+1) && grid[x+rand][y+1].type == EMPTY) {
        grid[x][y] = emptyParticle;
        grid[x+rand][y+1] = particle;
    } else if (inBounds(x, y+1) && grid[x][y+1].catagory == LIQUID) {
        if (GetRandomValue(1, 100) < 80) {
            grid[x][y] = grid[x][y+1];
            grid[x][y + 1] = particle;
        }
    } else if (inBounds(x+rand, y+1) && grid[x+rand][y+1].catagory == LIQUID) {
        if (GetRandomValue(1, 100) < 30) {
            grid[x][y] = grid[x+rand][y+1];
            grid[x+rand][y+1] = particle;
        }
    }
}

void updateWater(int x, int y, Particle particle) {
    particle.color = GetRandomValue(1, 100) < 5 ? randomColorTint(waterColor, 80, 100) : particle.color; 
    grid[x][y] = particle;

    int rand = GetRandomValue(1, 100) < 50 ? -1 : 1;

    if (inBounds(x, y+1) && grid[x][y+1].type == EMPTY) {
        grid[x][y] = emptyParticle;
        grid[x][y+1] = particle;
    } else if (inBounds(x+1, y+1) && inBounds(x-1, y+1) && grid[x+1][y+1].type == EMPTY && grid[x-1][y+1].type == EMPTY) {
        grid[x][y] = emptyParticle;
        grid[x+rand][y+1] = particle;
    } else if (inBounds(x+1, y+1) && grid[x+1][y+1].type == EMPTY) {
        grid[x][y] = emptyParticle;
        grid[x+1][y+1] = particle;
    } else if (inBounds(x-1, y+1) && grid[x-1][y+1].type == EMPTY) {
        grid[x][y] = emptyParticle;
        grid[x-1][y+1] = particle;
    } else if (!inBounds(x, y+1) || grid[x][y+1].type != EMPTY) {
        if (particle.dirX == 0) {
            particle.dirX = rand;
        }
        
        if (!inBounds(x+particle.dirX, y) || grid[x+particle.dirX][y].type != EMPTY) {
            particle.dirX = -particle.dirX;
        }

        if (inBounds(x+particle.dirX, y) && grid[x+particle.dirX][y].type == EMPTY) {
            grid[x][y] = emptyParticle;
            grid[x+particle.dirX][y] = particle;
        }
    }
}

void updateAcid(int x, int y, Particle particle) {
    int rand = GetRandomValue(1, 100) < 50 ? -1 : 1;

    if (inBounds(x, y+1) && grid[x][y+1].type == EMPTY) {
        grid[x][y] = emptyParticle;
        grid[x][y+1] = particle;
    } else if (inBounds(x, y+1) && grid[x][y+1].type != EMPTY && grid[x][y+1].type != ACID && GetRandomValue(1, 100) < 20) {
        grid[x][y] = emptyParticle;
        grid[x][y+1] = GetRandomValue(1, 100) < 80 ? emptyParticle : particle;
    } else if (inBounds(x+1, y+1) && inBounds(x-1, y+1) && grid[x+1][y+1].type == EMPTY && grid[x-1][y+1].type == EMPTY) {
        grid[x][y] = emptyParticle;
        grid[x+rand][y+1] = particle;
    } else if (inBounds(x+1, y+1) && grid[x+1][y+1].type == EMPTY) {
        grid[x][y] = emptyParticle;
        grid[x+1][y+1] = particle;
    } else if (inBounds(x-1, y+1) && grid[x-1][y+1].type == EMPTY) {
        grid[x][y] = emptyParticle;
        grid[x-1][y+1] = particle;
    } else if (!inBounds(x, y+1) || grid[x][y+1].type != EMPTY) {
        if (particle.dirX == 0) {
            particle.dirX = rand;
        }
        
        if (!inBounds(x+particle.dirX, y) || grid[x+particle.dirX][y].type == ACID) {
            particle.dirX = -particle.dirX;
        }

        if (inBounds(x+particle.dirX, y) && grid[x+particle.dirX][y].type == EMPTY) {
            grid[x][y] = emptyParticle;
            grid[x+particle.dirX][y] = particle;
        }
    }
}


int main() {
    // initialize grid
    for (int x = 0; x < gridWidth; x++) {
        for (int y = 0; y < gridHeight; y++) {
            grid[x][y] = emptyParticle;
        }
    }

    // create buttons
    Button stoneButton = {STONE, stoneColor};
    Button sandButton = {SAND, sandColor};
    Button waterButton = {WATER, waterColor};
    Button acidButton = {ACID, acidColor};
    std::array buttonArray = std::array<Button, 4>{stoneButton, sandButton, waterButton, acidButton};

    CellType selectedCellType = SAND;

    // create window
    InitWindow(screenWidth, screenHeight, "sand");
    SetTargetFPS(60);

    while (!WindowShouldClose()) {
        // get mouse grid position
        int mouseGridX = GetMouseX() / cellSize;
        int mouseGridY = GetMouseY() / cellSize;

        // clamp mouse grid position
        if (mouseGridX < 0) {
            mouseGridX = 0;
        }
        if (mouseGridX > gridWidth - 1) {
            mouseGridX = gridWidth - 1;
        }
        if (mouseGridY < 0) {
            mouseGridY = 0;
        }
        if (mouseGridY > gridHeight - 1) {
            mouseGridY = gridHeight - 1;
        }

        // button logic
        if (GetMouseY() > gridHeight * cellSize && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            for (int i = 0; i < buttonArray.size(); i++) {
                Button button = buttonArray.at(i);
                int posX = i * buttonSize;
                if (GetMouseX() > posX && GetMouseX() < posX + buttonSize) {
                    selectedCellType = button.type;
                }
            }
        }

        // placing
        if (IsMouseButtonDown(MOUSE_BUTTON_LEFT) && GetMouseY() < gridHeight * cellSize && clearMode == false){
            Particle newParticle = emptyParticle;
            switch (selectedCellType) {
                case SAND:
                    newParticle = sandParticle;
                    newParticle.color = randomColorTint(newParticle.color, 50, 80);
                    break;
                case WATER:
                    newParticle = waterParticle;
                    break;
                case STONE:
                    newParticle = stoneParticle;
                    break;
                case ACID:
                    newParticle = acidParticle;
                    break;
                default:
                    break;
            }

            for (int i = -brushSize; i < brushSize; i++) {
                for (int j = -brushSize; j < brushSize; j++) {
                    if (inBounds(mouseGridX + i, mouseGridY + j) && grid[mouseGridX + i][mouseGridY + j].type == EMPTY) {
                        grid[mouseGridX + i][mouseGridY + j] = newParticle;
                    }
                }
            }
        }

        // erasing
        if (IsMouseButtonDown(MOUSE_BUTTON_RIGHT)) {
            for (int i = -brushSize; i < brushSize; i++) {
                for (int j = -brushSize; j < brushSize; j++) {
                    if (inBounds(mouseGridX + i, mouseGridY + j)) {
                        grid[mouseGridX + i][mouseGridY + j] = emptyParticle;
                    }
                }
            }
        }

        // clearing
        if (IsKeyPressed(KEY_R)) {
            clearMode = true;
        }

        int particleAmmount = 0;
        if (clearMode) {
            for (int x = gridWidth - 1; x >= 0; x--) {
                for (int y = gridHeight - 1; y >= 0; y--) {
                    Particle particle = grid[x][y];
                    if (particle.type == EMPTY) continue;
                    particleAmmount++;

                    if (inBounds(x, y+clearModeSpeed) && grid[x][y+clearModeSpeed].type == EMPTY) {
                        grid[x][y] = emptyParticle;
                        grid[x][y+clearModeSpeed] = particle;
                    } else if (!inBounds(x, y+clearModeSpeed)) {
                        grid[x][y] = emptyParticle;
                    }
                }

            }
        }

        if (particleAmmount == 0) {
            clearMode = false;
        }

        // grid logic
        for (int x = 0; x < gridWidth; x++) {
            for (int y = 0; y < gridHeight; y++) {
                Particle particle = grid[x][y];

                if (particle.type == EMPTY || particle.updatedThisFrame || clearMode) {
                    continue;
                }

                particleAmmount++;
                particle.updatedThisFrame = true;

                switch (particle.type) {
                    case SAND:
                        updateSand(x, y, particle);
                        break;
                    case WATER:
                        updateWater(x, y, particle);
                        break;
                    case ACID:
                        updateAcid(x, y, particle);
                        break;
                    default:
                        break;
                }
            }
        } 

        std::cout << "ammout " << particleAmmount << "\n";

        // reset updated this frame
        for (int i = 0; i < gridWidth; i++) {
            for (int j = 0; j < gridHeight; j++) {
                grid[i][j].updatedThisFrame = false;
            }
        } 

        // drawing
        BeginDrawing();
        ClearBackground(WHITE);

        // draw grid
        for (int x = 0; x < gridWidth; x++) {
            for (int y = 0; y < gridHeight; y++) {
                if (grid[x][y].type != EMPTY) {
                    DrawRectangle(x * cellSize, y * cellSize, cellSize, cellSize, grid[x][y].color);
                }
            }
        } 

        // draw buttons
        for (int i = 0; i < buttonArray.size(); i++) {
            Button button = buttonArray.at(i);
            DrawRectangle(i * buttonSize, gridHeight * cellSize, buttonSize, buttonSize, button.color);
            if (button.type == selectedCellType) {
                DrawRectangleLines(i * buttonSize, gridHeight * cellSize, buttonSize, buttonSize, BLACK);
            }
        }

        DrawFPS(5, 5);
        DrawText(std::to_string(mouseGridX).c_str(), 5, 25, 20, BLACK);
        DrawText(std::to_string(mouseGridY).c_str(), 5, 45, 20, BLACK);

        EndDrawing();
    }

    CloseWindow();
    return 0;
}
