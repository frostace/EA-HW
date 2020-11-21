#pragma once

#include <vector>
#include <limits>
#include <string>
#include "Creature.cpp"
#include "Constants.h"

class Population
{
private:
public:
    std::vector<Creature> population;
    std::vector<Creature> dummies;
    std::vector<int> survivedIndices;
    int popSize;
    float bestDistance = 0;
    float fitnessSum = 0;
    std::string bestRobotTypes;

    Population(int popSize, int xdim, int ydim, int zdim) : popSize{popSize}
    {
        for (int i = 0; i < popSize; i++)
        {
            Creature robot(xdim, ydim, zdim);
            population.push_back(robot);
            Creature dummy(xdim, ydim, zdim);
            dummies.push_back(dummy);
        }
    }
    ~Population()
    {
    }

    void computeFitness()
    {
        fitnessSum = 0;
        for (auto &robot : population)
        {
            robot.settle();
            robot.measurePerformance();
            fitnessSum += robot.scaledCentroidX;
            if (robot.scaledCentroidX > bestDistance)
            {
                bestDistance = robot.scaledCentroidX;
                bestRobotTypes = robot.saveSkeleton();
            }
        }
    }

    void selection()
    {
        survivedIndices.clear();
        // roulette sus
        float stepSize = fitnessSum / popSize;
        float randomPointer = rand() / (float)RAND_MAX * stepSize;
        float pointer;
        for (int i = 0; i < popSize; i++)
        {
            // increment pointer
            pointer = randomPointer + i * stepSize;
            survivedIndices.push_back(rouletteIdx(pointer));
        }
    }

    int pickParentIdx()
    {
        return survivedIndices[rand() % popSize];
    }

    int rouletteIdx(float prob)
    {
        for (int i = 0; i < popSize; i++)
        {
            prob -= population[i].scaledCentroidX;
            if (prob < 0)
                return i;
        }
        return 0;
    }

    void regenerate(int method)
    {
        switch (method)
        {
        case NORMAL_CROSSOVER:
            regenerate_normal();
            break;
        case DETERMINISTIC_CROWDING:
            regenerate_dc();
            break;
        }
    }

    void regenerate_normal()
    {
        for (auto &robot : dummies)
        {
            // pick indices from survivedIndices randomly and make crossover
            int idxA = pickParentIdx();
            int idxB = pickParentIdx();

            // normal crossover
            robot.crossover(population[idxA], population[idxB]);

            if (rand() / (float)RAND_MAX < 0.1)
            {
                robot.mutate();
            }
        }

        // swap population with dummies
        population.swap(dummies);
    }

    void regenerate_dc()
    {
        // deal with 2 entries at a time
        for (int i = 0; i < popSize; i += 2)
        {
            int idxA = pickParentIdx();
            int idxB = pickParentIdx();
            crossover_dc(dummies[i], dummies[i + 1], population[idxA], population[idxB]);

            if (rand() / (float)RAND_MAX < 0.1)
                dummies[i].mutate();
            if (rand() / (float)RAND_MAX < 0.1)
                dummies[i + 1].mutate();
        }

        // swap population with dummies
        population.swap(dummies);
    }

    void crossover_dc(Creature &child1, Creature &child2, Creature &parent1, Creature &parent2)
    {
        std::vector<float> planeArgs = child1.genRandomArgs();
        child1.crossover(parent1, parent2, planeArgs, 1);
        child2.crossover(parent1, parent2, planeArgs, -1);

        child1.settle();
        child1.measurePerformance();
        child2.settle();
        child2.measurePerformance();

        // measure distance between 2 pairs and decide which 2 to survive
        if (distance(child1, parent1) + distance(child2, parent2) < distance(parent1, child2) + distance(parent2, child1))
        {
            if (child1.scaledCentroidX < parent1.scaledCentroidX)
                std::swap(child1, parent1);
            if (child2.scaledCentroidX < parent2.scaledCentroidX)
                std::swap(child2, parent2);
        }
        else
        {
            if (child1.scaledCentroidX < parent2.scaledCentroidX)
                std::swap(child1, parent2);
            if (child2.scaledCentroidX < parent1.scaledCentroidX)
                std::swap(child2, parent1);
        }
    }

    float distance(Creature &robot1, Creature &robot2)
    {
        // extract type vectors from robot 1 and robot 2
        std::vector<float> v1 = {};
        std::vector<float> v2 = {};

        for (int i = 0, len = robot1.cubes.size(); i < len; i++)
        {
            v1.push_back(robot1.cubes[i].currType);
            v2.push_back(robot2.cubes[i].currType);
        }
        return measureDistance(v1, v2);
    }
};