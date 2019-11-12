//
// Created by Vetle Wegner Ingeberg on 30/10/2019.
//

#ifndef TDR2TREE_ITHEMBAEVENT_H
#define TDR2TREE_ITHEMBAEVENT_H

#include <vector>

#include "Event/Event.h"
#include "Parser/Entry.h"

class TH2;
class TTree;
class TBranch;

#define MAX_NUM 128

namespace Event {

    struct iThembaEntry {
        uint16_t ID;
        uint16_t e_raw;
        double energy;
        double tfine;
        int64_t tcoarse;
        bool cfdvalid;
    };

    class iThembaData : public EventData
    {

    private:

        int mult;                   //!< Number of fields populated.
        uint16_t ID[MAX_NUM];       //!< ID's of the entries.
        uint16_t e_raw[MAX_NUM];    //!< Raw energy of the entries.
        double energy[MAX_NUM];     //!< Energy of the entries.
        double tfine[MAX_NUM];      //!< CFD correction of the timestamp.
        int64_t tcoarse[MAX_NUM];   //!< Timestamp of the entry.
        bool cfdvalid[MAX_NUM];     //!< Flag indicating if the CFD is valid.

        TBranch *b_mult;
        TBranch *b_ID;
        TBranch *b_e_raw;
        TBranch *b_energy;
        TBranch *b_tfine;
        TBranch *b_tcoarse;
        TBranch *b_cfdvalid;

    public:

        /*!
         * Constructor.
         */
        iThembaData()
        : mult(0), ID{}, e_raw{}, energy{}, tfine{}, tcoarse{}, cfdvalid{}
        , b_mult( nullptr ), b_ID( nullptr ), b_e_raw( nullptr ), b_energy( nullptr )
        , b_tfine( nullptr ), b_tcoarse( nullptr ), b_cfdvalid( nullptr ) {}

        /*!
         * Destructor
         */
        ~iThembaData() override = default;

        /*!
         * Add a word to this entry.
         * @param word - raw data from file.
         * @return True if mult < MAX_NUM, false otherwise.
         */
        bool Add(const Parser::Entry_t &word);

        /*!
         * Add a word to this entry.
         * @param word - raw data from file.
         * @return True if mult < MAX_NUM, false otherwise.
         */
        bool Add(const iThembaEntry &entry);

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

        //void SetBranchAddress(EventData *other) override;

        void Copy(const EventData *other) override;

        /*!
         * Get number of entries.
         */
        inline int GetSize() const { return mult; }

        /*!
         * Get as TDREntry
         */
        inline iThembaEntry operator[](const int &i)
        {
            assert(i < mult);
            return {ID[i], e_raw[i], energy[i],
                    tfine[i], tcoarse[i], cfdvalid[i]};
        }

        /*!
         * Get all entries as a vector.
         */
        inline std::vector <iThembaEntry> GetEntries() const
        {
            std::vector <iThembaEntry> entries(mult);
            for (int i = 0; i < mult; ++i) {
                entries.push_back({ID[i], e_raw[i], energy[i], tfine[i], tcoarse[i], cfdvalid[i]});
            }
            return entries;
        }

    };

    class iThembaTimeData : public EventData {
        int mult;                   //!< Number of fields populated.
        double tfine[MAX_NUM];      //!< CFD correction of the timestamp.
        int64_t tcoarse[MAX_NUM];   //!< Timestamp of the entry.
        bool cfdvalid[MAX_NUM];     //!< Flag indicating if the CFD is valid.

        TBranch *b_mult;
        TBranch *b_tfine;
        TBranch *b_tcoarse;
        TBranch *b_cfdvalid;


    public:

        /*!
         * Constructor.
         */
        iThembaTimeData()
        : mult(0), tfine{}, tcoarse{}, cfdvalid{}
        , b_mult( nullptr ), b_tfine( nullptr ), b_tcoarse( nullptr ), b_cfdvalid( nullptr ){}

        /*!
         * Destructor
         */
        ~iThembaTimeData() override = default;

        /*!
         * Add a word to this entry.
         * @param word - raw data from file.
         * @return True if mult < MAX_NUM, false otherwise.
         */
        bool Add(const Parser::Entry_t &word);

        /*!
         * Add a word to this entry.
         * @param word - raw data from file.
         * @return True if mult < MAX_NUM, false otherwise.
         */
        bool Add(const iThembaEntry &entry);

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

        void Copy(const EventData *other) override;

        //void SetBranchAddress(EventData *other) override;

        /*!
         * Get number of entries.
         */
        inline int GetSize() const { return mult; }

        /*!
         * Get as TDREntry
         */
        inline iThembaEntry operator[](const int &i)
        {
            assert(i < mult);
            return {0, 0, 0,tfine[i], tcoarse[i], cfdvalid[i]};
        }

        /*!
         * Get all entries as a vector.
         */
        inline std::vector<iThembaEntry> GetEntries() const
        {
            std::vector<iThembaEntry> entries(mult);
            for ( int i = 0 ; i < mult ; ++i ){
                entries.push_back({0, 0, 0, tfine[i], tcoarse[i], cfdvalid[i]});
            }
            return entries;
        }

    };


    class iThembaEvent : public Base
    {

    protected:

        iThembaData ringData;      //!< Ring dE-Si event structure.
        iThembaData sectData;      //!< Sector dE-Si event structure.
        iThembaData backData;      //!< Back E-Si event structure.
        iThembaData labrLData;     //!< LaBr 3x8" event structure.
        iThembaData labrSData;     //!< LaBr 2x2" (slow signal) event structure.
        iThembaData labrFData;     //!< LaBr 2x2" (fast signal) event structure.
        iThembaData cloverData;    //!< CLOVER event structure.
        iThembaTimeData rfData;    //!< RF event structure.

    public:

        //! Constructor.
        explicit iThembaEvent(TTree *tree = nullptr);

        /*!
         * Set the event from raw data.
         * @param data - list of entries.
         */
        explicit iThembaEvent(const std::vector <Parser::Entry_t> &data);

        /*!
         * Return a new object
         * @return a new object of same type.
         */
        inline Base *New() override { return new iThembaEvent; }

        /*!
         * Run addback routine.
         */
        void Addback(TH2 *hist);

        /*!
         * Get ring data.
         */
        inline std::vector <iThembaEntry> GetRing() const { return ringData.GetEntries(); }

        /*!
         * Get sector data.
         */
        inline std::vector <iThembaEntry> GetSect() const { return sectData.GetEntries(); }

        /*!
         * Get back data.
         */
        inline std::vector <iThembaEntry> GetBack() const { return backData.GetEntries(); }

        /*!
         * Get LaBr L data.
         */
        inline std::vector <iThembaEntry> GetLabrL() const { return labrLData.GetEntries(); }

        /*!
         * Get LaBr S data.
         */
        inline std::vector <iThembaEntry> GetLabrS() const { return labrSData.GetEntries(); }

        /*!
         * Get LaBr F data.
         */
        inline std::vector <iThembaEntry> GetLabrF() const { return labrFData.GetEntries(); }

        /*!
         * Get CLOVER data.
         */
        inline std::vector <iThembaEntry> GetClover() const { return cloverData.GetEntries(); }

        /*!
         * Get CLOVER data.
         */
        inline std::vector <iThembaEntry> GetRF() const { return rfData.GetEntries(); }

    };
}



#endif //TDR2TREE_ITHEMBAEVENT_H
