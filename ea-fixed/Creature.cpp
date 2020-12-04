#pragma once
#include <iostream>
#include <vector>
#include <string>
#include <unordered_set>
#include <unordered_map>
#include <iomanip>
#include <sstream>
#include <ctime>
#include <chrono>
#include <algorithm>
#include <memory>
#include <cmath>
#include "stdlib.h"
#include "stdio.h"

#include "Cube.cpp"
#include "Constants.h"

#include "omp.h" // parallel

extern int iterNums;

class Creature
{
private:
    int idim, jdim, kdim;
    bool settled = false;
    int windMag = rand() % 3;
    std::unordered_map<std::string, std::vector<float>> settledPos = {}; // map mass hashing to mass settled positions

    // maintain indices of masses of initialized springs, e.g. "i1,j1,k1&i2,j2,k2"
    std::unordered_map<std::string, std::shared_ptr<Spring>> springs = {};
    // init masses with corresponding positions within the 3d grid
    std::unordered_map<std::string, std::shared_ptr<Mass>> masses = {};
    std::vector<std::vector<std::vector<int>>> cubeTypes = {};
    std::unordered_map<int, std::vector<float>> plane_arg_map;

    bool classifier(std::vector<float> &plane_args, float x, float y, float z)
    {
        float a = plane_args[0];
        float b = plane_args[1];
        float c = plane_args[2];
        float d = plane_args[3];
        float e = plane_args[4];
        // take use of bipartitioning of a hyperplane + / -
        return a * (x - c) + b * (y - d) - (z - e) > 0;
    }
    bool classifier(std::vector<float> &plane_args, std::vector<float> coords)
    {
        float a = plane_args[0];
        float b = plane_args[1];
        float c = plane_args[2];
        float d = plane_args[3];
        float e = plane_args[4];
        // take use of bipartitioning of a hyperplane + / -
        return a * (coords[0] - c) + b * (coords[1] - d) - (coords[2] - e) > 0;
    }
    bool classifier(std::vector<float> &plane_args, std::vector<float> coords, int polarity)
    {
        float a = plane_args[0];
        float b = plane_args[1];
        float c = plane_args[2];
        float d = plane_args[3];
        float e = plane_args[4];
        // take use of bipartitioning of a hyperplane + / -
        return polarity * (a * (coords[0] - c) + b * (coords[1] - d) - (coords[2] - e)) > 0;
    }

    int assignType(float i, float j, float k)
    {
        for (int p = 0; p < 4; p++)
        {
            if (classifier(plane_arg_map[p], i, j, k) > 0)
                return cube_types[p];
        }
        return cube_types[4];
    }

    int hardCodeType(int i, int j, int k)
    {
        // TODO: modified on 11-25
        // if (k == 0 && (i == 1 || j == 1))
        //     return 4;
        // TODO: modified on 12-03
        // return rand() % 4;
        return cubeTypes[i][j][k];
    }

    void initCubeTypes()
    {
        cubeTypes.clear();
        for (int i = 0; i < idim; i++)
        {
            cubeTypes.push_back({});
            for (int j = 0; j < jdim; j++)
            {
                cubeTypes[i].push_back({});
                for (int k = 0; k < kdim; k++)
                {
                    cubeTypes[i][j].push_back(-1);
                }
            }
        }

        // init random 4 initial points
        std::vector<std::vector<int>> points = genInitialPoints(4, idim, jdim, kdim);
        // fill cubes around initial points
        for (int colorIdx = 0; colorIdx < points.size(); colorIdx++)
        {
            dfs_fill(points[colorIdx], 0, colorIdx, idim, jdim, kdim, cubeTypes);
        }

        //  fill remaining cubes
        for (int i = 0; i < idim; i++)
        {
            for (int j = 0; j < jdim; j++)
            {
                for (int k = 0; k < kdim; k++)
                {
                    if (cubeTypes[i][j][k] == -1)
                        cubeTypes[i][j][k] = randint(5);
                }
            }
        }
    }

