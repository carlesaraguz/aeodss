/***********************************************************************************************//**
 *  Geo-spatial data and simulation metrics.
 *  @class      HeatMap
 *  @authors    Carles Araguz (CA), carles.araguz@upc.edu
 *  @date       2019-may-19
 *  @version    0.1
 *  @copyright  This file is part of a project developed by Nano-Satellite and Payload Laboratory
 *              (NanoSat Lab) at Technical University of Catalonia - UPC BarcelonaTech.
 **************************************************************************************************/

#ifndef HEAT_MAP_HPP
#define HEAT_MAP_HPP

#include "prot.hpp"
// #include "mat.h"        /* Matlab library. */
// #include <string.h>     /* Dependencies of Matlab library. */
#include "ReportGenerator.hpp"

class HeatMap : public ReportGenerator
{
public:
    HeatMap(std::string name, Aggregate type);
    ~HeatMap(void);

    void setType(Aggregate hmt) { m_hm_type = hmt; }
    void setRevisitTime(unsigned int x, unsigned int y, double rt);
    void saveHeatMap(void);
    static unsigned int getLongitudeDimension(void);
    static unsigned int getLatitudeDimension(void);

private:
    Aggregate m_hm_type;        /* Max or average. */
    double** m_values;          /* Matrix of values. */
    unsigned int** m_count;     /* Matrix of counts. */
    static const unsigned int m_lng_range = (1800);   /* Dimensions of matrix. */
    static const unsigned int m_lat_range = (900);    /* Dimensions of matrix. */
};

#endif /* HEAT_MAP_HPP */
