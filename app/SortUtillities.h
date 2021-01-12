//
// Created by Vetle Wegner Ingeberg on 12/11/2019.
//

#ifndef SORTUTILLITIES_H
#define SORTUTILLITIES_H

// C++ STD headers
#include <vector>

// Parser library headers
#include <Parser/Entry.h>

// Utillities library
#include <Utilities/CLI_interface.h>



/*!
 * Function implementing the list splitter logic
 * \param settings Settings structure containing the input parameters from the user
 * \param entries buffer with the entries
 * \return True if entries are found and filled into the queue, False otherwise
 */
bool Split_entries(const Settings_t *settings, std::vector<Parser::Entry_t> &entries);

/*!
 * Entry point for the splitter thread
 * \param settings Settings structure containing the input parameters from the user
 * \param running flag indicating that the last buffer are in the queue
 */
void SpliterThread(const Settings_t *settings, const bool *running);

/*!
 * Function implementing the actual event building logic
 * \param settings Settings structure containing the input parameters from the user
 * \return True if any data was found in the queue, false otherwise.
 */
bool Make_events(const Settings_t *settings);

/*!
 * Entry point for the event builder thread
 * \param settings Settings structure containing the input parameters from the user
 * \param running flag to indicate that the last buffer from files have been filled in the input queue
 */
void EventBuilderThread(const Settings_t *settings, const bool *running);

/*!
 * Entry point for the ROOT file filler thread
 * \param settings Settings structure containing the input parameters from the user
 * \param running flag to indicate that the last built event has been filled to the input queue
 */
void RootFillerThread(const Settings_t *settings, const bool *running, int thread_id);

/*!
 * Implementation of the file conversion loop
 * \param settings Settings structure containing the input parameters from the user
 */
void ConvertFiles(const Settings_t *settings);

void ConvertPostgre(const Settings_t *settings);

void ConvertFilesCSV(const Settings_t *settings);

void ConvertROOT(const Settings_t *settings);

#endif // SORTUTILLITIES_H