    std::string hashing(float x, float y, float z)
    {
        std::stringstream stream;
        stream << std::fixed << std::setprecision(1) << x;
        stream << ",";
        stream << std::fixed << std::setprecision(1) << y;
        stream << ",";
        stream << std::fixed << std::setprecision(1) << z;
        return stream.str();
    }
    std::string hashing(int x1, int y1, int z1, int x2, int y2, int z2)
    {
        std::string str1 = std::to_string(x1) + "," + std::to_string(y1) + "," + std::to_string(z1);
        std::string str2 = std::to_string(x2) + "," + std::to_string(y2) + "," + std::to_string(z2);
        if (str1 < str2)
            return str1 + "&" + str2;
        return str2 + "&" + str1;
    }

public:
    Cube core;
    bool wind = false;
    int latestMutatedCubeIdx;
    int latestMutatedSpringIdx;
    int latestMutationType;
    std::vector<Cube> cubes = {};
    float totalEk;
    float totalEp;
    float centroidX = 0;
    float centroidY = 0;
    float centroidZ = 0;
    float centroidZMax = 0;
    float scaledCentroidX = 0;
    float scaledCentroidY = 0;
    float scaledCentroidZ = 0;
    float scaledCentroidXOffset = 0;
    float scaledCentroidYOffset = 0;
    float scaledCentroidZOffset = 0;
    float XScale = 0;
    float fitness = 0;
    float absFitness = 0;

    // constructor for n * n * n cubes
    Creature(int n) : idim{n}, jdim{n}, kdim{n}
    {
        initCreature(n, n, n, 0, 0, 0);
    }
    // constructor for n * n * n cubes
    Creature(int n, int xOffset, int yOffset, int zOffset) : idim{n}, jdim{n}, kdim{n}
    {
        initCreature(n, n, n, xOffset, yOffset, zOffset);
    }
    // constructor for imax * jmax * kmax cubes
    Creature(int imax, int jmax, int kmax) : idim{imax}, jdim{jmax}, kdim{kmax}
    {
        initCreature(imax, jmax, kmax, 0, 0, 0);
    }
    // constructor for imax * jmax * kmax cubes
    Creature(int imax, int jmax, int kmax, int xOffset, int yOffset, int zOffset) : idim{imax}, jdim{jmax}, kdim{kmax}
    {
        initCreature(imax, jmax, kmax, xOffset, yOffset, zOffset);
    }
    ~Creature() {}

