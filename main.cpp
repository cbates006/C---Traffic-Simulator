#include <iostream>
#include <windows.h>
#include <vector>
#include <cstdlib>
#include <ctime>

// Enums
enum Direction { NORTH, SOUTH, EAST, WEST };
enum LightColor { RED, GREEN };
enum TileType { EMPTY, ROAD, BUILDING, INTERSECTION, GREENLIGHT, REDLIGHT, CAR };

// Define console colors
#define ANSI_COLOR_RED "\x1b[31m"
#define ANSI_COLOR_GREEN "\x1b[32m"
#define ANSI_COLOR_RESET "\x1b[0m" 
#define ANSI_COLOR_YELLOW "\x1b[33m"
#define ANSI_COLOR_BLUE "\x1b[34m"

// Objects
struct Road {
    int locationLeft, locationRight, numCars;
};

struct Car {
    Direction direction;
    int x, y;
};

struct TrafficLight {
    LightColor color;
    int x, y;
};

// Console output optimization
void moveCursorToTop() {
    COORD coord = {0, 0};
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), coord);
}

// Draw city to console
void drawCity(const std::vector<std::vector<TileType>>& city) {
    for (const auto& row : city) {
        for (TileType tile : row) {
            switch (tile) {
                case EMPTY: std::cout << " "; break;
                case GREENLIGHT: std::cout << ANSI_COLOR_GREEN << "O" << ANSI_COLOR_RESET; break;
                case REDLIGHT: std::cout << ANSI_COLOR_RED << "O" << ANSI_COLOR_RESET; break;
                case INTERSECTION:std::cout << "="; break;
                case ROAD: std::cout << "="; break;
                case CAR: std::cout << ANSI_COLOR_YELLOW << "C" << ANSI_COLOR_RESET; break;
                case BUILDING: std::cout << ANSI_COLOR_BLUE << "B" << ANSI_COLOR_RESET; break;
            }
        }
        std::cout << "\n";
    }
}

// Move cars according to tick speed
void moveCar(Car& car, std::vector<std::vector<TileType>>& city, int width, int height) {

    int dx = 0, dy = 0;

    switch (car.direction) {
        case NORTH: dx = -1; break;
        case SOUTH: dx = 1; break;
        case EAST:  dy = 1; break;
        case WEST:  dy = -1; break;
    }

    int nx = car.x + dx;
    int ny = car.y + dy;

    // Wrap when car hits edge of map
    if (nx < 0) nx = height - 1;
    if (nx >= height) nx = 0;
    if (ny < 0) ny = width - 1;
    if (ny >= width) ny = 0;

    TileType current = city[car.x][car.y];
    TileType next = city[nx][ny];
    
    // Stop at redlights
    if (next == REDLIGHT) return;

    // Stop when car in front is stopped
    if (next == CAR) return;

    // Cross on green light
    if (next == GREENLIGHT) {
        nx += dx * 4;
        ny += dy * 4;

        next = city[nx][ny];

        if (next == CAR) return;
    }

    // Move car one tile
    city[car.x][car.y] = ROAD;
    car.x = nx;
    car.y = ny;
    city[car.x][car.y] = CAR;
}

