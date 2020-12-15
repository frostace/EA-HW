#pragma once

#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <functional>
#include <memory>

#include "Mass.cpp"
#include "Spring.cpp"
#include "Constants.h"
#include "omp.h"

class Cube
{
private:
    void fillArrWithRandomColor(float *arr)
    {
        for (int i = 0; i < 3; i++)
            *arr++ = rand() / (float)RAND_MAX;
    }
    void fillArrWithColor(float *arr, std::vector<float> colors)
    {
        for (int i = 0; i < 3; i++)
            *arr++ = colors[i];
    }

    void initMasses(float centerx, float centery, float centerz)
    {
        float x, y, z;
        for (int i = 0, j = 1, u = 2; i < 24; i += 3, j += 3, u += 3)
        {
            x = centerx + vertices[i] * sideLen / 2;
            y = centery + vertices[j] * sideLen / 2;
            z = centerz + vertices[u] * sideLen / 2;
            std::shared_ptr<Mass> m(new Mass(x, y, z));
            masses.push_back(m);
        }
    }
    // init masses with vector of existing masses, TODO: haven't tested
    void initMasses(std::vector<std::shared_ptr<Mass>> &massArray)
    {
        for (auto &m : massArray)
        {
            masses.push_back(m);
        }
    }
    void initMasses(float centerx, float centery, float centerz,
                    std::shared_ptr<Mass> m1, std::shared_ptr<Mass> m2, std::shared_ptr<Mass> m3, std::shared_ptr<Mass> m4,
                    int i1, int i2, int i3, int i4)
    {
        float x, y, z;
        for (int i = 0, j = 1, u = 2; i < 24; i += 3, j += 3, u += 3)
        {
            if (i / 3 == i1)
            {
                masses.push_back(m1);
            }
            else if (i / 3 == i2)
            {
                masses.push_back(m2);
            }
            else if (i / 3 == i3)
            {
                masses.push_back(m3);
            }
            else if (i / 3 == i4)
            {
                masses.push_back(m4);
            }
            else
            {
                x = centerx + vertices[i] * sideLen / 2;
                y = centery + vertices[j] * sideLen / 2;
                z = centerz + vertices[u] * sideLen / 2;
                std::shared_ptr<Mass> m(new Mass(x, y, z));
                masses.push_back(m);
            }
        }
    }
    void initSprings()
    {
        // init springs
        for (int i = 0; i < 7; i++)
        {
            for (int j = i + 1; j < 8; j++)
            {
                std::shared_ptr<Spring> s(new Spring(masses[i], masses[j]));
                springs.push_back(s);
            }
        }
    }
    void initSprings(std::vector<std::shared_ptr<Spring>> &springArray)
    {
        for (auto &s : springArray)
        {
            springs.push_back(s);
        }
    }
    void initSprings(std::shared_ptr<Spring> s1, std::shared_ptr<Spring> s2, std::shared_ptr<Spring> s3, std::shared_ptr<Spring> s4, std::shared_ptr<Spring> s5, std::shared_ptr<Spring> s6,
                     int j1, int j2, int j3, int j4, int j5, int j6)
    {
        // init springs
        for (int i = 0; i < 7; i++)
        {
            for (int j = i + 1; j < 8; j++)
            {
                int idx = i * 8 + 0.5 * i * (i + 1) + j - (i + 1);
                if (idx == j1)
                    springs.push_back(s1);
                else if (idx == j2)
                    springs.push_back(s2);
                else if (idx == j3)
                    springs.push_back(s3);
                else if (idx == j4)
                    springs.push_back(s4);
                else if (idx == j5)
                    springs.push_back(s5);
                else if (idx == j6)
                    springs.push_back(s6);
                else
                {
                    std::shared_ptr<Spring> s(new Spring(masses[i], masses[j]));
                    springs.push_back(s);
                }
            }
        }
    }

public:
    float sideLen = 1.0f;
    float totalEp = 0.0f; // total potential energy
    float totalEk = 0.0f; // total kinetic energy
    float centroidX = 0.0f;
    float centroidY = 0.0f;
    float centroidZ = 0.0f;
    int prevType;
    float prevK;
    float prevB;
    float prevC;
    int currType;

    float positions[108] = {};
    float vertices[24] = {
        -1.0f, -1.0f, -1.0f, // 0
        1.0f, -1.0f, -1.0f,  // 1
        1.0f, 1.0f, -1.0f,   // 2
        -1.0f, 1.0f, -1.0f,  // 3
        -1.0f, -1.0f, 1.0f,  // 4
        1.0f, -1.0f, 1.0f,   // 5
        1.0f, 1.0f, 1.0f,    // 6
        -1.0f, 1.0f, 1.0f,   // 7
    };
    int indices[36] = {
        2, 1, 0, 2, 0, 3, 7, 4, 5, 6, 7, 5, 6, 2, 3, 6, 3, 7, 5, 0, 1, 5, 4, 0, 0, 4, 7, 0, 7, 3, 6, 1, 2, 1, 6, 5};
    float *color = new float[3];
    std::vector<std::shared_ptr<Mass>> masses = {};
    std::vector<std::shared_ptr<Spring>> springs = {};

    Cube(float centerx, float centery, float centerz)
    {
        fillArrWithRandomColor(color);
        initMasses(centerx, centery, centerz);
        initSprings();
        updatePositions();
        // masses[4]->grounded = true;
    }

