// Constants.h
#if !defined(MYLIB_CONSTANTS_H)
#define MYLIB_CONSTANTS_H 1

const double GRAVITY = 9.81;         // gravitational acceleration
const double damping = 0.999;        // damping ratio
const double dt = 0.0008;            // simulation time step
const double k_vertices_soft = 2000; // stiffness of springs
const double k_ground = 2000000;     // stiffness of ground
const double friction_mu_s = 1;      // friction coeff of rubber-concrete
const double friction_mu_k = 0.8;
double omega = 10;
const float pi = 3.1415926535897932384626;
const double k_w = 20 * pi;

std::vector<float> k_spring = {1000, 20000, 2000, 2000, 0};
std::vector<float> b_spring = {0, 0, 0.02, 0.02, 0};
std::vector<float> c_spring = {0, 0, 0, pi, 0};
std::vector<std::vector<float>> cube_colors = {
    // types:                    color           |  k         | b     | c
    {0.172f, 0.243f, 0.313f}, // midnight blue      1,000       0       0  (black)
    {0.584f, 0.647f, 0.651f}, // concrete           20,000      0       0  (gray)
    {0.153f, 0.682f, 0.376f}, // nephritis          5,000       0.25    0  (green)
    {0.906f, 0.298f, 0.235f}, // alizarin           5,000       0.25    pi (red)
    {1.0f, 1.0f, 1.0f}        // white              0           0       0  (white)
};
std::vector<int> cube_types = {0, 1, 2, 3, 4};

// up, front, left, right, back, down
std::vector<std::vector<int>> refMassIndices = {
    {5, 6, 7, 4}, {1, 2, 6, 5}, {1, 0, 4, 5}, {2, 3, 7, 6}, {0, 3, 7, 4}, {1, 2, 3, 0}};
std::vector<std::vector<int>> refSpringIndices = {{25, 26, 22, 27, 23, 24},
                                                  {7, 11, 10, 16, 15, 25},
                                                  {0, 3, 4, 3, 4, 22},
                                                  {13, 17, 16, 21, 20, 27},
                                                  {2, 6, 3, 21, 18, 24},
                                                  {7, 8, 0, 13, 1, 2}};

float genMagnitude()
{
    return 1 + (0.5 - rand() / float(RAND_MAX)) / 0.5 * 0.05;
}

#endif