    void initCreature(int imax, int jmax, int kmax, int xOffset, int yOffset, int zOffset)
    {
        initCubeTypes();
        XScale = std::max(std::max(imax, jmax), kmax);

        for (int i = 0; i <= imax; i++)
        {
            for (int j = 0; j <= jmax; j++)
            {
                for (int k = 0; k <= kmax; k++)
                {
                    std::shared_ptr<Mass> m(new Mass(i + xOffset, j + yOffset, k + zOffset));
                    masses[hashing(i, j, k)] = m;
                }
            }
        }
        // init springs with indices of masses within the 3d grid
        for (int i = 0; i < imax; i++)
        {
            for (int j = 0; j < jmax; j++)
            {
                for (int k = 0; k < kmax; k++)
                {
                    int type = hardCodeType(i, j, k);
                    std::vector<std::shared_ptr<Mass>> cubeMasses = {};
                    std::vector<std::shared_ptr<Spring>> cubeSprings = {};
                    // iterate through all pairs of masses to assign masses
                    for (int d1 = 0; d1 < 8; d1++)
                    {
                        int x1 = i + cube8dirs[d1][0];
                        int y1 = j + cube8dirs[d1][1];
                        int z1 = k + cube8dirs[d1][2];
                        cubeMasses.push_back(masses[hashing(x1, y1, z1)]);
                    }

                    // iterate through all pairs of masses to init springs
                    for (int d1 = 0; d1 < 7; d1++)
                    {
                        for (int d2 = d1 + 1; d2 < 8; d2++)
                        {
                            int x1 = i + cube8dirs[d1][0];
                            int y1 = j + cube8dirs[d1][1];
                            int z1 = k + cube8dirs[d1][2];
                            int x2 = i + cube8dirs[d2][0];
                            int y2 = j + cube8dirs[d2][1];
                            int z2 = k + cube8dirs[d2][2];
                            std::string springIdx = hashing(x1, y1, z1, x2, y2, z2);
                            // spring reference already exist
                            if (springs.find(springIdx) != springs.end())
                            {
                                cubeSprings.push_back(springs[springIdx]);
                                continue;
                            }
                            std::shared_ptr<Spring> s(new Spring(type, masses[hashing(x1, y1, z1)], masses[hashing(x2, y2, z2)]));
                            springs[springIdx] = s;
                            cubeSprings.push_back(s);
                        }
                    }
                    Cube c(type, cubeMasses, cubeSprings);
                    cubes.push_back(c);
                }
            }
        }
    }
    void iterate()
    {
        iterNums += 1;
        auto start = std::chrono::system_clock::now();
        totalEk = 0;
        totalEp = 0;
        float centroidXTotal = 0;
        float centroidYTotal = 0;
        float centroidZTotal = 0;
        for (auto &cube : cubes)
        {
            // if (wind)
            //     cube.addWind(windMag);
            cube.iterate(true);
        }
        for (auto &cube : cubes)
        {
            cube.integrate();
            totalEk += cube.totalEk;
            totalEp += cube.totalEp;
        }

        for (auto &cube : cubes)
        {
            cube.clearComputed();
            // TODO: if cube type is air, do I want to calculate its centroid location ?
            // for now, I think this helps with eliminating torn apart robots
            centroidXTotal += cube.centroidX;
            centroidYTotal += cube.centroidY;
            centroidZTotal += cube.centroidZ;
        }
        centroidX = centroidXTotal / cubes.size();
        centroidY = centroidYTotal / cubes.size();
        centroidZ = centroidZTotal / cubes.size();
        centroidZMax = std::max(centroidZMax, centroidZ);
        // TODO: modified on 12-03
        scaledCentroidX = centroidX / XScale - scaledCentroidXOffset;
        scaledCentroidY = centroidY / XScale - scaledCentroidYOffset;
        scaledCentroidZ = std::max(0.0f, centroidZ / XScale - scaledCentroidZOffset);
        auto end = std::chrono::system_clock::now();
        std::chrono::duration<double> elapsed_seconds = end - start;
        // std::cout << to_string(28 / elapsed_seconds.count()) << std::endl;
    }

    void iterate(bool breathing)
    {
        if (breathing)
            iterNums += 1;
        auto start = std::chrono::system_clock::now();
        totalEk = 0;
        totalEp = 0;
        float centroidXTotal = 0;
        float centroidYTotal = 0;
        float centroidZTotal = 0;
        for (auto &cube : cubes)
        {
            cube.iterate(breathing);
        }
        for (auto &cube : cubes)
        {
            cube.integrate();
            totalEk += cube.totalEk;
            totalEp += cube.totalEp;
        }
        for (auto &cube : cubes)
        {
            cube.clearComputed();
            centroidXTotal += cube.centroidX;
            centroidYTotal += cube.centroidY;
            centroidZTotal += cube.centroidZ;
        }
        centroidX = centroidXTotal / cubes.size();
        centroidY = centroidYTotal / cubes.size();
        centroidZ = centroidZTotal / cubes.size();
        scaledCentroidX = std::max(0.0f, centroidX / XScale - scaledCentroidXOffset);
        scaledCentroidY = std::max(0.0f, centroidY / XScale - scaledCentroidYOffset);
        scaledCentroidZ = std::max(0.0f, centroidZ / XScale - scaledCentroidZOffset);
        auto end = std::chrono::system_clock::now();
        std::chrono::duration<double> elapsed_seconds = end - start;
        // std::cout << to_string(28 / elapsed_seconds.count()) << std::endl;
    }

    void mutate()
    {
        // donot mutate air to other materials
        latestMutatedCubeIdx = rand() % cubes.size();
        if (cubes[latestMutatedCubeIdx].currType == 4)
        {
            latestMutationType = 2;
            return;
        }
        // mutate a single cube
        float randomP = rand() / (float)RAND_MAX;
        if (randomP < 0.9) // 0.8 * 0.15 = 0.12
        {
            // update a random spring arguments
            latestMutationType = 0;
            latestMutatedSpringIdx = rand() % 28;
            cubes[latestMutatedCubeIdx].mutate(latestMutatedSpringIdx);
        }
        else // 0.2 * 0.15 = 0.03
        {
            // switch cube type
            latestMutationType = 1;
            cubes[latestMutatedCubeIdx].mutate();
        }

        settled = false;
    }

