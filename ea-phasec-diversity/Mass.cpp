#pragma once

#include <vector>
#include <iostream>
#include <algorithm>
#include "Constants.h"
#include <cmath>
#include <memory>

/* Uncomment to always generate capital UUIDs. */
//#define capitaluuid true

/* Uncomment to always generate lower-case UUIDs. */
//#define lowercaseuuid true

class Mass
{
private:
public:
    bool grounded = false;
    float mass = 1.0f;
    bool computed = false;
    char *idx;
    std::vector<float> initPos = {0, 0, 0};
    std::vector<float> pos = {0, 0, 0};
    std::vector<float> vel = {0, 0, 0};
    std::vector<float> acc = {0, 0, 0};
    std::vector<float> force = {0, 0, 0};

    Mass(float x, float y, float z)
    {
        // TODO: refactor this with one line
        pos[0] = x;
        pos[1] = y;
        pos[2] = z;
        initPos[0] = x;
        initPos[1] = y;
        initPos[2] = z;
    }

    ~Mass()
    {
    }

    void cumulateForce(std::vector<float> f)
    {
        force[0] += f[0];
        force[1] += f[1];
        force[2] += f[2];
    }

    void applyForce()
    {
        // grounded coeff
        int coeff = grounded ? 0 : 1;

        // update acc
        acc[0] = force[0] / mass * coeff;
        acc[1] = force[1] / mass * coeff;
        acc[2] = force[2] / mass * coeff;

        // update vel
        vel[0] += acc[0] * dt;
        vel[1] += acc[1] * dt;
        vel[2] += acc[2] * dt;

        // damping
        vel[0] *= damping;
        vel[1] *= damping;
        vel[2] *= damping;

        // update pos
        pos[0] += vel[0] * dt;
        pos[1] += vel[1] * dt;
        pos[2] += vel[2] * dt;
    }

    void addGravity()
    {
        force[2] += -GRAVITY * mass;
    }

    void addWind(int mag)
    {
        force[0] += mag * rand() / (float)RAND_MAX;
    }

    void addGroundForce()
    {
        // force[2] += -k_ground * std::min((float)0.0f, pos[2]);
        if (pos[2] < 0)
            vel[2] = abs(vel[2]);
    }

    void addFriction()
    {
        if (force[2] > 0)
            return;
        // only if normal pressure is pointing to the ground
        float static_fmax = friction_mu_s * abs(force[2]);
        float fh = std::sqrt(force[0] * force[0] + force[1] * force[1]);

        if (pos[2] < 0)
        {
            if (fh < static_fmax)
            {
                force[0] = 0;
                force[1] = 0;
            }
            else
            {
                // decompose static_fmax to x, y directions
                force[0] += -static_fmax * force[0] / fh;
                force[1] += -static_fmax * force[1] / fh;
            }
        }
    }

    void clearForce()
    {
        force[0] = 0;
        force[1] = 0;
        force[2] = 0;
    }

    void clearMotion()
    {
        // TODO: refactor with initPos.copy()
        pos = {initPos[0], initPos[1], initPos[2]};
        vel = {0, 0, 0};
        acc = {0, 0, 0};
        force = {0, 0, 0};
    }

    float getPotentialEnergy()
    {
        return mass * GRAVITY * pos[2];
    }

    float getGroundPotentialEnergy()
    {
        if (pos[2] > 0)
            return 0;
        return 0.5 * k_ground * pos[2] * pos[2];
    }

    float getKineticEnergy()
    {
        return 0.5 * mass * (vel[0] * vel[0] + vel[1] * vel[1] + vel[2] * vel[2]);
    }

    void printInfo()
    {
        std::cout << "mass: " << mass << std::endl;
        std::cout << "pos: (" << pos[0] << ", " << pos[1] << ", " << pos[2] << ")" << std::endl;
        std::cout << "vel: (" << vel[0] << ", " << vel[1] << ", " << vel[2] << ")" << std::endl;
        std::cout << "acc: (" << acc[0] << ", " << acc[1] << ", " << acc[2] << ")" << std::endl;
        std::cout << "force: (" << force[0] << ", " << force[1] << ", " << force[2] << ")" << std::endl;
    }
};