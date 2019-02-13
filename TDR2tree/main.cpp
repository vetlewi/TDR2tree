#include <iostream>

#include <string>
#include <vector>

#include "tools.h"
#include "CommandLineInterface.h"
#include "Calibration.h"

#include <algorithm>

struct test {
    bool remove;
    int value;
};

bool operator<(const test &lhs, const test &rhs){ return lhs.value < rhs.value; }
bool operator>(const test &lhs, const test &rhs){ return lhs.value < rhs.value; }

/*int main(int argc, char *argv[])
{

    std::vector<int> v = {0, 1, 1, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 22, 21, 1, -1, 2, -2};
    std::vector<int> v_new;

    std::sort(v.begin(), v.end(), [](int lhs, int rhs){ return lhs > rhs; });

    while ( v.size() > 0 ){
        std::cout << v[0] << ": ";
        for (size_t i = 1 ; i < v.size() ; ++i){
            std::cout << v[i] << " ";
            if ( v[i] - v[0] != 0 ){
                v_new.push_back(v[i]);
            }
        }
        std::cout << std::endl;
        v = v_new;
        v_new.clear();
    }



    return 0;
}*/



int main(int argc, char *argv[])
{

    CommandLineInterface interface;
    std::vector<std::string> input_file;
    std::string output_file, cal_file;
    bool make_tree;
    interface.Add("-i", "Input file", &input_file);
    interface.Add("-o", "Output file", &output_file);
    interface.Add("-c", "Calibration file", &cal_file);
    interface.Add("-mt", "Build tree", &make_tree);
    interface.CheckFlags(argc, argv);


    if (input_file.empty() || output_file.empty() ){
        std::cerr << "Input or output file missing." << std::endl;
        return -1;
    }

    if ( !cal_file.empty() ){
        if ( !SetCalibration(cal_file.c_str()) ){
            std::cerr << "Error reading calibration file." << std::endl;
            return -1;
        }
    }


    Convert_to_ROOT(input_file, output_file.c_str(), make_tree);
    //std::cout << "Done converting to ROOT" << std::endl;
    return 0;

}

