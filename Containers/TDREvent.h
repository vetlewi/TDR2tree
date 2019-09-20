//
// Created by Vetle Wegner Ingeberg on 16/09/2019.
//

#ifndef TDR2TREE_TDREVENT_H
#define TDR2TREE_TDREVENT_H

#include <cassert>
#include <vector>

#include "BasicStruct.h"
#include "Event.h"

#define MAX_NUM 128

class TH2;

namespace Event {

    struct TDREntry {
        uint16_t ID;
        uint16_t e_raw;
        double energy;
        double tfine;
        int64_t tcoarse;
        bool cfdvalid;
    };

    class TDREventData : public EventData {

    private:

        int mult;                   //!< Number of fields populated.
        uint16_t ID[MAX_NUM];       //!< ID's of the entries.
        uint16_t e_raw[MAX_NUM];    //!< Raw energy of the entries.
        double energy[MAX_NUM];     //!< Energy of the entries.
        double tfine[MAX_NUM];      //!< CFD correction of the timestamp.
        int64_t tcoarse[MAX_NUM];   //!< Timestamp of the entry.
        bool cfdvalid[MAX_NUM];     //!< Flag indicating if the CFD is valid.


    public:

        /*!
         * Constructor.
         */
        TDREventData() : mult(0) {}

        /*!
         * Destructor
         */
        ~TDREventData() {}

        /*!
         * Add a word to this entry.
         * @param word - raw data from file.
         * @return True if mult < MAX_NUM, false otherwise.
         */
        bool Add(const word_t &word);

        /*!
         * Add a word to this entry.
         * @param word - raw data from file.
         * @return True if mult < MAX_NUM, false otherwise.
         */
        bool Add(const TDREntry &entry);

        /*!
         * Reset the class
         */
        inline void Reset() override { mult = 0; }

        /*!
         * Setup the correct branches.
         * @param tree - tree where this will be referred in.
         * @param baseName - base name of the branches.
         */
        void SetupBranch(TTree *tree, const char *baseName) override;

        /*!
         * Get number of entries.
         */
        inline int GetSize() const { return mult; }

        /*!
         * Get as TDREntry
         */
        inline TDREntry operator[](const int &i)
        {
            assert(i < mult);
            return {ID[i], e_raw[i], energy[i],
                    tfine[i], tcoarse[i], cfdvalid[i]};
        }

        /*!
         * Get all entries as a vector.
         */
        inline std::vector<TDREntry> GetEntries() const
        {
            std::vector<TDREntry> entries(mult);
            for ( int i = 0 ; i < mult ; ++i ){
                entries.push_back({ID[i], e_raw[i], energy[i], tfine[i], tcoarse[i], cfdvalid[i]});
            }
            return entries;
        }

    };

    class TDRTimeData : public EventData {
        int mult;                   //!< Number of fields populated.
        double tfine[MAX_NUM];      //!< CFD correction of the timestamp.
        int64_t tcoarse[MAX_NUM];   //!< Timestamp of the entry.
        bool cfdvalid[MAX_NUM];     //!< Flag indicating if the CFD is valid.


    public:

        /*!
         * Constructor.
         */
        TDRTimeData() : mult(0) {}

        /*!
         * Destructor
         */
        ~TDRTimeData() {}

        /*!
         * Add a word to this entry.
         * @param word - raw data from file.
         * @return True if mult < MAX_NUM, false otherwise.
         */
        bool Add(const word_t &word);

        /*!
         * Add a word to this entry.
         * @param word - raw data from file.
         * @return True if mult < MAX_NUM, false otherwise.
         */
        bool Add(const TDREntry &entry);

        /*!
         * Reset the class
         */
        inline void Reset() override { mult = 0; }

        /*!
         * Setup the correct branches.
         * @param tree - tree where this will be referred in.
         * @param baseName - base name of the branches.
         */
        void SetupBranch(TTree *tree, const char *baseName) override;

        /*!
         * Get number of entries.
         */
        inline int GetSize() const { return mult; }

        /*!
         * Get as TDREntry
         */
        inline TDREntry operator[](const int &i)
        {
            assert(i < mult);
            return {0, 0, 0,tfine[i], tcoarse[i], cfdvalid[i]};
        }

        /*!
         * Get all entries as a vector.
         */
        inline std::vector<TDREntry> GetEntries() const
        {
            std::vector<TDREntry> entries(mult);
            for ( int i = 0 ; i < mult ; ++i ){
                entries.push_back({0, 0, 0, tfine[i], tcoarse[i], cfdvalid[i]});
            }
            return entries;
        }

    };

    class TDREvent : public Base {

    protected:

        TDREventData ringData;      //!< Ring dE-Si event structure.
        TDREventData sectData;      //!< Sector dE-Si event structure.
        TDREventData backData;      //!< Back E-Si event structure.
        TDREventData labrLData;     //!< LaBr 3x8" event structure.
        TDREventData labrSData;     //!< LaBr 2x2" (slow signal) event structure.
        TDREventData labrFData;     //!< LaBr 2x2" (fast signal) event structure.
        TDREventData cloverData;    //!< CLOVER event structure.
        TDRTimeData rfData;         //!< RF event structure.

    public:

        //! Constructor.
        TDREvent();

        /*!
         * Set the event from raw data.
         * @param data - list of entries.
         */
        explicit TDREvent(const std::vector<word_t> &data);

        /*!
         * Return a new object
         * @return a new object of same type.
         */
        inline Base *New() override { return new TDREvent; }

        /*!
         * Run addback routine.
         */
        void Addback(TH2 *hist);

        /*!
         * Get ring data.
         */
        inline std::vector<TDREntry> GetRing() const { return ringData.GetEntries(); }

        /*!
         * Get sector data.
         */
        inline std::vector<TDREntry> GetSect() const { return sectData.GetEntries(); }

        /*!
         * Get back data.
         */
        inline std::vector<TDREntry> GetBack() const { return backData.GetEntries(); }

        /*!
         * Get LaBr L data.
         */
        inline std::vector<TDREntry> GetLabrL() const { return labrLData.GetEntries(); }

        /*!
         * Get LaBr S data.
         */
        inline std::vector<TDREntry> GetLabrS() const { return labrSData.GetEntries(); }

        /*!
         * Get LaBr F data.
         */
        inline std::vector<TDREntry> GetLabrF() const { return labrFData.GetEntries(); }

        /*!
         * Get CLOVER data.
         */
        inline std::vector<TDREntry> GetClover() const { return cloverData.GetEntries(); }

        /*!
         * Get CLOVER data.
         */
        inline std::vector<TDREntry> GetRF() const { return rfData.GetEntries(); }


    };

}

#endif //TDR2TREE_TDREVENT_H
