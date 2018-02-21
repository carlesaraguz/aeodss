/***********************************************************************************************//**
 *  Program entry point.
 *  @authors    Carles Araguz (CA), carles.araguz@upc.edu
 *  @date       2018-feb-20
 *  @version    0.1
 *  @copyright  This file is part of a project developed by Nano-Satellite and Payload Laboratory
 *              (NanoSat Lab) at Technical University of Catalonia - UPC BarcelonaTech.
 **************************************************************************************************/

#include "prot.hpp"

#include "Satellite.hpp"

int main(int argc, char** argv)
{
    std::string root_path = Utils::getRootPath();
    if(argc >= 2) {
        Config::n_sats = std::strtol(argv[1], nullptr, 10);
    } else {
        Config::n_sats = 1;
    }

    std::cout << "Root path: " << root_path << "\n";

    std::vector<Satellite> sats;
    std::vector<std::ofstream> sat_files;
    sats.reserve(Config::n_sats);
    // sat_files.reserve(Config::n_sats);
    std::stringstream sat_filename;
    for(int i = 0; i < Config::n_sats; i++) {
        /* Add a new Satellite: */
        sats.push_back(i);

        /* Create its data file: */
        sat_filename << root_path << "data/satdata_" << std::fixed << std::setfill('0') << std::setw(4) << i << ".csv";
        sat_files.push_back(std::move(std::ofstream(sat_filename.str())));

        /* Initialize data file: */
        if(sat_files[sat_files.size() - 1].fail()) {
            std::cout << sat_filename.str() << " -- error\n";
        } else {
            std::cout << sat_filename.str() << "\n";
        }
        sat_filename.str("");
        sat_files[sat_files.size() - 1] << "Sat, Time, Rx, Ry, Rz\n";
        sat_files[sat_files.size() - 1] << std::scientific << std::setprecision(4);
    }

    double sim_time = 0.0;
    Vector3D psat;

    while(sim_time <= Config::max_time) {
        for(size_t i = 0; i < sats.size(); i++) {
            psat = sats[i].setDays(sim_time / 86400.0);
            sat_files[i] << i << ", ";
            sat_files[i] << std::setw(11) << sim_time << ", ";
            sat_files[i] << std::setw(11) << psat.x << ", ";
            sat_files[i] << std::setw(11) << psat.y << ", ";
            sat_files[i] << std::setw(11) << psat.z << "\n";
        }
        sim_time += Config::time_step;
    }

    for(size_t i = 0; i < sat_files.size(); i++) {
        sat_files[i] << std::defaultfloat << std::endl;
        sat_files[i].close();
    }

    return 0;
}
