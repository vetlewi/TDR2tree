//
// Created by Vetle Wegner Ingeberg on 11/11/2019.
//

#ifndef ITLEVENT_H
#define ITLEVENT_H

#include "Parser/Entry.h"
#include "Event/Event.h"

#define MAX_MULT 256

class TBranch;

namespace Event {

    //! Struct to store the essencial data for an entry.
    struct iTLEntry {
        uint16_t ID;
        uint16_t e_raw;
        double energy;
        double tfine;
        int64_t tcoarse;
        bool cfdvalid;
    };

    //! Data type to store iTL data
    class iTLData : public EventData {

    private:
        int         mult;               //!< Event multiplicity
        uint16_t    ID[MAX_MULT];       //!< Entry detector ID
        uint16_t    e_raw[MAX_MULT];    //!< Raw channel # for energy
        double      energy[MAX_MULT];   //!< Calibrated energy
        double      tfine[MAX_MULT];    //!< CFD correction
        int64_t     tcoarse[MAX_MULT];  //!< Timestamp of the entry
        bool        cfdvalid[MAX_MULT]; //!< CFD flag

        // We also need to keep track of the branch
        TBranch *bMult;
        TBranch *bID;
        TBranch *bRaw;
        TBranch *bEnergy;
        TBranch *bTfine;
        TBranch *bTcoarse;
        TBranch *bCfdvalid;

    public:

        /*!
         * Constructor
         */
        iTLData() : mult(0), ID{}, e_raw{}, energy{}, tfine{}, tcoarse{}, cfdvalid{}
                , bMult( nullptr ), bID( nullptr ), bRaw( nullptr ), bEnergy( nullptr )
                , bTfine( nullptr ), bTcoarse( nullptr ), bCfdvalid( nullptr ){}

        /*!
         * Destructor
         */
        ~iTLData() override = default;

        /*!
         * Add a word to this entry.
         * @param word - raw data from file.
         * @return True if mult < MAX_NUM, false otherwise.
         */
        bool Add(const Parser::Entry_t &word);

        bool Add(const iTLEntry &entry);

        /*!
         * Reset the entries
         */
        inline void Reset() override { mult = 0; }

        /*!
         * Setup the correct branches.
         * @param tree - tree where this will be referred in.
         * @param baseName - base name of the branches.
         */
        void SetupBranch(TTree *tree, const char *baseName) override;

        /*!
         * Copy data from a different object
         */
        void Copy(const EventData *other) override;

        /*!
         * Get a vector with all entries
         */
        std::vector<iTLEntry> GetEntries() const;
    };


    class iTLEvent : public Base {

    protected:

        iTLData ringData;
        iTLData sectData;
        iTLData backData;
        iTLData labrLData;
        iTLData labrSData;
        iTLData labrFData;
        iTLData cloverData;
        iTLData rfData;

    public:

        //! Constructor
        explicit iTLEvent(TTree *tree = nullptr);

        /*!
         * Set the event from raw data.
         */
        explicit iTLEvent(const std::vector <Parser::Entry_t> &data);

        /*!
         * Return a new object
         * @return a new object of same type.
         */
        inline Base *New() override { return new iTLEvent; }

        /*!
         * Get ring data.
         */
        inline std::vector <iTLEntry> GetRing() const { return ringData.GetEntries(); }

        /*!
         * Get sector data.
         */
        inline std::vector <iTLEntry> GetSect() const { return sectData.GetEntries(); }

        /*!
         * Get back data.
         */
        inline std::vector <iTLEntry> GetBack() const { return backData.GetEntries(); }

        /*!
         * Get LaBr L data.
         */
        inline std::vector <iTLEntry> GetLabrL() const { return labrLData.GetEntries(); }

        /*!
         * Get LaBr S data.
         */
        inline std::vector <iTLEntry> GetLabrS() const { return labrSData.GetEntries(); }

        /*!
         * Get LaBr F data.
         */
        inline std::vector <iTLEntry> GetLabrF() const { return labrFData.GetEntries(); }

        /*!
         * Get CLOVER data.
         */
        inline std::vector <iTLEntry> GetClover() const { return cloverData.GetEntries(); }

        /*!
         * Get CLOVER data.
         */
        inline std::vector <iTLEntry> GetRF() const { return rfData.GetEntries(); }

        void Addback(TH2 *);

    };

}

#endif // ITLEVENT_H
