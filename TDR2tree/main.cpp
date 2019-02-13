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
    Options opt;
    interface.Add("-i", "Input file", &input_file);
    interface.Add("-o", "Output file", &output_file);
    interface.Add("-c", "Calibration file", &cal_file);
    interface.Add("-mt", "Build tree", &opt.make_tree);
    interface.Add("-ab", "Add-back; If addback should be applied on clovers", &opt.use_addback);
    interface.Add("-ar", "Indicate if all small LaBr detectors should be used as reference", &opt.use_all_labrS);
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

    Convert_to_ROOT(input_file, output_file.c_str(), opt);
    return 0;

}

