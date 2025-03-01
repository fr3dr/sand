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
    STATIC,
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
    LIQUID,
    GAS
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
    int aliveTime = 0;
};

struct Button {
    CellType type;
    Color color;
};


// define particles
Particle emptyParticle;
Particle staticParticle = {.type = STATIC, .catagory = SOLID, .color = {30, 30, 30, 255}};
Particle sandParticle = {.type = SAND, .catagory = SOLID, .color = BEIGE};
Particle waterParticle = {.type = WATER, .catagory = LIQUID, .color = BLUE};
Particle stoneParticle = {.type = STONE, .catagory = SOLID, .color = DARKGRAY};
Particle acidParticle = {.type = ACID, .catagory = LIQUID, .color = GREEN};
Particle steamParticle = {.type = STEAM, .catagory = GAS, .color = GRAY};
Particle lavaParticle = {.type = LAVA, .catagory = LIQUID, .color = {255, 100, 0, 255}};


const int buttonSize = 32;

const int screenWidth = 1024;
const int screenHeight = 1024;
const int cellSize = 8;
const int gridWidth = screenWidth / cellSize;
const int gridHeight = screenHeight / cellSize - buttonSize / cellSize;
Particle grid[gridWidth][gridHeight];

int brushSize = 2;

bool clearMode = false;
int clearModeSpeed = 6;


// helper functions
bool inBounds(int x, int y) {
    return x >= 0 && y >= 0 && x < gridWidth && y < gridHeight;
}

Color randomColorTint(Color baseColor, int bottomRange, int topRange) {
    Vector3 color = ColorToHSV(baseColor);
    color.z = (float) GetRandomValue(bottomRange, topRange) / 100.0f;
    return ColorFromHSV(color.x, color.y, color.z);
}

bool chance(int chance) {
    return GetRandomValue(1, 100) <= chance;
}


// update functions
void updateSand(int x, int y, Particle particle) {
    int rand = chance(50) ? -1 : 1;

    if (inBounds(x, y+1) && grid[x][y+1].type == EMPTY) {
        grid[x][y] = emptyParticle;
        grid[x][y+1] = particle;
    } else if (inBounds(x+rand, y+1) && grid[x+rand][y+1].type == EMPTY && grid[x+rand][y].catagory != SOLID) {
        grid[x][y] = emptyParticle;
        grid[x+rand][y+1] = particle;
    }
    
    else if (inBounds(x, y+1) && grid[x][y+1].catagory == LIQUID) {
        if (chance(80)) {
            grid[x][y] = grid[x][y+1];
            grid[x][y+1] = particle;
        }
    } else if (inBounds(x+rand, y+1) && grid[x+rand][y+1].catagory == LIQUID && grid[x+rand][y].catagory != SOLID) {
        if (chance(30)) {
            grid[x][y] = grid[x+rand][y+1];
            grid[x+rand][y+1] = particle;
        }
    }

    else if (inBounds(x, y+1) && grid[x][y+1].catagory == GAS) {
        grid[x][y] = grid[x][y+1];
        grid[x][y+1] = particle;
    }
}

