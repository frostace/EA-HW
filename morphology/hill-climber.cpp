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

void hill_climber()
{
    int sizeX = 3, sizeY = 3, sizeZ = 3;
    float bestDistance = 0;
    // std::vector<int> bestRobotTypes = {};
    Creature bestRobot(sizeX, sizeY, sizeZ);

    std::cout << "init finish" << std::endl;
#pragma omp parallel
    {
        // TODO: refactor this part to be sequential
        int patrolNums = omp_get_num_threads(); // number of parallel climbing patrols

        std::vector<Creature> patrols = {};
        std::vector<int> stuckIterNums = {};
        int winnerIdx = 0;

        for (int i = 0; i < patrolNums; i++)
        {
            Creature robot(sizeX, sizeY, sizeZ);
            robot.settle();
            robot.measurePerformance();

            if (robot.scaledCentroidX > bestDistance)
            {
                winnerIdx = 0;
                bestDistance = robot.scaledCentroidX;
            }
            patrols.push_back(robot);
            stuckIterNums.push_back(0);
        }

        std::cout << "patrols standby: " << patrols.size() << std::endl;

        // fill bestRobotTypes
        // bestRobotTypes = patrols[winnerIdx].saveTypes();
        bestRobot = patrols[winnerIdx];

#pragma omp for
        for (int i = 0; i < 10000; i++)
        {
            int id, niters;
            id = omp_get_thread_num();

            // mutate patrol in this thread
            float prevDistance = patrols[id].scaledCentroidX;
            patrols[id].mutate();

            // evaluate performance
            patrols[id].settle();
            patrols[id].measurePerformance();

            // if not better, load spring params back to original patrol => use cache to maintain last step params
            // equal is acceptable, since it could be mutating air
            if (patrols[id].scaledCentroidX < prevDistance)
            {
                patrols[id].recover();
                stuckIterNums[id] += 1;
            }
            else
            {
                stuckIterNums[id] = 0;
            }

            // compare with global record and update record if better
            if (patrols[id].scaledCentroidX > bestDistance)
            {
                bestDistance = patrols[id].scaledCentroidX;
                // bestRobotTypes = patrols[id].saveTypes();
                bestRobot = patrols[id];
            }

            // if stuck times > 1000, refresh patrol
            if (stuckIterNums[id] > 100)
            {
                patrols[id].reborn();
            }

            // print best distance in terminal
            std::cout << to_string(bestDistance) << std::endl;
            writeToFile(to_string(bestDistance) + "\n");
        }
    }

    // write best robot distance and spring types to file
    bestRobot.saveSkeleton(skeleton);
}

int main(int argc, char **argv)
{
    srand(time(NULL));

    int runtimes = strtol(argv[1], nullptr, 10);

    for (int t = 0; t < runtimes; t++)
    {
        // init file
        skeleton.open("skeleton-" + to_string(t) + ".txt");
        myfile.open("hill-" + to_string(t) + ".txt");

        hill_climber();

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
