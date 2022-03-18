#ifndef COMMAND_LINE_INTERFACE_H
#define COMMAND_LINE_INTERFACE_H

#include <vector>
#include <string>
#include <optional>

#include "experimentsetup.h"

namespace CLI {

    enum veto_action {
        ignore,
        keep,
        remove
    };

    enum sort_type {
        coincidence,
        gap
    };

    struct Options
    {
        // Required arguments
        std::optional<std::vector<std::string>> input;
        std::optional<std::string> output;

        // Optional arguments
        std::optional<std::string> CalibrationFile;
        std::optional<std::string> RangeFile;
        std::optional<double> coincidenceTime = 1500;
        std::optional<double> SplitTime = 1500;
        std::optional<bool> tree = false;
        std::optional<sort_type> sortType = sort_type::coincidence;
        std::optional<DetectorType> Trigger = DetectorType::eDet;
        std::optional<bool> addback = false;
        std::optional<veto_action> VetoAction = veto_action::ignore;
    };

    Options ParseCLA(const int &argc, char *argv[]);

}

std::ostream &operator<<(std::ostream &os, const CLI::Options &opt);

#endif // COMMAND_LINE_INTERFACE_H