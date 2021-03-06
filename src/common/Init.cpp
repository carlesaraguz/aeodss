/***********************************************************************************************//**
 *  Program initialization and setup class.
 *  @class      Init
 *  @authors    Carles Araguz (CA), carles.araguz@upc.edu
 *  @date       2018-feb-20
 *  @version    0.1
 *  @copyright  This file is part of a project developed at Nano-Satellite and Payload Laboratory
 *              (NanoSat Lab), Technical University of Catalonia - UPC BarcelonaTech.
 **************************************************************************************************/

#include "Init.hpp"

CREATE_LOGGER(Init)

void Init::doInit(void)
{
    LogStream::setLogLevel(LogStream::Level::DEBUG);

    Random::doInit();

    Config::fnt_monospace.loadFromFile("/usr/share/fonts/truetype/ubuntu/UbuntuMono-R.ttf");
    Config::fnt_normal.loadFromFile("/usr/share/fonts/truetype/ubuntu/Ubuntu-L.ttf");
    Config::fnt_awesome.loadFromFile("/usr/share/fonts/opentype/font-awesome/FontAwesome.otf");

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
    gradient[0.0f]  = sf::Color(255, 255, 255, 255);    /* White.       */
    gradient[0.25f] = sf::Color(237,  52,  52, 255);    /* Red.         */
    gradient[0.5f]  = sf::Color(255, 236,  38, 255);    /* Yellow.      */
    gradient[0.75f] = sf::Color( 51, 221,  85, 255);    /* Green.       */
    gradient[1.0f]  = sf::Color( 27,  68, 214, 255);    /* Blue.        */
    Config::color_gradient_rainbow.setGradient(gradient);
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

    if(Config::ga_crossover_points > Config::max_tasks - 1) {
        Config::ga_crossover_points = Random::getUi(1, Config::max_tasks - 1);
        Log::warn << "GA Scheduler: crossover points changed to " << Config::ga_crossover_points << ".\n";
    }

    Config::root_path = getRootPath();
    Log::dbg << "Process root path: " << Config::root_path << "\n";

    PayoffFunctions::bindPayoffFunctions();

    omp_set_nested(Config::parallel_nested);
}

void Init::createOutputDirectories(void)
{
    Config::data_path = createDataDir();
    Log::dbg << "Results directory: " << Config::data_path << "\n";
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

std::string Init::createDataDir(void)
{
    std::string data_path;
    if(Config::create_data_dirname) {
        if(Config::root_path.length() <= 0) {
            Log::err << "Unable to create data directory without a root path.\n";
            throw std::runtime_error("Unable to create data directory without a root path");
        }
        /* Q&D version. To be improved if necessary in the future: */
        std::time_t t = std::time(nullptr);
        char str[100];
        std::strftime(str, sizeof(str), "%Y_%m_%d_%H%M%S", std::localtime(&t));
        int rn = Random::getUi(10, 99);
        std::string path = std::string(str) + "_" + std::to_string(rn);
        if(Config::simulation_name.length() == 0) {
            path += "/";
        } else {
            path += "_" + Config::simulation_name + "/";
        }
        data_path = Config::root_path + "data/" + path;
    } else {
        data_path = Config::simulation_name;
    }
    std::string cmd = "mkdir -p " + data_path;
    if(std::system(cmd.c_str()) != 0) {
        Log::err << "Unable to create data directory: " << data_path << ". Check permissions.\n";
        return "";
    }
    std::string cp_cmd = "cp " + Config::conf_file + " " + data_path;
    if(std::system(cp_cmd.c_str()) != 0) {
        Log::err << "Unable to copy the configuration file in the data directory. Check permissions.\n";
        Log::err << "This command failed: \'" << cp_cmd << "\'\n";
    }
    return data_path;
}
