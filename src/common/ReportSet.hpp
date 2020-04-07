/***********************************************************************************************//**
 *  A singleton that stores all instances of ReportGenerator objects.
 *  @class      ReportSet
 *  @authors    Carles Araguz (CA), carles.araguz@upc.edu
 *  @date       2019-jan-11
 *  @version    0.1
 *  @copyright  This file is part of a project developed at Nano-Satellite and Payload Laboratory
 *              (NanoSat Lab), Technical University of Catalonia - UPC BarcelonaTech.
 **************************************************************************************************/

#ifndef REPORT_SET
#define REPORT_SET

#include "prot.hpp"

class ReportGenerator;

class ReportSet
{
public:
    static ReportSet& getInstance(void);
    void outputAll(void);
    void outputAllHeaders(void);
    void publish(ReportGenerator* rg);

private:
    std::vector<ReportGenerator*> m_publish_list;

    ReportSet(void) = default;
};

#endif /* REPORT_SET */
