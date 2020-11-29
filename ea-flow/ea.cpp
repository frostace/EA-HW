#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <signal.h> // raise(SIGTRAP)
#include <stdlib.h>
#include <cmath>

// Cube
#include "Cube.cpp"
#include "Creature.cpp"
#include "Population.cpp"
#include "Constants.h"

#include "omp.h"

int iterNums = 0;

using namespace std;
ofstream myfile, skeleton, energy;

void writeToFile(string content)
{
    if (myfile.is_open())
    {
        myfile << content;
    }
}

void ea()
{
    // init population, bestDistance and bestRobotTypes
    int sizeX = 1, sizeY = 1, sizeZ = 1;
    float bestDistance = 0;
    std::string bestRobotTypes = "";

    int popSize = 30;

    Population population(popSize, sizeX, sizeY, sizeZ);
    std::cout << "init finish" << std::endl;

    for (int i = 0; i < generationNum; i++)
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

        writeToFile(std::to_string(bestDistance) + "\n");

        population.selection();
        population.regenerate(DETERMINISTIC_CROWDING);
    }

    skeleton << bestRobotTypes;
}

void ea_coevolve()
{
    // init population, bestDistance and bestRobotTypes
    int sizeX = 4, sizeY = 4, sizeZ = 4;
    float bestDistance = 0, genDistance = 0;
    std::string bestRobotTypes = "", genRobotTypes = "";

    int popSize = 16;

    Population population1(popSize, sizeX, sizeY, sizeZ);
    Population population2(popSize, sizeX, sizeY, sizeZ);
    std::cout << "init finish" << std::endl;

    for (int i = 0; i < generationNum; i++)
    {
        population1.computeFitness();
        population2.computeFitness();
        population1.computeFitness(population2);
        population2.computeFitness(population1);
        if (population1.bestDistance > population2.bestDistance)
        {
            genDistance = population1.bestDistance;
            genRobotTypes = population1.bestRobotTypes;
        }
        else
        {
            genDistance = population2.bestDistance;
            genRobotTypes = population2.bestRobotTypes;
        }
        std::string content = "Gen#" + std::to_string(i) + " : " + std::to_string(genDistance) +
                              " , d1: " + std::to_string(population1.bestDistance) +
                              " , d2: " + std::to_string(population2.bestDistance) +
                              " , f1: " + std::to_string(population1.bestFitness) +
                              " , f2: " + std::to_string(population2.bestFitness);
        std::cout << content << std::endl;
        /*
        std::cout << "Population1: " << std::endl;
        for (auto &robot : population1.population)
        {
            std::cout << robot.scaledCentroidX << ", ";
        }
        std::cout << std::endl;
        std::cout << "Population2: " << std::endl;
        for (auto &robot : population2.population)
        {
            std::cout << robot.scaledCentroidX << ", ";
        }
        std::cout << std::endl;
        */

        if (genDistance > bestDistance)
        {
            bestDistance = genDistance;
            bestRobotTypes = genRobotTypes;
        }

        writeToFile(content + "\n");

        population1.selection();
        population2.selection();
        population1.regenerate(DETERMINISTIC_CROWDING);
        population2.regenerate(DETERMINISTIC_CROWDING);
    }

    skeleton << bestRobotTypes;
}

void ea_flow()
{
    // init population, bestDistance and bestRobotTypes
    int sizeX = 4, sizeY = 4, sizeZ = 4;
    float bestDistance = 0, genDistance = 0;
    std::string bestRobotTypes = "", genRobotTypes = "";

    int popSize = 16;

    Population population1(popSize, sizeX, sizeY, sizeZ);
    Population population2(popSize, sizeX, sizeY, sizeZ);
    std::cout << "init finish" << std::endl;

    for (int i = 0; i < generationNum; i++)
    {
        population1.computeFitness();
        population2.computeFitness();
        population1.computeFitness(population2);
        population2.computeFitness(population1);
        if (population1.bestDistance > population2.bestDistance)
        {
            genDistance = population1.bestDistance;
            genRobotTypes = population1.bestRobotTypes;
        }
        else
        {
            genDistance = population2.bestDistance;
            genRobotTypes = population2.bestRobotTypes;
        }
        std::string content = "Gen#" + std::to_string(i) + " : " + std::to_string(genDistance) +
                              " , d1: " + std::to_string(population1.bestDistance) +
                              " , d2: " + std::to_string(population2.bestDistance) +
                              " , f1: " + std::to_string(population1.bestFitness) +
                              " , f2: " + std::to_string(population2.bestFitness);
        std::cout << content << std::endl;
        /*
        std::cout << "Population1: " << std::endl;
        for (auto &robot : population1.population)
        {
            std::cout << robot.scaledCentroidX << ", ";
        }
        std::cout << std::endl;
        std::cout << "Population2: " << std::endl;
        for (auto &robot : population2.population)
        {
            std::cout << robot.scaledCentroidX << ", ";
        }
        std::cout << std::endl;
        */

        if (genDistance > bestDistance)
        {
            bestDistance = genDistance;
            bestRobotTypes = genRobotTypes;
        }

        writeToFile(content + "\n");

        population1.selection();
        population2.selection();
        population1.regenerate(DETERMINISTIC_CROWDING);
        population2.regenerate(DETERMINISTIC_CROWDING);

        std::swap(population1.population[randint(popSize)], population2.population[randint(popSize)]);
    }

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
        myfile.open("ea-" + to_string(t) + ".txt");

        // ea_coevolve();
        ea_flow();
        // ea();

        myfile.close();
        skeleton.close();
    }

    return 0;
}