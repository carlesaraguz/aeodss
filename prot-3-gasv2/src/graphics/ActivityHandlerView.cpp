/***********************************************************************************************//**
 *  Graphical representation of a set of activities.
 *  @class      ActivityHandlerView
 *  @authors    Carles Araguz (CA), carles.araguz@upc.edu
 *  @date       2018-mar-21
 *  @version    0.1
 *  @copyright  This file is part of a project developed at Nano-Satellite and Payload Laboratory
 *              (NanoSat Lab), Technical University of Catalonia - UPC BarcelonaTech.
 **************************************************************************************************/

#include "ActivityHandlerView.hpp"

ActivityHandlerView::ActivityHandlerView(std::string aid)
    : m_act_disp_type(ActivityDisplayType::ALL)
    , m_act_own_ptr(nullptr)
    , m_act_others_ptr(nullptr)
    , m_agent_id(aid)
{ }

void ActivityHandlerView::update(void)
{
    if(m_act_own_ptr == nullptr || m_act_others_ptr == nullptr) {
        return;
    }
    std::vector<std::shared_ptr<SegmentView> > segs;
    std::shared_ptr<SegmentView> segv_ptr;
    switch(m_act_disp_type) {
        case ActivityDisplayType::NONE:
            break;
        case ActivityDisplayType::ALL:
            for(auto& act : *m_act_own_ptr) {
                segv_ptr = act->getView();
                if(segv_ptr != nullptr) {
                    segs.push_back(segv_ptr);
                }
            }
            for(auto& actmap : *m_act_others_ptr) {
                for(auto& act : actmap.second) {
                    segv_ptr = act->getView();
                    if(segv_ptr != nullptr) {
                        segs.push_back(segv_ptr);
                    }
                }
            }
            break;
        case ActivityDisplayType::OWNED:
            for(auto& act : *m_act_own_ptr) {
                segv_ptr = act->getView();
                if(segv_ptr != nullptr) {
                    segs.push_back(segv_ptr);
                }
            }
            break;
        case ActivityDisplayType::CONFIRMED:
            for(auto& act : *m_act_own_ptr) {
                if(act->isConfimed()) {
                    segv_ptr = act->getView();
                    if(segv_ptr != nullptr) {
                        segs.push_back(segv_ptr);
                    }
                }
            }
            for(auto& actmap : *m_act_others_ptr) {
                for(auto& act : actmap.second) {
                    if(act->isConfimed()) {
                        segv_ptr = act->getView();
                        if(segv_ptr != nullptr) {
                            segs.push_back(segv_ptr);
                        }
                    }
                }
            }
            break;
        case ActivityDisplayType::BY_AGENT:
            for(auto& f_elem : m_filter) {
                if(f_elem.first == m_agent_id) {
                    for(auto& act : *m_act_own_ptr) {
                        segv_ptr = act->getView();
                        if(segv_ptr != nullptr) {
                            segs.push_back(segv_ptr);
                        }
                    }
                } else {
                    /* Look if this agent is in the map: */
                    if(m_act_others_ptr->find(f_elem.first) != m_act_others_ptr->end()) {
                        /* if it is, add all its segments: */
                        for(auto& act : m_act_others_ptr->at(f_elem.first)) {
                            segv_ptr = act->getView();
                            if(segv_ptr != nullptr) {
                                segs.push_back(segv_ptr);
                            }
                        }
                    }
                }
            }
            break;
        case ActivityDisplayType::SET:
            for(auto& act : *m_act_own_ptr) {
                if(std::find(m_filter.begin(), m_filter.end(), std::make_pair(act->getAgentId(), act->getId())) != m_filter.end()) {
                    segv_ptr = act->getView();
                    if(segv_ptr != nullptr) {
                        segs.push_back(segv_ptr);
                    }
                }
            }
            for(auto& actmap : *m_act_others_ptr) {
                for(auto& act : actmap.second) {
                    if(std::find(m_filter.begin(), m_filter.end(), std::make_pair(actmap.first, act->getId())) != m_filter.end()) {
                        segv_ptr = act->getView();
                        if(segv_ptr != nullptr) {
                            segs.push_back(segv_ptr);
                        }
                    }
                }
            }
            break;
    }
    m_segments.swap(segs);
}
void ActivityHandlerView::display(ActivityDisplayType adt, std::vector<std::pair<std::string, unsigned int> > filter)
{
    m_act_disp_type = adt;
    m_filter = filter;
}

void ActivityHandlerView::draw(sf::RenderTarget& target, sf::RenderStates states) const
{
    for(auto& segment_ptr : m_segments) {
        target.draw(*segment_ptr, states);
    }
}
