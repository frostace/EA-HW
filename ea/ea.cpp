#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <signal.h> // raise(SIGTRAP)
#include <stdlib.h>

// Cube
#include "Cube.cpp"
#include "Creature.cpp"
#include "Population.cpp"
#include "Constants.h"

#include "omp.h"

int iterNums = 0;

using namespace std;
ofstream myfile, skeleton, energy;
float breathingT0 = -1.0f;

void writeToFile(string content)
{
    if (myfile.is_open())
    {
        myfile << content;
    }
}

void fillArrWithRandomColor(float *arr)
{
    for (int i = 0; i < 3; i++)
        *arr++ = rand() / (float)RAND_MAX;
}

void test(Creature &robot)
{
    int frameNum = 0;
    // Loop for several iterations and measure X distance at the end
    while (frameNum < 2000)
    {
        frameNum += 1;

        // =====================================================================
        //  Update cube
        // =====================================================================
        robot.iterate();
        // robot.printInfo();
        // robot.saveEnergyTo(myfile);
    }
    cout << "robot dist: " << robot.scaledCentroidX << endl;
}

void ea()
{
    // init population, bestDistance and bestRobotTypes
    int sizeX = 3, sizeY = 3, sizeZ = 3;
    float bestDistance = 0;
    std::string bestRobotTypes = "";

    int popSize = 100;

    Population population(popSize, 3, 3, 3);
    std::cout << "init finish" << std::endl;

    for (int i = 0; i < evalNums / popSize; i++)
    {
        population.computeFitness();
        std::cout << "Gen#" + std::to_string(i) + " : " + std::to_string(population.bestDistance)
                  << " , avg: " << std::to_string(population.fitnessSum / popSize)
                  << std::endl;

        if (population.bestDistance > bestDistance)
        {
            bestDistance = population.bestDistance;
            bestRobotTypes = population.bestRobotTypes;
        }

        writeToFile(std::to_string(bestDistance));

        population.selection();
        population.regenerate(DETERMINISTIC_CROWDING);
    }

    skeleton << bestRobotTypes;

    // for loop:
    //      compute fitness for each individual

    //      get best individual, compare with global best

    //      regenerate population
    //           roulette sus selection for parents selectParent()
    //           crossover(robot1, robot2), global function
    //           deterministic crowding
}

int main(int argc, char **argv)
{
    srand(time(NULL));

    int runtimes = strtol(argv[1], nullptr, 10);

    for (int t = 0; t < runtimes; t++)
    {
        // init file
        skeleton.open("skeleton-" + to_string(t) + ".txt");
        myfile.open("ea-" + to_string(t) + ".txt");

        ea();

        myfile.close();
        skeleton.close();
    }

    return 0;
}
