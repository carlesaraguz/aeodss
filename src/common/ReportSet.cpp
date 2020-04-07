/***********************************************************************************************//**
 *  A singleton that stores all instances of ReportGenerator objects.
 *  @class      ReportSet
 *  @authors    Carles Araguz (CA), carles.araguz@upc.edu
 *  @date       2019-jan-11
 *  @version    0.1
 *  @copyright  This file is part of a project developed at Nano-Satellite and Payload Laboratory
 *              (NanoSat Lab), Technical University of Catalonia - UPC BarcelonaTech.
 **************************************************************************************************/

#include "ReportSet.hpp"
#include "ReportGenerator.hpp"

ReportSet& ReportSet::getInstance(void)
{
    static ReportSet rs;
    return rs;
}

void ReportSet::outputAll(void)
{
    for(auto& rg : m_publish_list) {
        rg->outputReport();
    }
}

void ReportSet::outputAllHeaders(void)
{
    for(auto& rg : m_publish_list) {
        rg->outputReportHeader();
    }
}

void ReportSet::publish(ReportGenerator* rg)
{
    if(rg != nullptr) {
        m_publish_list.push_back(rg);
    }
}