// Logic to generate city
std::vector<std::vector<TileType>> generateCity(int width, int height, int numRoads) {
    std::vector<std::vector<TileType>> city(height, std::vector<TileType>(width, EMPTY));

    int horizon = height / numRoads / 2;
    int vert = width / numRoads / 2;
    int numCars = 8;

    std::vector<Road> roadsList;
    std::vector<TrafficLight> lightList;
    std::vector<Car> carsList;

    // Horizontal roads
    for (int i = 0; i < numRoads; ++i) {
        Road roadObj{horizon, horizon + 1, numCars};
        roadsList.push_back(roadObj);

        for (int x = 0; x < width; ++x) {
            city[roadObj.locationLeft][x] = ROAD;
            city[roadObj.locationRight][x] = ROAD;
        }

        for (int i = 0; i < roadObj.numCars; ++i) {
            int carPlace = rand() % width;

            if (city[roadObj.locationRight][carPlace] == ROAD) {
                city[roadObj.locationRight][carPlace] = CAR;
                carsList.push_back({EAST, roadObj.locationRight, carPlace});
            }

            carPlace = rand() % width;

            if (city[roadObj.locationLeft][carPlace] == ROAD) {
                city[roadObj.locationLeft][carPlace] = CAR;
                carsList.push_back({WEST, roadObj.locationLeft, carPlace});
            }
        }

        horizon += height / numRoads;
    }

    // Vertical roads and intersections
    for (int i = 0; i < numRoads; ++i) {
        Road roadObj{vert, vert + 1, numCars};
        roadsList.push_back(roadObj);

        for (int i = 0; i < height; ++i) {
            if (city[i][roadObj.locationLeft] == ROAD || city[i][roadObj.locationLeft] == CAR) {

                city[i][roadObj.locationLeft] = INTERSECTION;
                city[i][roadObj.locationRight] = INTERSECTION;

                city[i][roadObj.locationLeft - 1] = REDLIGHT;
                lightList.push_back({RED, i, roadObj.locationLeft - 1});

                city[i][roadObj.locationRight + 1] = REDLIGHT;
                lightList.push_back({RED, i, roadObj.locationRight + 1});

                if (city[i - 1][roadObj.locationLeft] != INTERSECTION &&
                    city[i - 1][roadObj.locationRight] != INTERSECTION) {

                    city[i - 1][roadObj.locationLeft] = GREENLIGHT;
                    lightList.push_back({GREEN, i - 1, roadObj.locationLeft});

                    city[i - 1][roadObj.locationRight] = GREENLIGHT;
                    lightList.push_back({GREEN, i - 1, roadObj.locationRight});

                    city[i + 2][roadObj.locationLeft] = GREENLIGHT;
                    lightList.push_back({GREEN, i + 2, roadObj.locationLeft});

                    city[i + 2][roadObj.locationRight] = GREENLIGHT;
                    lightList.push_back({GREEN, i + 2, roadObj.locationRight});
                }
            } 

            else {
                if (city[i][roadObj.locationLeft] != GREENLIGHT) {
                    city[i][roadObj.locationLeft] = ROAD;
                    city[i][roadObj.locationRight] = ROAD;
                }
            }
        }

        // Place cars in random locations along roads
        for (int i = 0; i < roadObj.numCars; ++i) {
            int carPlace = rand() % height;

            if (city[carPlace][roadObj.locationRight] == ROAD) {
                city[carPlace][roadObj.locationRight] = CAR;
                carsList.push_back({NORTH, carPlace, roadObj.locationRight});
            }

            carPlace = rand() % height;

            if (city[carPlace][roadObj.locationLeft] == ROAD) {
                city[carPlace][roadObj.locationLeft] = CAR;
                carsList.push_back({SOUTH, carPlace, roadObj.locationLeft});
            }
        }

        vert += width / numRoads;
    }

    // Place buildings alongside roads
    for (int h = 0; h < height; ++h) {
        for (int w = 0; w < width; ++w) {
            if (city[h][w] == EMPTY && (
                (h > 0 && city[h - 1][w] == ROAD) ||
                (h < height - 1 && city[h + 1][w] == ROAD) ||
                (w > 0 && city[h][w - 1] == ROAD) ||
                (w < width - 1 && city[h][w + 1] == ROAD))) {

                if (rand() % 100 < 15) {
                    city[h][w] = BUILDING;
                }
            }
        }
    }

    // Loop to update city
    SetConsoleOutputCP(CP_UTF8);
    int tick = 0;

    while (true) {
        moveCursorToTop();
        drawCity(city);

        // Move cars
        for (auto& car : carsList) {
            moveCar(car, city, width, height);
        }

        // Toggle lights
        if (tick % 10 == 0) {
            for (auto& light : lightList) {
                light.color = (light.color == GREEN) ? RED : GREEN;
                city[light.x][light.y] =
                    (light.color == GREEN) ? GREENLIGHT : REDLIGHT;
            }
        }

        tick++;
        Sleep(200);
    }
}

// Main function
int main() {
    int width;
    int height;
    int numRoads;
    srand(time(0));

    std::cout << "Welcome to Traffic Simulator! This is a small C++ game that generates a visualization of a city using console outputs and simulates cars driving along the roads."<< std::endl;
    std::cout << "The cars will stop at red lights and move through intersections when the light is green. To make this program friendly to all machines, each element is represented" << std::endl;
    std::cout << "by a letter or symbol with 'C' being cars and multicolored 'O's being lights. When you are ready to begin, enter the number of roads you would like to simulate." << std::endl;
    std::cout << "It is recommended to enter a number between 1 and 6, though you can go up to 8 if you have a large enough display window. It is also recommended that you make your" << std::endl;
    std::cout << "console window full-screen." << std::endl;
    
    std::cin >> numRoads;

    std::cout << "Now enter the width you would like your city to be followed by the height. A good starting size would be 90x50 to see how well it size fits your console window." << std::endl;

    std::cin >> width >> height;
    
    generateCity(width,height,numRoads);
    return 0;
}