    void recover()
    {
        if (latestMutationType == 0) // spring param
        {
            cubes[latestMutatedCubeIdx].recover(latestMutatedSpringIdx);
        }
        else if (latestMutationType == 1) // switch cube type
        {
            cubes[latestMutatedCubeIdx].recover();
        }
    }

    void reborn()
    {
        cubes.clear();
        // init springs with indices of masses within the 3d grid
        for (int i = 0; i < idim; i++)
        {
            for (int j = 0; j < jdim; j++)
            {
                for (int k = 0; k < kdim; k++)
                {
                    // int type = assignType(i, j, k);
                    int type = rand() % 4;
                    std::vector<std::shared_ptr<Mass>> cubeMasses = {};
                    std::vector<std::shared_ptr<Spring>> cubeSprings = {};
                    // iterate through all pairs of masses to assign masses
                    for (int d1 = 0; d1 < 8; d1++)
                    {
                        int x1 = i + cube8dirs[d1][0];
                        int y1 = j + cube8dirs[d1][1];
                        int z1 = k + cube8dirs[d1][2];
                        cubeMasses.push_back(masses[hashing(x1, y1, z1)]);
                    }

                    // iterate through all pairs of masses to init springs
                    for (int d1 = 0; d1 < 7; d1++)
                    {
                        for (int d2 = d1 + 1; d2 < 8; d2++)
                        {
                            int x1 = i + cube8dirs[d1][0];
                            int y1 = j + cube8dirs[d1][1];
                            int z1 = k + cube8dirs[d1][2];
                            int x2 = i + cube8dirs[d2][0];
                            int y2 = j + cube8dirs[d2][1];
                            int z2 = k + cube8dirs[d2][2];
                            std::string springIdx = hashing(x1, y1, z1, x2, y2, z2);
                            // spring reference already exist
                            if (springs.find(springIdx) != springs.end())
                            {
                                cubeSprings.push_back(springs[springIdx]);
                                continue;
                            }
                            std::shared_ptr<Spring> s(new Spring(type, masses[hashing(x1, y1, z1)], masses[hashing(x2, y2, z2)]));
                            springs[springIdx] = s;
                            cubeSprings.push_back(s);
                        }
                    }
                    Cube c(type, cubeMasses, cubeSprings);
                    cubes.push_back(c);
                }
            }
        }

        settled = false;
    }

    void crossover(Creature &parentA, Creature &parentB)
    {
        // randomize a hyperplane to bipartition the robot layout
        std::vector<float> planeArgs = genRandomArgs();

        // fill one part of spring params from parentA
        // fill the other part of spring params from parentB
        // classify by center of the spring
        for (auto &it : springs)
        {
            std::string hashed = it.first; // 1,2,3&2,3,4
            if (classifier(planeArgs, decodeHashing(hashed)))
            {
                it.second->k = parentA.springs[hashed]->k;
                it.second->b = parentA.springs[hashed]->b;
                it.second->c = parentA.springs[hashed]->c;
            }
            else
            {
                it.second->k = parentB.springs[hashed]->k;
                it.second->b = parentB.springs[hashed]->b;
                it.second->c = parentB.springs[hashed]->c;
            }
        }
        /*
        // classify by cube
        for (int i = 0; i < cubes.size(); i++)
        {
            if (classifier(planeArgs, cubes[i].masses[0]->pos))
            {
                // switch type of cube to type of parentA.cubes[i]
                cubes[i].mutateType(parentA.cubes[i].currType);
            }
            else
            {
                // switch type of cube to type of parentB.cubes[i]
                cubes[i].mutateType(parentB.cubes[i].currType);
            }
        }
        */

        settled = false;
    }

