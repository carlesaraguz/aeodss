/***********************************************************************************************//**
 *  Program initialization and setup class.
 *  @class      Init
 *  @authors    Carles Araguz (CA), carles.araguz@upc.edu
 *  @date       2018-feb-20
 *  @version    0.1
 *  @copyright  This file is part of a project developed by Nano-Satellite and Payload Laboratory
 *              (NanoSat Lab) at Technical University of Catalonia - UPC BarcelonaTech.
 **************************************************************************************************/

#include "Init.hpp"

void Init::doInit(void)
{
    Random::doInit();

    Config::fnt_monospace.loadFromFile("/usr/share/fonts/truetype/ubuntu-font-family/UbuntuMono-R.ttf");
    Config::fnt_normal.loadFromFile("/usr/share/fonts/truetype/ubuntu-font-family/Ubuntu-L.ttf");
    Config::fnt_awesome.loadFromFile("/usr/share/fonts/opentype/font-awesome/FontAwesome.otf");
    Config::color_orange = sf::Color(255, 153, 102);
    Config::color_dark_green = sf::Color(35, 94, 92);

    std::map<float, sf::Color> gradient;
    gradient[0.f]  = sf::Color::Red;
    gradient[0.5f] = sf::Color::Green;
    gradient[1.f]  = sf::Color::Blue;
    Config::color_gradient_rgb.setGradient(gradient);
    gradient.clear();
    gradient[0.f]  = sf::Color(186,  49,  37, 255);     /* Red.         */
    gradient[0.5f] = sf::Color(  5, 123, 191, 255);     /* Blue.        */
    gradient[1.f]  = sf::Color(  0, 132,  70, 255);     /* Green.       */
    Config::color_gradient_rbg.setGradient(gradient);
    gradient.clear();
    gradient[0.f]   = sf::Color::Black;
    gradient[0.05f] = sf::Color(186,  49,  37, 255);    /* Red.         */
    gradient[0.5f]  = sf::Color(  5, 123, 191, 255);    /* Blue.        */
    gradient[1.f]   = sf::Color(  0, 132,  70, 255);    /* Green.       */
    Config::color_gradient_krbg.setGradient(gradient);
    gradient.clear();
    gradient[0.f]  = sf::Color(173,   0,   0, 255);     /* Dark red.    */
    gradient[0.5f] = sf::Color(255, 165,   0, 255);     /* Orange.      */
    gradient[1.f]  = sf::Color(  0, 123, 255, 255);     /* Sky blue.    */
    Config::color_gradient_1.setGradient(gradient);
    gradient.clear();
    gradient[0.f]  = sf::Color( 13,  50,  99, 255);     /* Dark blue.   */
    gradient[0.5f] = sf::Color( 64, 143, 247, 255);     /* Light blue.  */
    gradient[1.f]  = sf::Color(219, 234, 255, 255);     /* Pale blue.   */
    Config::color_gradient_blue.setGradient(gradient);
    gradient.clear();

    // Config::max_task_duration = -Config::max_capacity / (Config::capacity_restore - Config::capacity_consume);
    Config::max_task_duration = 100;
    std::cout << "Maximum task duration: " << Config::max_task_duration << " t-steps.\n";

    if(Config::ga_crossover_points > Config::max_tasks - 1) {
        Config::ga_crossover_points = Random::getUi(1, Config::max_tasks - 1);
        std::cerr << "GA Scheduler: crossover points changed to " << Config::ga_crossover_points << ".\n";
    }

    // Config::root_path = getRootPath();
    // std::cout << "Process root path: " << Config::root_path << "\n";
}

std::string Init::getRootPath(void)
{
    int fds[2], err;
    char full_path[500];
    std::string retval;

    if((err = pipe(fds)) != 0) {
        std::cerr << "Init error: unable to create a pipe.\n";
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
        std::exit(-1);

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
        std::vector<std::string> path_parts = Utils::split(Utils::trim(std::string(full_path)), '/');
        std::ostringstream oss;
        std::for_each(path_parts.begin(), std::prev(path_parts.end(), 2), [&oss] (std::string str) {
            if(!str.empty()) {
                oss << "/" << str;
            }
        });
        retval = oss.str() + "/";
    } else {
        /* Error. */
        std::cerr << "Init error: unable to fork.\n";
    }
    return retval;
}
