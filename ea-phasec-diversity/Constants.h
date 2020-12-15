// Constants.h
#if !defined(MYLIB_CONSTANTS_H)
#define MYLIB_CONSTANTS_H 1

#include <numeric>
#include <cmath>

const int evalNums = 100000;
const float mutationRate = 0.4;
const int generationNum = 1000;

const double GRAVITY = 9.81;         // gravitational acceleration
const double damping = 0.99;         // damping ratio
const double dt = 0.001;             // simulation time step
const double k_vertices_soft = 2000; // stiffness of springs
const double k_ground = 2000000;     // stiffness of ground
const double friction_mu_s = 1;      // friction coeff of rubber-concrete
const double friction_mu_k = 0.8;
double omega = 10;
const float pi = 3.1415926535897932384626;
const double k_w = 15 * pi;

std::vector<float> k_spring = {8000, 20000, 8000, 8000, 0};
std::vector<float> b_spring = {0, 0, 0.2, 0.2, 0};
std::vector<float> c_spring = {2 * pi, 2 * pi, 2 * pi, pi, 2 * pi};
std::vector<std::vector<float>> cube_colors = {
    // types:                    color           |  k         | b     | c
    {0.584f, 0.647f, 0.651f}, // concrete           8,000       0       0
    {0.172f, 0.243f, 0.313f}, // midnight blue      20,000      0       0
    {0.153f, 0.682f, 0.376f}, // nephritis          8,000       0.20    0
    {0.906f, 0.298f, 0.235f}, // alizarin           8,000       0.20    pi
    {1.0f, 1.0f, 1.0f}        // white              0           0       0
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

// generative representation radius
const int radius = 2;
std::vector<std::vector<int>> directions = {
    {0, 0, 1},  // up
    {1, 0, 0},  // front
    {0, -1, 0}, // left
    {0, 1, 0},  // right
    {-1, 0, 0}, // back
};
// std::vector<std::vector<int>> directions3d = {
//     {1, 0, 0}, {-1, 0, 0}, {0, 1, 0}, {0, -1, 0}, {1, 1, 0}, {1, -1, 0}, {-1, 1, 0}, {-1, -1, 0}, {1, 0, 1}, {-1, 0, 1}, {0, 1, 1}, {0, -1, 1}, {1, 1, 1}, {1, -1, 1}, {-1, 1, 1}, {-1, -1, 1}, {0, 0, 1}, {1, 0, -1}, {-1, 0, -1}, {0, 1, -1}, {0, -1, -1}, {1, 1, -1}, {1, -1, -1}, {-1, 1, -1}, {-1, -1, -1}, {0, 0, -1}};
std::vector<std::vector<int>> directions3d = {
    {1, 0, 0}, {0, 1, 0}, {1, 1, 0}, {1, 0, 1}, {0, 1, 1}, {1, 1, 1}, {0, 0, 1}};
// 8 position offsets of cube masses
std::vector<std::vector<int>>
    cube8dirs = {
        {0, 0, 0},
        {0, 1, 0},
        {1, 1, 0},
        {1, 0, 0},
        {0, 0, 1},
        {0, 1, 1},
        {1, 1, 1},
        {1, 0, 1},
};

void dfs_fill(std::vector<int> coords, int depth, int colorIdx, int idim, int jdim, int kdim, std::vector<std::vector<std::vector<int>>> &types)
{
    int i = (coords[0] + idim) % idim;
    int j = (coords[1] + jdim) % jdim;
    int k = (coords[2] + kdim) % kdim;
    // base case
    if (depth > radius || types[i][j][k] != -1)
        return;

    types[i][j][k] = colorIdx;

    for (auto &dir : directions3d)
        dfs_fill({coords[0] + dir[0], coords[1] + dir[1], coords[2] + dir[2]}, depth + 1, colorIdx, idim, jdim, kdim, types);
}

int randint(int dim)
{
    return rand() % dim;
}

std::vector<std::vector<int>> genInitialPoints(int pointNums, int idim, int jdim, int kdim)
{
    std::vector<std::vector<int>> output = {};
    for (int num = 0; num < pointNums; num++)
    {
        output.push_back({randint(idim), randint(jdim), randint(kdim)});
    }
    return output;
}

float genMagnitude()
{
    return 1 + (0.5 - rand() / float(RAND_MAX)) / 0.5 * 0.05;
}

float safe_div(float numerator, float denominator)
{
    if (denominator == 0)
        return numerator / (denominator + 0.000001);
    else
        return numerator / denominator;
}

float computeSTD(std::vector<float> v)
{
    float sum = std::accumulate(v.begin(), v.end(), 0.0);
    float mean = sum / v.size();

    std::vector<float> diff(v.size());
    transform(v.begin(), v.end(), diff.begin(), [mean](float x) { return x - mean; });
    float sq_sum = inner_product(diff.begin(), diff.end(), diff.begin(), 0.0);
    float stdev = std::sqrt(sq_sum / v.size());
    return stdev;
}
float computeMean(std::vector<float> v)
{
    float sum = std::accumulate(v.begin(), v.end(), 0.0);
    float mean = sum / v.size();

    return mean;
}

float cosineSimilarity(std::vector<float> v1, std::vector<float> v2)
{
    float mag1 = std::sqrt(std::inner_product(v1.begin(), v1.end(), v1.begin(), 0.0));
    float mag2 = std::sqrt(std::inner_product(v2.begin(), v2.end(), v2.begin(), 0.0));
    float innerP = std::inner_product(v1.begin(), v1.end(), v2.begin(), 0.0);

    return abs(safe_div(innerP, (mag1 * mag2)));
}

float measureDistance(std::vector<float> v1, std::vector<float> v2)
{
    std::vector<float> v1Norm, v2Norm;

    float miu1 = computeMean(v1);
    float std1 = computeSTD(v1);
    float miu2 = computeMean(v2);
    float std2 = computeSTD(v2);

    for (int i = 0, len = v1.size(); i < len; i++)
    {
        v1Norm.push_back(safe_div((v1[i] - miu1), std1));
        v2Norm.push_back(safe_div((v2[i] - miu2), std2));
    }

    // compute cosine similarity of the normalized vectors

    return 1 - cosineSimilarity(v1Norm, v2Norm);
}

const int NORMAL_CROSSOVER = 0x1001;
const int DETERMINISTIC_CROWDING = 0x1002;
#endif
