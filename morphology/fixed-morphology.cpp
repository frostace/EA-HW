#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <signal.h> // raise(SIGTRAP)
#include <stdlib.h>

// Cube
#include "Cube.cpp"
#include "Creature.cpp"

using namespace std;
double xpos_old, ypos_old;
ofstream myfile, skeleton, energy;

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

int main(int argc, char **argv)
{
    srand(time(NULL));

    int runtimes = strtol(argv[1], nullptr, 10);

    for (int t = 0; t < runtimes; t++)
    {
        Creature robot(20, 1, 1);

        // init file
        energy.open("energy.txt");
        skeleton.open("skeleton.txt");
        myfile.open("random-" + to_string(t) + ".txt");

        int frameNum = 0;
        float bestDistance = 0;
        for (int i = 0; i < 100000; i++)
        {
            robot.settle();
            robot.measurePerformance();
            if (robot.scaledCentroidX > bestDistance)
            {
                bestDistance = robot.scaledCentroidX;
            }
            cout << "Iterations: " << i << ", best: " << bestDistance << endl;
            writeToFile(to_string(bestDistance) + "\n");
            robot.mutate(1);
        }

        robot.saveSkeleton(skeleton);

        energy.close();
        myfile.close();
        skeleton.close();
    }

    return 0;
}