    void crossover(Creature &parentA, Creature &parentB, std::vector<float> planeArgs, int polarity)
    {
        // fill one part of spring params from parentA
        // fill the other part of spring params from parentB
        // classify by center of the spring
        for (auto &it : springs)
        {
            std::string hashed = it.first; // 1,2,3&2,3,4
            if (classifier(planeArgs, decodeHashing(hashed), polarity))
            {
                it.second->k = parentA.springs[hashed]->k;
                it.second->b = parentA.springs[hashed]->b;
                it.second->c = parentA.springs[hashed]->c;
            }
            else
            {
                it.second->k = parentB.springs[hashed]->k;
                it.second->b = parentB.springs[hashed]->b;
                it.second->c = parentB.springs[hashed]->c;
            }
        }
        // classify by cube
        /*
        for (int i = 0; i < cubes.size(); i++)
        {
            if (classifier(planeArgs, cubes[i].masses[0]->pos, polarity))
            {
                // switch type of cube to type of parentA.cubes[i]
                cubes[i].mutateType(parentA.cubes[i].currType);
            }
            else
            {
                // switch type of cube to type of parentB.cubes[i]
                cubes[i].mutateType(parentB.cubes[i].currType);
            }
        }
        */

        settled = false;
    }

    std::vector<float> decodeHashing(std::string hashed) // 1,2,3&2,3,4
    {
        auto i1_pos = 0;
        auto j1_pos = hashed.find(",", i1_pos) + 1;
        auto k1_pos = hashed.find(",", j1_pos) + 1;

        auto i2_pos = hashed.find("&", k1_pos) + 1;
        auto j2_pos = hashed.find(",", i2_pos) + 1;
        auto k2_pos = hashed.find(",", j2_pos) + 1;

        int i1 = std::stoi(hashed.substr(i1_pos, j1_pos - i1_pos - 1));
        int j1 = std::stoi(hashed.substr(j1_pos, k1_pos - j1_pos - 1));
        int k1 = std::stoi(hashed.substr(k1_pos, i2_pos - k1_pos - 1));

        int i2 = std::stoi(hashed.substr(i2_pos, j2_pos - i2_pos - 1));
        int j2 = std::stoi(hashed.substr(j2_pos, k2_pos - j2_pos - 1));
        int k2 = std::stoi(hashed.substr(k2_pos));

        return {(float)(i1 + i2) / 2.0f, (float)(j1 + j2) / 2.0f, (float)(k1 + k2) / 2.0f};
    }

    void saveSkeleton(std::ofstream &file)
    {
        if (file.is_open())
        {
            std::string content = "";
            // write cube types in line
            for (auto &cube : cubes)
            {
                content += std::to_string(cube.currType) + ", ";
            }
            content += "\n";
            for (auto &it : springs)
            {
                content += it.first + ": k=" + std::to_string(it.second->k) + ", b=" + std::to_string(it.second->b) + ", c=" + std::to_string(it.second->c) + "\n";
            }
            file << content;
        }
    }

    std::string saveSkeleton()
    {
        std::string content = "";
        // write cube types in line
        for (auto &cube : cubes)
        {
            content += std::to_string(cube.currType) + ", ";
        }
        content += "\n";
        for (auto &it : springs)
        {
            content += it.first + ": k=" + std::to_string(it.second->k) + ", b=" + std::to_string(it.second->b) + ", c=" + std::to_string(it.second->c) + "\n";
        }
        return content;
    }

    std::vector<int> saveTypes()
    {
        std::vector<int> output = {};
        for (auto &cube : cubes)
        {
            output.push_back(cube.currType);
        }

        return output;
    }

