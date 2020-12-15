#pragma once

#include <iostream>
#include <vector>
#include <cmath>
#include <cstdlib>
#include <chrono>
#include <ctime>
#include <memory>

#include "Mass.cpp"
#include "Constants.h"

class Spring
{
private:
    // l0 = l00 * (1 + b * sin(wt + c))
    float l0;  // dynamic rest length
    float l00; // maintain static rest length
    float w = (float)k_w;

public:
    float k = k_vertices_soft;
    float b;
    float c;
    std::shared_ptr<Mass> m1;
    std::shared_ptr<Mass> m2;
    bool computed = false;
    char *idx;
    Spring(float l, std::shared_ptr<Mass> m01, std::shared_ptr<Mass> m02)
    {
        m1 = m01;
        m2 = m02;
        l0 = l;
        l00 = l;
    }
    Spring(int type, std::shared_ptr<Mass> m01, std::shared_ptr<Mass> m02)
    {
        m1 = m01;
        m2 = m02;
        l0 = getCurLen();
        l00 = l0;
        k = k_spring[type];
        b = b_spring[type];
        c = c_spring[type];
    }
    Spring(std::shared_ptr<Mass> &m01, std::shared_ptr<Mass> &m02)
    {
        m1 = m01;
        m2 = m02;
        // l0 = 0.01 * rand() / double(RAND_MAX) + getCurLen();
        l0 = getCurLen();
        l00 = l0;
    }
    ~Spring()
    {
    }

    float getCurLen()
    {
        return std::sqrt(
            (m1->pos[0] - m2->pos[0]) * (m1->pos[0] - m2->pos[0]) + (m1->pos[1] - m2->pos[1]) * (m1->pos[1] - m2->pos[1]) + (m1->pos[2] - m2->pos[2]) * (m1->pos[2] - m2->pos[2]));
    }

    std::vector<float> getCurForce()
    {
        std::vector<float> m1Force = {};
        float l = getCurLen();
        for (int i = 0; i < 3; i++)
        {
            m1Force.push_back((m2->pos[i] - m1->pos[i]) / l * k * (l - l0));
        }

        return m1Force;
    }

    float getPotentialEnergy()
    {
        float l = getCurLen();
        return 0.5 * k * (l - l0) * (l - l0);
    }

    void cumulateRestoreForceToConnectedMasses()
    {
        std::vector<float> f = getCurForce();
        m1->cumulateForce(f);
        std::transform(f.begin(), f.end(), f.begin(),
                       std::bind(std::multiplies<float>(), std::placeholders::_1, -1));
        m2->cumulateForce(f);
    }

    void breathe(int iterNums)
    {
        // make initial w * t + c = 0, otherwise, cube would explode
        auto t_clock = std::chrono::system_clock::now();
        std::time_t time = std::chrono::system_clock::to_time_t(t_clock);
        std::ctime(&time);
        float time_float = time;
        float t;

        t = iterNums * dt - c / w;

        l0 = l00 * (1 + b * sin(w * t + c));
    }

    void printInfo()
    {
        // std::cout << "l0: " << l0 << std::endl;
        // std::cout << "k: " << k << std::endl;
        std::cout << "b: " << b << std::endl;
        std::cout << "c: " << c << std::endl;
    }
};