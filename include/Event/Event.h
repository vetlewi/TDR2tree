#ifndef EVENT_BASE_H
#define EVENT_BASE_H

#include <map>
#include <string>


class TTree;

namespace Event {

/*
 * A pure virtual base class for the event data (ie. storing the entries)
 */
    class EventData {

    public:

        //! Destructor
        virtual ~EventData() = default;

        //! Set the branches of the tree to the event data.
        virtual void SetupBranch(TTree *tree, const char *baseName) = 0;

        //! Reset all data.
        virtual void Reset() = 0;

        //! Get the branch.
        //virtual void SetBranchAddress(EventData *other) = 0;

        virtual void Copy(const EventData *other) = 0;

        //! Copy operator
        EventData &operator=(const EventData *other)
        {
            this->Copy(other);
            return *this;
        }

    };


    class Base {

    protected:

        //! Mapping of all the entries. In derived types every EventData member has to be registered here.
        std::map<std::string, EventData *> event_data;

        //! Register a member.
        void Register(EventData *member, const std::string &name) {
            event_data[name] = member;
        }

    public:

        //! Destructor
        virtual ~Base() = default;

        //! Copy contents of another event.
        void Copy(Base *other_event) {
            for (auto &data : event_data) {
                data.second->Copy(other_event->event_data[data.first]);
            }
        }

        //! Set the branches of the tree to the event data.
        void SetupTree(TTree *tree) {
            for (auto &entry_container : event_data) {
                entry_container.second->SetupBranch(tree, entry_container.first.c_str());
            }
        }

        /*!
         * New method
         * @return Return a new object of the same type.
         */
        virtual Base *New() = 0;

    };

}

#endif // EVENT_BASE_H