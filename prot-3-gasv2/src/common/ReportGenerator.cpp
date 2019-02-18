/***********************************************************************************************//**
 *  Interface for classes that can report values in output files.
 *  @class      ReportGenerator
 *  @authors    Carles Araguz (CA), carles.araguz@upc.edu
 *  @date       2019-jan-10
 *  @version    0.1
 *  @copyright  This file is part of a project developed at Nano-Satellite and Payload Laboratory
 *              (NanoSat Lab), Technical University of Catalonia - UPC BarcelonaTech.
 **************************************************************************************************/

#include "ReportGenerator.hpp"

CREATE_LOGGER(ReportGenerator)

ReportGenerator::ReportGenerator(std::string name, bool publish)
    : m_row_count(0)
    , m_report_filename(Config::data_path + name)
    , m_enabled(false)
{
    if(publish) {
        ReportSet::getInstance().publish(this);
    }
}

ReportGenerator::~ReportGenerator(void)
{
    if(m_report_file.is_open()) {
        m_report_file.close();
    }
    Log::dbg << "Output data file generated: " << m_report_filename << "\n";
}

void ReportGenerator::enableReport(void)
{
    if(!m_report_file.is_open()) {
        m_report_file.open(m_report_filename, std::ios_base::out);
        if(!m_report_file.is_open()) {
            Log::err << "Error creating file " << m_report_filename << "\n";
        }
    }
    m_enabled = true;
}

void ReportGenerator::disableReport(void)
{
    if(m_report_file.is_open()) {
        m_report_file << std::flush;
        m_report_file.close();
    }
    m_enabled = false;
}

unsigned int ReportGenerator::addReportColumn(std::string colname)
{
    m_column_names.push_back(colname);
    m_column_values.push_back("");
    return m_column_names.size() - 1;
}

void ReportGenerator::setReportColumnValue(unsigned int col_idx, std::string value)
{
    if(!m_enabled) {
        return;
    }
    try {
        m_column_values.at(col_idx) = value;
    } catch(const std::exception& e) {
        Log::err << "Failed to access column index " << col_idx << " to set its value.\n";
        throw std::runtime_error("Wrong column index in report file");
    }
}

void ReportGenerator::setReportColumnValue(std::string col_name, std::string value)
{
    if(!m_enabled) {
        return;
    }
    auto it = std::find(m_column_names.begin(), m_column_names.end(), col_name);
    if(it != m_column_names.end()) {
        *it = value;
    } else {
        Log::err << "Failed to access column index named \'" << col_name << "\' to set its value.\n";
        throw std::runtime_error("Wrong column name in report file");
    }
}

void ReportGenerator::setReportColumnValue(unsigned int col_idx, float value)
{
    std::stringstream ss;
    ss << value;
    setReportColumnValue(col_idx, ss.str());
}

void ReportGenerator::setReportColumnValue(std::string col_name, float value)
{
    std::stringstream ss;
    ss << value;
    setReportColumnValue(col_name, ss.str());
}

void ReportGenerator::outputReport(void)
{
    if(m_enabled && m_report_file.is_open()) {
        m_report_file << std::fixed << std::setprecision(6) << VirtualTime::now() << ",";
        for(auto c = m_column_values.begin(); c != m_column_values.end(); c++) {
            m_report_file << *c;
            if(std::next(c) != m_column_values.end()) {
                m_report_file << ",";
            } else {
                m_report_file << std::endl;
            }
            *c = ""; /* Clears this value. */
        }
        m_report_file << std::flush;
        if(++m_row_count >= 50) {
            flush();
            m_row_count = 0;
        }
    }
}

void ReportGenerator::flush(void)
{
    if(!m_enabled) {
        return;
    }
    if(m_report_file.is_open()) {
        m_report_file.close();
    }
    m_report_file.open(m_report_filename, std::ios::app);
}

void ReportGenerator::outputReportHeader(void)
{
    if(m_enabled && m_report_file.is_open()) {
        m_report_file << "t,";
        for(auto c = m_column_names.begin(); c != m_column_names.end(); c++) {
            m_report_file << *c;
            if(std::next(c) != m_column_names.end()) {
                m_report_file << ",";
            } else {
                m_report_file << "\n";
            }
        }
        m_report_file << std::flush;
    }
}
