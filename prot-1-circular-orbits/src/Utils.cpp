/***********************************************************************************************//**
 *  Static class with utilities.
 *  @class      Utils
 *  @authors    Carles Araguz (CA), carles.araguz@upc.edu
 *  @date       2018-mmm-00
 *  @version    0.1
 *  @copyright  This file is part of a project developed by Nano-Satellite and Payload Laboratory
 *              (NanoSat Lab) at Technical University of Catalonia - UPC BarcelonaTech.
 **************************************************************************************************/

#include "Utils.hpp"


std::random_device Utils::m_rand_dev;
std::default_random_engine Utils::m_rand_engine = std::default_random_engine(m_rand_dev());
std::uniform_real_distribution<double> Utils::m_uniform_dist;

double Utils::genRandom(double a, double b)
{
    if(a >= b) {
        return (a - b) * m_uniform_dist(m_rand_engine) + b;
    } else {
        return (b - a) * m_uniform_dist(m_rand_engine) + a;
    }
}

double Utils::genRandom(void)
{
    return genRandom(0.0, 1.0);
}

std::string Utils::getRootPath(void)
{
    int fds[2], err;
    char full_path[500];
    std::string retval;

    if((err = pipe(fds)) != 0) {
        std::cerr << "Unable to create a pipe.";
        return "";
    }

    pid_t ppid = getpid();
    pid_t cpid = fork();
    if(cpid == 0) {
        /* Child */
        close(1);           /* Close stdout.                    */
        err = dup(fds[1]);  /* Redirect stdout to pipe_W.       */
        close(fds[0]);      /* Close unused File descriptor.    */
        close(fds[1]);      /* Close unused File descriptor.    */

        std::string readlink_arg = "/proc/" + std::to_string(ppid) + "/exe";
        execlp("readlink", "readlink", readlink_arg.c_str(), (char  *)nullptr);
        exit(-1);

    } else if(cpid > 0) {
        /* Parent */
        close(fds[1]);  /* Close unused File descriptor.    */
        int c = 0;
        while(read(fds[0], full_path + c, 1) == 1 && c < 500) {
            if(full_path[c] == 10 || full_path[c] == 12) {
                full_path[c] = ' ';
                break;
            }
            c++;
        }
        waitpid(cpid, nullptr, 0);
        std::vector<std::string> path_parts = StringUtils::split(StringUtils::trim(std::string(full_path)), '/');
        std::ostringstream oss;
        std::for_each(path_parts.begin(), std::prev(path_parts.end(), 2), [&oss] (std::string str) {
            if(!str.empty()) {
                oss << "/" << str;
            }
        });
        retval = oss.str() + "/";
    } else {
        /* Error. */
        std::cerr << "Unable to fork.";
    }
    return retval;
}
