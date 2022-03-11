//
// Created by Vetle Wegner Ingeberg on 11/03/2022.
//

#ifndef TDR2TREE_TASKS_H
#define TDR2TREE_TASKS_H

#include <Task.h>
#include <CommandLineInterface.h>

namespace Task {

    /*!
     * A container for multiple tasks sharing the same input queue and has no output.
     * At the moment only able to be used by the Task::Sort class.
     * \tparam T
     */
    template<typename T, typename V>
    class ParallelTasksEnd {

    private:
        T &input_queue;
        std::vector<V> tasks;

    public:

        ParallelTasksEnd(T &_input_queue, const size_t &num_parallel, const CLI::Options &options)
            : input_queue( _input_queue )
            , tasks( )
        {
            tasks.reserve(num_parallel);
            for ( size_t n = 0 ; n < num_parallel ; ++n )
                tasks.emplace_back(input_queue, options);
        }

        V &operator[](const size_t &n){ return tasks[n]; }

        typename std::vector<V>::iterator &begin(){ return tasks.begin(); }
        typename std::vector<V>::iterator &end(){ return tasks.end(); }

    };

}

#endif //TDR2TREE_TASKS_H
