//
// Created by Vetle Wegner Ingeberg on 12/11/2019.
//

#ifndef CLI_INTERFACE_H
#define CLI_INTERFACE_H

#include <vector>
#include <string>

// Libs
#include <Buffer/Buffer.h>
#include <Parser/Entry.h>
#include <Parameters/experimentsetup.h>
#include <Event/Event.h>

namespace Parser {
    class Base;
}

// External dependencies
#include <blockingconcurrentqueue.h>
// Typedefs
typedef moodycamel::BlockingConcurrentQueue<Parser::Entry_t> Entry_queue_t;
typedef moodycamel::BlockingConcurrentQueue< std::vector<Parser::Entry_t> > Event_queue_t;

struct Settings_t {
    std::vector<std::string> input_files;   //!< List of all files to read from
    std::string output_file;                //!< File to write to
    std::string file_title;                 //!< Title of the output file
    bool build_tree;                        //!< Flag to indicate output to a tree
    std::string tree_name;                  //!< Name of the output tree
    std::string tree_title;                 //!< Title of the output tree
    Fetcher::Buffer *buffer_type;           //!< Defines the buffer type (and the format)
    Parser::Base *parser;                   //!< A parser object (defined by the format)
    Event::Base *event_type;                //!< Type of the event (defined by the format)
    double split_time;                      //!< Time gap where entries are split
    double event_time;                      //!< Time where entries are grouped together
    DetectorType trigger_type;              //!< Detector type acting as "trigger" in event builder
    Entry_queue_t *input_queue;             //!< Queue with sorted entries from the parser
    Event_queue_t *split_queue;             //!< Queue with grouped entries after splitting
    Event_queue_t *built_queue;             //!< Queue with finished built entries
    size_t num_split_threads;               //!< Number of splitter threads
    size_t num_filler_threads;              //!< Number of filler threads

    ~Settings_t(); // Clean-up
};


#endif // CLI_INTERFACE_H
