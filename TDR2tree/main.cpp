#include <iostream>

#include <string>
#include <vector>

#include "tools.h"
#include "CommandLineInterface.h"
#include "Calibration.h"



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

