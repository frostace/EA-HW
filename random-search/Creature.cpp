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
#include "stdlib.h"
#include "stdio.h"
#include <memory>

#include "Cube.cpp"
#include "Constants.h"

#include "omp.h" // parallel

extern int iterNums;

class Creature
{
private:
    int idim, jdim, kdim;
    bool settled = false;
    std::unordered_map<std::string, std::vector<float>> settledPos = {}; // map mass hashing to mass settled positions
    std::unordered_set<std::string> visited = {};                        // deprecated

    // maintain indices of masses of initialized springs, e.g. "i1,j1,k1&i2,j2,k2"
    std::unordered_map<std::string, std::shared_ptr<Spring>> springs = {};
    // init masses with corresponding positions within the 3d grid
    std::unordered_map<std::string, std::shared_ptr<Mass>> masses = {};
    std::vector<std::vector<int>> directions = {
        {0, 0, 1},  // up
        {1, 0, 0},  // front
        {0, -1, 0}, // left
        {0, 1, 0},  // right
        {-1, 0, 0}, // back
    };
    std::vector<std::vector<int>> cube8dirs = {
        {0, 0, 0},
        {0, 1, 0},
        {1, 1, 0},
        {1, 0, 0},
        {0, 0, 1},
        {0, 1, 1},
        {1, 1, 1},
        {1, 0, 1},
    };

    std::vector<float> genRandomArgs()
    {
        std::vector<float> output = {};
        for (int i = 0; i < 2; i++)
        {
            output.push_back((0.5 - rand() / (float)RAND_MAX) * 100);
        }
        output.push_back(rand() / (float)RAND_MAX * 8);
        output.push_back(rand() / (float)RAND_MAX * 8);
        return output;
    }

    std::unordered_map<int, std::vector<float>> plane_arg_map;

