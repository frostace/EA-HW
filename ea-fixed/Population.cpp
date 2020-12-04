#pragma once

#include <vector>
#include <limits>
#include <string>
#include <cmath>
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
    float bestFitness = 0;
    float fitnessSum = 0;
    std::string bestRobotTypes;

    Population(int popSize, int xdim, int ydim, int zdim) : popSize{popSize}
    {
        for (int i = 0; i < popSize; i++)
        {
            Creature robot(xdim, ydim, zdim);
            // robot.loadSkeleton("skeleton-3-usable.txt");
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
            fitnessSum += std::max(0.0f, robot.fitness);
            // TODO: refactor with address update and save at the end instead of frequent saving
            if (robot.fitness > bestDistance)
            {
                bestDistance = robot.fitness;
                bestRobotTypes = robot.saveSkeleton();
            }
        }

        auto cpr = [&](Creature &robot1, Creature &robot2) -> bool {
            return robot1.fitness > robot2.fitness;
        };

        sort(population.begin(), population.end(), cpr);
    }

    void printAllFitness()
    {
        for (auto &robot : population)
        {
            std::cout << robot.fitness << ", ";
        }
        std::cout << std::endl;
    }

    // compute subjective fitness
    void computeFitness(Population &opponents)
    {
        // TODO: refactor this algorithm and make it compute fitness for both populations within a single loop
        fitnessSum = 0;
        bestFitness = 0;
        // opponents.fitnessSum = 0;
        // count how many individuals in opponents are worse than individual i
        int idx_j = 0;
        for (int i = 0; i < popSize; i++)
        {
            while (idx_j < popSize && opponents.population[idx_j].absFitness >= population[i].absFitness)
            {
                // TODO: edge case : 2 arrays are all zeros
                // opponents.population[idx_j].fitness = popSize - i;
                // opponents.fitnessSum += popSize - i;
                idx_j += 1;
            }
            population[i].fitness = popSize - idx_j;
            bestFitness = std::max(bestFitness, population[i].fitness);
            fitnessSum += std::max(0.0f, population[i].fitness);
        }

        // fill in remaining fitness of opponents' population
        // while (idx_j < popSize)
        // {
        //     opponents.population[idx_j].fitness = 0;
        //     idx_j += 1;
        // }
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
            prob -= population[i].fitness;
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

            if (rand() / (float)RAND_MAX < mutationRate)
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

            if (rand() / (float)RAND_MAX < mutationRate)
                dummies[i].mutate();
            if (rand() / (float)RAND_MAX < mutationRate)
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

        // std::cout << "child1" << std::endl;
        child1.settle();
        child1.measurePerformance();
        // std::cout << "child2" << std::endl;
        child2.settle();
        child2.measurePerformance();

        // measure distance between 2 pairs and decide which 2 to survive
        if (distance(child1, parent1) + distance(child2, parent2) < distance(parent1, child2) + distance(parent2, child1))
        {
            if (child1.scaledCentroidZ < parent1.scaledCentroidZ)
                std::swap(child1, parent1);
            if (child2.scaledCentroidZ < parent2.scaledCentroidZ)
                std::swap(child2, parent2);
        }
        else
        {
            if (child1.scaledCentroidZ < parent2.scaledCentroidZ)
                std::swap(child1, parent2);
            if (child2.scaledCentroidZ < parent1.scaledCentroidZ)
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

            // TODO: modified on 12-03
            // v1.push_back(robot1.cubes[i].currType);
            // v2.push_back(robot2.cubes[i].currType);
            for (auto &s : robot1.cubes[i].springs)
            {
                v1.push_back(s->b);
                v1.push_back(s->c);
            }
            for (auto &s : robot2.cubes[i].springs)
            {
                v2.push_back(s->b);
                v2.push_back(s->c);
            }
        }
        return measureDistance(v1, v2);
    }
};