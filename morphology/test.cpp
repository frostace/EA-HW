#include <iostream>
#include <fstream>

int main()
{
    std::ofstream skeleton;
    skeleton.open("skeleton.txt");
    skeleton << "test";
    skeleton.close();
}