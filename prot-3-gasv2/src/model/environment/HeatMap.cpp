/***********************************************************************************************//**
 *  Geo-spatial data and simulation metrics.
 *  @class      HeatMap
 *  @authors    Carles Araguz (CA), carles.araguz@upc.edu
 *  @date       2019-may-19
 *  @version    0.1
 *  @copyright  This file is part of a project developed by Nano-Satellite and Payload Laboratory
 *              (NanoSat Lab) at Technical University of Catalonia - UPC BarcelonaTech.
 **************************************************************************************************/

#include "HeatMap.hpp"

CREATE_LOGGER(HeatMap)

HeatMap::HeatMap(std::string name, Aggregate type)
    : ReportGenerator(name, false)
    , m_hm_type(type)
    , m_lng_range(900)
    , m_lat_range(450)
{
    m_values = new double*[m_lng_range];
    m_count  = new unsigned int*[m_lng_range];
    for(unsigned int xx = 0; xx < m_lng_range; xx++) {
        addReportColumn("x" + std::to_string(xx));
        m_values[xx] = new double[m_lat_range];
        m_count[xx]  = new unsigned int[m_lat_range];
        for(unsigned int yy = 0; yy < m_lat_range; yy++) {
            m_values[xx][yy] = -1.0;
            m_count[xx][yy]  = 0;
        }
    }
    enableReport();
}

HeatMap::~HeatMap(void)
{
    for(unsigned int xx = 0; xx < m_lng_range; xx++) {
        delete[] m_values[xx];
        delete[] m_count[xx];
    }
    delete[] m_values;
    delete[] m_count;
}


void HeatMap::setRevisitTime(unsigned int x, unsigned int y, double rt)
{
    if(x < m_lng_range && y < m_lat_range) {
        switch(m_hm_type) {
            case Aggregate::MAX_VALUE:
                m_values[x][y] = std::max(m_values[x][y], rt);
                break;
            case Aggregate::MIN_VALUE:
                if(m_values[x][y] < 0.0) {
                    m_values[x][y] = rt;
                } else {
                    m_values[x][y] = std::min(m_values[x][y], rt);
                }
                break;
            case Aggregate::MEAN_VALUE:
            case Aggregate::SUM_VALUE:
            case Aggregate::COUNT:
                if(m_values[x][y] < 0.0) {
                    m_values[x][y] = rt;
                } else {
                    m_values[x][y] += rt;
                }
                break;
        }
        m_count[x][y]++;
    } else {
        Log::err << "Fatal error accessing a heatmap value: (" << x << "," << y << ")\n";
        std::exit(1);
    }
}

void HeatMap::saveHeatMap(void)
{
    #if 0
    MATFile *pmat;
    mxArray *pa1, *pa2, *pa3;
    std::vector<int> myInts;
    myInts.push_back(1);
    myInts.push_back(2);

    double data[9] = { 1.0, 4.0, 7.0, 2.0, 5.0, 8.0, 3.0, 6.0, 9.0 };
    const char *file = "mattest.mat";
    char str[256];
    int status;

    printf("Creating file %s...\n\n", file);
    pmat = matOpen(file, "w");

    pa2 = mxCreateDoubleMatrix(3,3,mxREAL);
    memcpy((void *)(mxGetPr(pa2)), (void *)data, sizeof(data));
    status = matPutVariableAsGlobal(pmat, "GlobalDouble", pa2);

    /* clean up */
    mxDestroyArray(pa2);
    matClose(pmat);
    #endif

    truncateReport();
    for(unsigned int yy = 0; yy < m_lat_range; yy++) {
        for(unsigned int xx = 0; xx < m_lng_range; xx++) {
            if(m_hm_type == Aggregate::COUNT) {
                setReportColumnValue(xx, (double)m_count[xx][yy]);
            } else {
                if(m_hm_type == Aggregate::MEAN_VALUE && m_count[xx][yy] > 0) {
                    setReportColumnValue(xx, (double)(m_values[xx][yy] / (double)m_count[xx][yy]));
                } else if(m_hm_type != Aggregate::MEAN_VALUE && m_count[xx][yy] > 0) {
                    setReportColumnValue(xx, (double)m_values[xx][yy]);
                } else if(m_count[xx][yy] == 0 || m_values[xx][yy] == -1.0) {
                    setReportColumnValue(xx, -1);
                }
            }
        }
        outputReport(yy == m_lat_range - 1);
    }
}