    bool classifier(std::vector<float> &plane_args, float x, float y, float z)
    {
        float a = plane_args[0];
        float b = plane_args[1];
        float c = plane_args[2];
        float d = plane_args[3];
        // take use of bipartitioning of a hyperplane + / -
        return a * (x - c) + b * (y - d) - z > 0;
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

    int hardCodeType(float i, float j, float k)
    {
        if (k == 0)
        {
            if (i == 1 || j == 1)
            {
                return 4;
            }
        }
        return rand() % 4;
    }

    void initPlaneArgMap()
    {
        plane_arg_map[0] = genRandomArgs();
        plane_arg_map[1] = genRandomArgs();
        plane_arg_map[2] = genRandomArgs();
        plane_arg_map[3] = genRandomArgs();
    }

    void dfs(Cube &cube, int depth, float cx, float cy, float cz)
    {
        // base case
        if (depth >= 8 || false)
            return;

        // randomly pick 1 direction to grow
        int initIdx = rand() % 3;
        for (int idx = initIdx, delta = 0; delta < 3; idx = (initIdx + delta++) % 5)
        {
            std::vector<int> dir = directions[idx];
            // std::cout << "dir: " << idx << std::endl;
            float cx_next = cx + dir[0];
            float cy_next = cy + dir[1];
            float cz_next = cz + dir[2];
            if (visited.find(hashing(cx_next, cy_next, cz_next)) != visited.end())
                continue;

            visited.insert(hashing(cx_next, cy_next, cz_next));

            // init new cube with shared reference of some masses and springs
            Cube newCube(cx_next, cy_next, cz_next,
                         cube.masses[refMassIndices[idx][0]],
                         cube.masses[refMassIndices[idx][1]],
                         cube.masses[refMassIndices[idx][2]],
                         cube.masses[refMassIndices[idx][3]],
                         refMassIndices[5 - idx][0],
                         refMassIndices[5 - idx][1],
                         refMassIndices[5 - idx][2],
                         refMassIndices[5 - idx][3],
                         cube.springs[refSpringIndices[idx][0]],
                         cube.springs[refSpringIndices[idx][1]],
                         cube.springs[refSpringIndices[idx][2]],
                         cube.springs[refSpringIndices[idx][3]],
                         cube.springs[refSpringIndices[idx][4]],
                         cube.springs[refSpringIndices[idx][5]],
                         refSpringIndices[5 - idx][0],
                         refSpringIndices[5 - idx][1],
                         refSpringIndices[5 - idx][2],
                         refSpringIndices[5 - idx][3],
                         refSpringIndices[5 - idx][4],
                         refSpringIndices[5 - idx][5]);

            cubes.push_back(newCube);

            // recurse
            dfs(newCube, depth + 1, cx_next, cy_next, cz_next);
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

    void create(Cube &cube, int depth, float cx, float cy, float cz)
    {
        dfs(cube, depth, cx, cy, cz);
    }

public:
    Cube core;
    int latestMutatedCubeIdx;
    int latestMutatedSpringIdx;
    int latestMutationType;
    std::vector<Cube> cubes = {};
    float totalEk;
    float totalEp;
    float centroidX = 0;
    float scaledCentroidX = 0;
    float scaledCentroidXOffset = 0;
    float XScale = 0;

    Creature(float x, float y, float z) : core(x, y, z)
    {
        // init plane_arg_map
        initPlaneArgMap();
        cubes.push_back(core);
        visited.insert(hashing(x, y, z));

        create(core, 0, x, y, z);
    }
    // constructor for n * n * n cubes
    Creature(int n) : idim{n}, jdim{n}, kdim{n}
    {
        initCreature(n, n, n, 0);
    }
    // constructor for imax * jmax * kmax cubes
    Creature(int imax, int jmax, int kmax) : idim{imax}, jdim{jmax}, kdim{kmax}
    {
        initCreature(imax, jmax, kmax, 0);
    }
    ~Creature() {}

    void initCreature(int imax, int jmax, int kmax, int hOffset)
    {
        XScale = std::max(std::max(imax, jmax), kmax) + 1;
        // init plane_arg_map
        initPlaneArgMap();

        for (int i = 0; i <= imax; i++)
        {
            for (int j = 0; j <= jmax; j++)
            {
                for (int k = 0; k <= kmax; k++)
                {
                    std::shared_ptr<Mass> m(new Mass(i, j, k + hOffset));
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
                    // int type = assignType(i, j, k);
                    // int type = rand() % 4;
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

                    // std::cout << "==========" << std::endl;
                    // std::cout << "cube masses: " << cubeMasses.size() << std::endl;
                    // std::cout << "cube springs: " << cubeSprings.size() << std::endl;
                    // for (auto &m : cubeMasses)
                    // {
                    //     std::cout << "mass idx: ";
                    //     puts(m->idx);
                    // }
                    // for (auto &s : cubeSprings)
                    // {
                    //     std::cout << "spring idx: ";
                    //     puts(s->idx);
                    // }
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
        for (auto &cube : cubes)
        {
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
            centroidXTotal += cube.centroidX;
        }
        centroidX = centroidXTotal / cubes.size();
        scaledCentroidX = centroidX / XScale - scaledCentroidXOffset;
        auto end = std::chrono::system_clock::now();
        std::chrono::duration<double> elapsed_seconds = end - start;
        // std::cout << to_string(28 / elapsed_seconds.count()) << std::endl;
    }

    void iterate(bool breathing)
    {
        auto start = std::chrono::system_clock::now();
        totalEk = 0;
        totalEp = 0;
        float centroidXTotal = 0;
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
        }
        centroidX = centroidXTotal / cubes.size();
        scaledCentroidX = centroidX / XScale - scaledCentroidXOffset;
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
                    int type = rand() % 5;
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

    void saveSkeleton(std::ofstream &file)
    {
        if (file.is_open())
        {
            std::string content = "";
            // write cube types in line
            // TODO: to be tested
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
        // TODO: to be tested
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
        // check if have computed settled positions
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

        settled = true;

        // iterate without breathing
        int frameNum = 0;
        // Loop for several iterations and measure X distance at the end
        while (frameNum < 100)
        {
            frameNum += 1;
            iterate(false);
            // std::cout << "i: " << frameNum << ", cube pos: " << scaledCentroidX << std::endl;
        }
        // record settled positions
        for (auto &it : masses)
        {
            settledPos[it.first] = it.second->pos;
            it.second->clearMotion();
        }
        scaledCentroidXOffset = centroidX / XScale;
        scaledCentroidX = 0;
    }

    void measurePerformance()
    {
        int frameNum = 0;
        // Loop for several iterations and measure X distance at the end
        while (frameNum < 250)
        {
            frameNum += 1;
            iterate();
        }
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
            for (int idx = 0; idx < 8; idx++)
            {
                std::cout << "idx: " << idx << std::endl;
                cube.masses[idx]->printInfo();
            }
            // for (int idx = 0; idx < 28; idx++)
            // {
            //     cube.springs[idx]->printInfo();
            // }
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