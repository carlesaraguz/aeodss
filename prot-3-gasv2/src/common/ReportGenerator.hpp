/***********************************************************************************************//**
 *  Interface for classes that can report values in output files.
 *  @class      ReportGenerator
 *  @authors    Carles Araguz (CA), carles.araguz@upc.edu
 *  @date       2019-jan-10
 *  @version    0.1
 *  @copyright  This file is part of a project developed at Nano-Satellite and Payload Laboratory
 *              (NanoSat Lab), Technical University of Catalonia - UPC BarcelonaTech.
 **************************************************************************************************/

#ifndef REPORT_GENERATOR_HPP
#define REPORT_GENERATOR_HPP

#include "prot.hpp"
#include "ReportSet.hpp"

class ReportGenerator
{
public:
    ReportGenerator(std::string name, bool publish = true);
    ~ReportGenerator(void);

    unsigned int addReportColumn(std::string colname);
    void setReportColumnValue(unsigned int col_idx, std::string value);
    void setReportColumnValue(std::string col_name, std::string value);
    void setReportColumnValue(unsigned int col_idx, float value);
    void setReportColumnValue(std::string col_name, float value);
    void outputReport(void);
    void outputReportHeader(void);
    void enableReport(void);
    void disableReport(void);

private:
    std::vector<std::string> m_column_names;
    std::vector<std::string> m_column_values;
    int m_row_count;
    std::string m_report_filename;
    std::ofstream m_report_file;
    bool m_enabled;

    void flush(void);
};

#endif /* REPORT_GENERATOR_HPP */