void updateWater(int x, int y, Particle particle) {
    particle.color = chance(5) ? randomColorTint(waterParticle.color, 80, 100) : particle.color; 
    grid[x][y] = particle;

    int rand = chance(50) ? -1 : 1;

    if (inBounds(x, y+1) && grid[x][y+1].type == EMPTY) {
        grid[x][y] = emptyParticle;
        grid[x][y+1] = particle;
    }

    else if (inBounds(x, y+1) && grid[x][y+1].catagory == GAS) {
        grid[x][y] = grid[x][y+1];
        grid[x][y+1] = particle;
    } else if (inBounds(x, y+1) && grid[x][y+1].catagory == LIQUID && grid[x][y+1].type != WATER) {
        grid[x][y] = grid[x][y+1];
        grid[x][y+1] = particle;
    }

    else if (inBounds(x+1, y+1) && inBounds(x-1, y+1) && grid[x+1][y+1].type == EMPTY && grid[x-1][y+1].type == EMPTY) {
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

void updateStone(int x, int y, Particle particle) {
    if (inBounds(x, y+1) && grid[x][y+1].type == EMPTY) {
        grid[x][y] = emptyParticle;
        grid[x][y+1] = particle;
    } else if (inBounds(x, y+1) && (grid[x][y+1].catagory == LIQUID || grid[x][y+1].catagory == GAS)) {
        grid[x][y] = grid[x][y+1];
        grid[x][y+1] = particle;
    }
}

void updateAcid(int x, int y, Particle particle) {
    int rand = chance(50) ? -1 : 1;

    if (inBounds(x, y+1) && grid[x][y+1].type == EMPTY) {
        grid[x][y] = emptyParticle;
        grid[x][y+1] = particle;
    }

    else if (inBounds(x, y+1) && grid[x][y+1].type != EMPTY && grid[x][y+1].type != ACID && chance(20)) {
        grid[x][y] = emptyParticle;
        grid[x][y+1] = chance(80) ? emptyParticle : particle;
    } else if (inBounds(x, y-1) && grid[x][y-1].type != EMPTY && grid[x][y-1].type != ACID && chance(20)) {
        grid[x][y] = emptyParticle;
        grid[x][y-1] = chance(80) ? emptyParticle : particle;
    } else if (inBounds(x+1, y) && grid[x+1][y].type != EMPTY && grid[x+1][y].type != ACID && chance(20)) {
        grid[x][y] = emptyParticle;
        grid[x+1][y] = chance(80) ? emptyParticle : particle;
    } else if (inBounds(x+1, y) && grid[x+1][y].type != EMPTY && grid[x+1][y].type != ACID && chance(20)) {
        grid[x][y] = emptyParticle;
        grid[x+1][y] = chance(80) ? emptyParticle : particle;
    }

    else if (inBounds(x+1, y+1) && inBounds(x-1, y+1) && grid[x+1][y+1].type == EMPTY && grid[x-1][y+1].type == EMPTY) {
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

void updateSteam(int x, int y, Particle particle) {
    if (particle.aliveTime > 1024 && chance(1)) {
        grid[x][y] = waterParticle;
        return;
    }

    particle.color = ColorBrightness(steamParticle.color, -particle.aliveTime / 3000.0f);
    grid[x][y] = particle;

    int randX = chance(50) ? -1 : 1;
    int randY = chance(90) ? -1 : 1;

    if (inBounds(x+randX, y+randY) && grid[x+randX][y+randY].type == EMPTY) {
        grid[x][y] = emptyParticle;
        grid[x+randX][y+randY] = particle;
    } else if (inBounds(x+randX, y+randY) && grid[x+randX][y+randY].catagory == LIQUID) {
        grid[x][y] = grid[x+randX][y+randY];
        grid[x+randX][y+randY] = particle;
    }
}

void updateLava(int x, int y, Particle particle) {
    particle.color = chance(5) ? randomColorTint(lavaParticle.color, 50, 100) : particle.color;
    grid[x][y] = particle;

    int rand = chance(50) ? -1 : 1;

    if (inBounds(x, y+1) && grid[x][y+1].type == EMPTY) {
        grid[x][y] = emptyParticle;
        grid[x][y+1] = particle;
    }

    else if (inBounds(x, y+1) && grid[x][y+1].catagory == GAS) {
        grid[x][y] = grid[x][y+1];
        grid[x][y+1] = particle;
    }

    else if (inBounds(x, y+1) && grid[x][y+1].type == WATER) {
        grid[x][y] = steamParticle;
        grid[x][y+1] = stoneParticle;
    } else if (inBounds(x, y-1) && grid[x][y-1].type == WATER) {
        grid[x][y-1] = steamParticle;
        grid[x][y] = stoneParticle;
    } else if (inBounds(x+1, y) && grid[x+1][y].type == WATER) {
        grid[x][y] = steamParticle;
        grid[x+1][y] = stoneParticle;
    } else if (inBounds(x-1, y) && grid[x-1][y].type == WATER) {
        grid[x][y] = steamParticle;
        grid[x-1][y] = stoneParticle;
    }

    else if (inBounds(x+1, y+1) && inBounds(x-1, y+1) && grid[x+1][y+1].type == EMPTY && grid[x-1][y+1].type == EMPTY) {
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


int main() {
    // initialize grid
    for (int x = 0; x < gridWidth; x++) {
        for (int y = 0; y < gridHeight; y++) {
            grid[x][y] = emptyParticle;
        }
    }

    // create buttons
    Button staticButton = {staticParticle.type, staticParticle.color};
    Button stoneButton = {stoneParticle.type, stoneParticle.color};
    Button sandButton = {sandParticle.type, sandParticle.color};
    Button waterButton = {waterParticle.type, waterParticle.color};
    Button acidButton = {acidParticle.type, acidParticle.color};
    Button steamButton = {steamParticle.type, steamParticle.color};
    Button lavaButton = {lavaParticle.type, lavaParticle.color};
    std::array buttonArray = std::array<Button, 7>{staticButton, stoneButton, sandButton, waterButton, acidButton, steamButton, lavaButton};

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
                case STATIC:
                    newParticle = staticParticle;
                    break;
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
                case STEAM:
                    newParticle = steamParticle;
                    break;
                case LAVA:
                    newParticle = lavaParticle;
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
                if (clearMode) {
                    break;
                }

                Particle* particle = &grid[x][y];

                if (particle->type == EMPTY || particle->updatedThisFrame) {
                    continue;
                }

                particleAmmount++;
                particle->updatedThisFrame = true;
                particle->aliveTime++;

                switch (particle->type) {
                    case SAND:
                        updateSand(x, y, *particle);
                        break;
                    case WATER:
                        updateWater(x, y, *particle);
                        break;
                    case STONE:
                        updateStone(x, y, *particle);
                        break;
                    case ACID:
                        updateAcid(x, y, *particle);
                        break;
                    case STEAM:
                        updateSteam(x, y, *particle);
                        break;
                    case LAVA:
                        updateLava(x, y, *particle);
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
