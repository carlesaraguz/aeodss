/***********************************************************************************************//**
 *  Graphical representation of a set of activities.
 *  @class      ActivityHandlerView
 *  @authors    Carles Araguz (CA), carles.araguz@upc.edu
 *  @date       2018-mar-21
 *  @version    0.1
 *  @copyright  This file is part of a project developed at Nano-Satellite and Payload Laboratory
 *              (NanoSat Lab), Technical University of Catalonia - UPC BarcelonaTech.
 **************************************************************************************************/

#ifndef ACTIVITY_HANDLER_VIEW_HPP
#define ACTIVITY_HANDLER_VIEW_HPP

#include "prot.hpp"
#include "SegmentView.hpp"
#include "Activity.hpp"

enum class ActivityDisplayType {
    NONE,
    ALL,
    OWNED,
    CONFIRMED,
    BY_AGENT,
    SET
};

class ActivityHandlerView : public sf::Drawable
{
public:
    ActivityHandlerView(std::string aid = "unknown");
    void setOwnActivityList(std::vector<std::shared_ptr<Activity> >* alist_ptr) { m_act_own_ptr = alist_ptr; }
    void setOthersActivityList(std::map<std::string, std::vector<std::shared_ptr<Activity> > > const * alist_ptr) { m_act_others_ptr = alist_ptr; }
    void display(ActivityDisplayType adt, std::vector<std::pair<std::string, unsigned int> > filter = { });
    void update(void);
    void setAgentId(std::string aid) { m_agent_id = aid; }


private:
    std::string m_agent_id;
    ActivityDisplayType m_act_disp_type;
    std::vector<std::pair<std::string, unsigned int> > m_filter;

    std::vector<std::shared_ptr<SegmentView> > m_segments;
    std::vector<std::shared_ptr<Activity> > const * m_act_own_ptr;
    std::map<std::string, std::vector<std::shared_ptr<Activity> > > const * m_act_others_ptr;

    void draw(sf::RenderTarget& target, sf::RenderStates states) const override;
};

#endif /* ACTIVITY_HANDLER_VIEW_HPP */