    Cube(float centerx, float centery, float centerz,
         std::shared_ptr<Mass> m1, std::shared_ptr<Mass> m2, std::shared_ptr<Mass> m3, std::shared_ptr<Mass> m4,
         int i1, int i2, int i3, int i4,
         std::shared_ptr<Spring> s1, std::shared_ptr<Spring> s2, std::shared_ptr<Spring> s3, std::shared_ptr<Spring> s4, std::shared_ptr<Spring> s5, std::shared_ptr<Spring> s6,
         int j1, int j2, int j3, int j4, int j5, int j6)
    {
        fillArrWithRandomColor(color);
        initMasses(centerx, centery, centerz,
                   m1, m2, m3, m4,
                   i1, i2, i3, i4);
        initSprings(s1, s2, s3, s4, s5, s6,
                    j1, j2, j3, j4, j5, j6);
        updatePositions();
        // masses[4]->grounded = true;
    }

    Cube(int type, std::vector<std::shared_ptr<Mass>> &massArray, std::vector<std::shared_ptr<Spring>> &springArray)
    {
        currType = type;
        fillArrWithColor(color, cube_colors[type]);
        initMasses(massArray);
        initSprings(springArray);
        updatePositions();
    }

    Cube(std::vector<std::shared_ptr<Mass>> &massArray, std::vector<std::shared_ptr<Spring>> &springArray)
    {
        fillArrWithColor(color, cube_colors[2]);
        initMasses(massArray);
        initSprings(springArray);
        updatePositions();
    }

    Cube()
    {
        fillArrWithRandomColor(color);
        initMasses(0, 0, 0);
        initSprings();
        updatePositions();
    }

    ~Cube()
    {
    }

    void iterate(bool breathing, int iterNums)
    {
        // skip iteration if cube is air
        if (currType == 4)
            return;
        totalEp = 0;
        totalEk = 0;

        // int springCount = 0;
        // int massCount = 0;
        // #pragma omp parallel for
        for (int i = 0; i < springs.size(); i++)
        {
            auto s = springs[i];
            // duplicate computation avoidance
            if (s->computed)
                continue;
            s->cumulateRestoreForceToConnectedMasses();
            if (breathing)
                s->breathe(iterNums);

            s->computed = true;

            // compute spring potential energy
            totalEp += s->getPotentialEnergy();
        }

        // apply force on each mass
        // #pragma omp parallel for
        for (int i = 0; i < masses.size(); i++)
        {
            auto m = masses[i];
            // duplicate computation avoidance
            if (m->computed)
                continue;
            m->addGroundForce();
            m->addGravity();
            m->addFriction();

            m->computed = true;

            // compute mass potential energy and kinetic energy
            totalEp += m->getPotentialEnergy();
            // totalEp += m->getGroundPotentialEnergy();
            totalEk += m->getKineticEnergy();
        }
    }

    void integrate()
    {
        float centroidXTotal = 0;
        float centroidYTotal = 0;
        float centroidZTotal = 0;
        for (auto &m : masses)
        {
            m->applyForce();
            m->clearForce();
            centroidXTotal += m->pos[0];
            centroidYTotal += m->pos[1];
            centroidZTotal += m->pos[2];
        }
        centroidX = centroidXTotal / 8;
        centroidY = centroidYTotal / 8;
        centroidZ = centroidZTotal / 8;
        updatePositions();
    }

    void addWind(int mag)
    {
        for (auto &m : masses)
        {
            m->addWind(mag);
        }
    }

    void clearComputed()
    {
        for (auto &m : masses)
        {
            m->computed = false;
        }
        for (auto &s : springs)
        {
            s->computed = false;
        }
    }

    void mutate()
    {
        // switch cube type
        prevType = currType;
        currType = rand() % 4;
        fillArrWithColor(color, cube_colors[currType]);
        for (auto &s : springs)
        {
            s->k = k_spring[currType];
            s->b = b_spring[currType];
            s->c = c_spring[currType];
        }
    }

    void mutateType(int type)
    {
        // switch cube type
        prevType = currType;
        currType = type;
        fillArrWithColor(color, cube_colors[currType]);
        for (auto &s : springs)
        {
            s->k = k_spring[currType];
            s->b = b_spring[currType];
            s->c = c_spring[currType];
        }
    }

    void mutate(int springIdx)
    {
        // mutate spring idx
        // prevK = springs[springIdx]->k;
        prevB = springs[springIdx]->b;
        prevC = springs[springIdx]->c;
        // springs[springIdx]->k = (prevK + 0.01) * genMagnitude();
        springs[springIdx]->b = (prevB + 0.01) * genMagnitude();
        springs[springIdx]->c = (prevC + 0.01) * genMagnitude();
    }

    void recover()
    {
        // revert cube type back to previous type
        currType = prevType;
        fillArrWithColor(color, cube_colors[currType]);
        for (auto &s : springs)
        {
            s->k = k_spring[currType];
            s->b = b_spring[currType];
            s->c = c_spring[currType];
        }
    }

    void recover(int springIdx)
    {
        // revert spring params back to prev set of params
        springs[springIdx]->k = prevK;
        springs[springIdx]->b = prevB;
        springs[springIdx]->c = prevC;
    }

    void updatePositions()
    {
        // clear positions
        int i = 0;
        for (int idx : indices)
        {
            positions[i++] = masses[idx]->pos[0];
            positions[i++] = masses[idx]->pos[1];
            positions[i++] = masses[idx]->pos[2];
        }
    }

    void overrideType(int type)
    {
        currType = type;
        fillArrWithColor(color, cube_colors[type]);
    }

    void printInfo()
    {
        for (int i = 0; i < 8; i++)
        {
            masses[i]->printInfo();
        }
        for (float val : positions)
        {
            std::cout << val << ", ";
        }
        std::cout << std::endl;
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