    void loadSkeleton(const std::string &filepath)
    {
        std::ifstream stream(filepath);

        std::string line;
        std::string delimiter = ", ";

        // load cube types
        getline(stream, line);
        auto start = 0U;
        auto end = line.find(delimiter);
        int cubeIdx = 0;
        while (end != std::string::npos)
        {
            int type = std::stof(line.substr(start, end - start));
            cubes[cubeIdx++].overrideType(type);
            start = end + delimiter.length();
            end = line.find(delimiter, start);
        }

        // int type = std::stof(line.substr(start, end));
        // cubes[cubeIdx].currType = type;
        // cubes[cubeIdx].fillArrWithColor(cubes[cubeIdx].color, cube_colors[type]);
        // cubeIdx += 1;

        // load k, b, c
        while (getline(stream, line))
        {
            size_t k_pos = line.find("k=") + 2;
            size_t b_pos = line.find("b=") + 2;
            size_t c_pos = line.find("c=") + 2;
            std::string hashedIdx = line.substr(0, k_pos - 4);
            float k_val = std::stof(line.substr(k_pos, b_pos - k_pos - 4));
            float b_val = std::stof(line.substr(b_pos, c_pos - b_pos - 4));
            float c_val = std::stof(line.substr(c_pos));

            // assign set of [k,b,c] received from skeleton file
            springs[hashedIdx]->k = k_val;
            springs[hashedIdx]->b = b_val;
            springs[hashedIdx]->c = c_val;
        }
    }

    void settle()
    {
        iterNums = 0;
        // check if have computed settled positions
        // TODO: attempting to remove settled status record here since we don't reuse settle status in real regenerations
        if (settled)
        {
            // pass computed settled positions to masses and return
            for (auto &it : masses)
            {
                it.second->pos = settledPos[it.first]; // assign settled positions by order of insertion
                it.second->clearMotion();
            }
            scaledCentroidX = 0;
            return;
        }

        for (auto &it : masses)
        {
            it.second->clearMotion();
        }

        // settled = true;

        // iterate without breathing
        int frameNum = 0;
        // Loop for several iterations and measure X distance at the end
        while (frameNum < 20) // .1 cycles
        {
            frameNum += 1;
            iterate(false);
        }
        // TODO: modified on 12-01
        // while (frameNum < 450) // 8 cycles: 250 ~ 450
        // {
        //     frameNum += 1;
        //     iterate();
        // }
        // record settled positions
        for (auto &it : masses)
        {
            settledPos[it.first] = it.second->pos;
            // TODO: modified on 11-25
            // it.second->clearMotion();
        }
        scaledCentroidXOffset = centroidX / XScale;
        scaledCentroidYOffset = centroidY / XScale;
        scaledCentroidZOffset = centroidZ / XScale;
        scaledCentroidX = 0;
    }

    void measurePerformance()
    {
        int iterNums = 0;
        int frameNum = 0;
        // Loop for several iterations and measure X distance at the end
        while (frameNum < 400) // 2 cycles
        {
            frameNum += 1;
            iterate();
        }
        fitness = std::sqrt(scaledCentroidX * scaledCentroidX + scaledCentroidY * scaledCentroidY);
        // TODO: modified on 12-03
        // penetrate too high Z coord
        if (scaledCentroidZ > 0.5)
        {
            fitness = 0;
        }
        absFitness = fitness;
    }

    std::vector<float> genRandomArgs()
    {
        std::vector<float> output = {};
        for (int i = 0; i < 2; i++)
        {
            output.push_back((0.5 - rand() / (float)RAND_MAX) * 100);
        }
        output.push_back(rand() / (float)RAND_MAX * idim);
        output.push_back(rand() / (float)RAND_MAX * jdim);
        output.push_back(rand() / (float)RAND_MAX * kdim);
        return output;
    }

    void printInfo()
    {
        // for (auto &cube : cubes)
        // {
        //     for (int i = 0; i < 8; i++)
        //     {
        //         std::cout << i << ": " << &cube.masses[i] << std::endl;
        //     }
        // }
        for (auto &cube : cubes)
        {
            std::cout << "cube type: " << cube.currType << std::endl;
            // for (int idx = 0; idx < 8; idx++)
            // {
            //     std::cout << "idx: " << idx << std::endl;
            //     cube.masses[idx]->printInfo();
            // }
            for (int idx = 0; idx < 28; idx++)
            {
                cube.springs[idx]->printInfo();
            }
            std::cout << "============" << std::endl;
        }
    }

    // print kinetic energy and potential energy
    void saveEnergyTo(std::ofstream &myfile)
    {
        std::string content = std::to_string(totalEk) + ", " + std::to_string(totalEp) + ", " + std::to_string(totalEp + totalEk) + "\n";
        if (myfile.is_open())
        {
            myfile << content;
        }
    }
};