/***********************************************************************************************//**
 *  Handler and store of intents.
 *  @class      IntentHandler
 *  @authors    Carles Araguz (CA), carles.araguz@upc.edu
 *  @date       2018-apr-06
 *  @version    0.1
 *  @copyright  This file is part of a project developed by Nano-Satellite and Payload Laboratory
 *              (NanoSat Lab) at Technical University of Catalonia - UPC BarcelonaTech.
 **************************************************************************************************/

#ifndef INTENT_HANDLER_HPP
#define INTENT_HANDLER_HPP

#include "prot.hpp"
#include "Intent.hpp"
#include "SegmentView.hpp"

enum class IntentSelection : int
{
    ALL     = 0x00,
    FUTURE  = 0x01, /* Only from the future, never intents that have expired. */
    OWN     = 0x02, /* Only intents owned by the agent. */
};

class IntentHandler
{
public:
    typedef std::map<std::string, std::map<unsigned int, Intent> > IntentTable;
    typedef std::map<std::string, std::map<unsigned int, SegmentView> > SegmentTable;
    typedef typename IntentTable::mapped_type IntentTableElem;

    struct Opts {
        IntentSelection filter;
        int n_intents;
        float time;
        std::string aid;
    };

    static const Opts select_all;

    IntentHandler(std::string aid);

    IntentTable getIntents(void) const;
    IntentTable getIntents(Opts opt, const IntentTable* exclude = nullptr) const;
    // IntentTable exchangeIntents(const IntentTable& intents, std::string agent_dst);
    void processRcvIntents(const IntentTable& isas);
    void createIntent(Intent i);
    void addNewIntent(Intent i, std::string aid);
    void updateIntent(Intent i, std::string aid);
    int getTotalIntentCount(void) const;
    int getIntentCount(std::string aid, float now) const;
    int getActiveIntentsAt(float t, std::string aid) const;
    bool isActiveAt(float t);
    float getLastIntentTime(std::string aid) const;

    /* Getters and setters: */
    const SegmentTable& getViews(void) const { return m_segments; }

private:
    typedef typename IntentTable::mapped_type::const_iterator IntentTableElemIt;

    std::string m_agent_id;
    IntentTable m_intents;
    SegmentTable m_segments;

    void addNewIntent(IntentTableElemIt ib, IntentTableElemIt ie, std::string aid);
};

inline IntentSelection operator&(IntentSelection lhs, IntentSelection rhs)
{
    return static_cast<IntentSelection>(static_cast<int>(lhs) & static_cast<int>(rhs));
}

inline IntentSelection& operator&=(IntentSelection& lhs, IntentSelection rhs)
{
    lhs = static_cast<IntentSelection>(static_cast<int>(lhs) & static_cast<int>(rhs));
    return lhs;
}

inline IntentSelection operator|(IntentSelection lhs, IntentSelection rhs)
{
    return static_cast<IntentSelection>(static_cast<int>(lhs) | static_cast<int>(rhs));
}

inline IntentSelection& operator|=(IntentSelection& lhs, IntentSelection rhs)
{
    lhs = static_cast<IntentSelection>(static_cast<int>(lhs) | static_cast<int>(rhs));
    return lhs;
}

#endif /* INTENT_HANDLER_HPP */
