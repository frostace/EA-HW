#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <signal.h> // raise(SIGTRAP)
#include <stdlib.h>

// Cube
#include "Cube.cpp"
#include "Creature.cpp"
#include "omp.h"

int iterNums = 0;

using namespace std;
double xpos_old, ypos_old;
ofstream myfile, skeleton, energy;
float breathingT0 = -1.0f;

void writeToFile(string content)
{
    if (myfile.is_open())
    {
        myfile << content;
    }
}

void random_search()
{
    int sizeX = 3, sizeY = 3, sizeZ = 3;
    float bestDistance = 0;
    std::string bestRobotTypes = "";
    Creature robot(sizeX, sizeY, sizeZ);

    std::cout << "init finish" << std::endl;
    for (int i = 0; i < evalNums; i++)
    {
        robot.settle();
        robot.measurePerformance();
        if (robot.scaledCentroidX > bestDistance)
        {
            bestDistance = robot.scaledCentroidX;
            bestRobotTypes = robot.saveSkeleton();
        }

        std::cout << bestDistance << std::endl;
        writeToFile(std::to_string(bestDistance));

        robot.mutate();
    }

    // write best robot distance and spring types to file
    skeleton << bestRobotTypes;
}

int main(int argc, char **argv)
{
    srand(time(NULL));

    int runtimes = strtol(argv[1], nullptr, 10);

    for (int t = 0; t < runtimes; t++)
    {
        // init file
        skeleton.open("skeleton-" + to_string(t) + ".txt");
        myfile.open("random-" + to_string(t) + ".txt");

        random_search();

        myfile.close();
        skeleton.close();
    }

    // Creature robot(20, 1, 1);
    // robot.loadSkeleton("skeleton-0.txt");
    // robot.printInfo();
    // std::cout << "after loading" << std::endl;

    // std::ofstream skeleton_new;
    // skeleton_new.open("skeleton_new.txt");
    // robot.saveSkeleton(skeleton_new);
    // robot.printInfo();
    // skeleton_new.close();

    return 0;
}
