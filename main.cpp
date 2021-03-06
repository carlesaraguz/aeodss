/***********************************************************************************************//**
 *  Program entry point.
 *  @authors    Carles Araguz (CA), carles.araguz@upc.edu
 *  @date       2018-mar-20
 *  @version    0.1
 *  @copyright  This file is part of a project developed by Nano-Satellite and Payload Laboratory
 *              (NanoSat Lab) at Technical University of Catalonia - UPC BarcelonaTech.
 **************************************************************************************************/

#include "prot.hpp"

#include "Agent.hpp"
#include "AgentBuilder.hpp"
#include "MultiView.hpp"
#include "MessageBox.hpp"
#include "Init.hpp"

CREATE_LOGGER(main)

bool run_sandbox = true;

struct ControlInfo {
    bool planning;
    std::string agent_id;
};

std::timed_mutex mutex_draw;
std::timed_mutex mutex_control;
std::vector<std::shared_ptr<Agent> > agents;
std::vector<ControlInfo> control_info;
std::shared_ptr<World> world;
bool exit_draw_loop;
bool exit_control_loop;

/* Function headers: */
void handleEvents(sf::RenderWindow& w);
void testModePayoff(void);
void parseTLEFile(void);
void draw_loop(void);
void control_loop(void);

void draw_loop(void)
{
    Log::dbg << "Draw thread started.\n";
    sf::ContextSettings settings;
    settings.antialiasingLevel = 0;
    sf::RenderWindow window(sf::VideoMode(Config::win_width, Config::win_height), "Autonomous DSS Simulation Tool", sf::Style::Titlebar | sf::Style::Close, settings);
    window.setFramerateLimit(5);

    /* Configure Agent Views: */
    for(auto a : agents) {
        a->showResources(true);
    }
    agents[0]->displayActivities(ActivityDisplayType::ALL);
    agents[0]->showResources(true);
    agents[0]->getEnvironment()->buildView();

    /* Create multi-views: ---------------------------------------------------------------------- */
    std::vector<std::shared_ptr<const HasView> > avs(agents.begin(), agents.end());   /* Casts. */
    MultiView mv1, mv2, mv3, mv4;
    // mv1.setViews(avs);
    mv2.addViewToBack(agents[0]->getEnvironment());
    mv2.addViewToBack(std::static_pointer_cast<const HasView>(agents[0]->getActivityHandler()));
    mv2.addViewToBack(avs[0]);
    mv3.addViewToBack(world);
    mv4.addViewToBack(world);
    for(unsigned int i = 0; i < agents.size(); i++) {
        mv1.addViewToBack(std::static_pointer_cast<const HasView>(agents[i]->getLink()));
        mv1.addViewToBack(avs[i]);
        // mv1.addViewToBack(std::static_pointer_cast<const HasView>(agents[i]->getActivityHandler()));
        mv3.addViewToBack(avs[i]);
        mv4.addViewToBack(avs[i]);
    }

    mv1.setScale(0.5f, 0.5f);
    mv2.setScale(0.5f, 0.5f);
    mv3.setScale(0.5f, 0.5f);
    mv4.setScale(0.5f, 0.5f);
    mv2.setPosition(Config::win_width / 2.f, 0.f);
    mv3.setPosition(0.f, Config::win_height / 2.f);
    mv4.setPosition(Config::win_width / 2.f, Config::win_height / 2.f);

    /* Add some background: */
    sf::Texture world_map_texture;
    sf::Sprite world_map1, world_map2, world_map3, world_map4;
    if(!world_map_texture.loadFromFile(Config::root_path + "res/equirectangular_map_white_outline_transparent_background.png")) {
        Log::err << "Failed to load equirectangular map texture.\n";
        world_map1.setColor(sf::Color(0, 0, 0, 0));
        world_map2.setColor(sf::Color(0, 0, 0, 0));
        world_map3.setColor(sf::Color(0, 0, 0, 0));
        world_map4.setColor(sf::Color(0, 0, 0, 0));
    } else {
        world_map_texture.setSmooth(true);
        float scale_factor = (float)(Config::win_width / 2) / (float)world_map_texture.getSize().x;
        world_map1.setTexture(world_map_texture);
        world_map2.setTexture(world_map_texture);
        world_map3.setTexture(world_map_texture);
        world_map4.setTexture(world_map_texture);
        world_map1.setColor(sf::Color(255, 255, 255, 128)); /* Half transparent white. */
        world_map2.setColor(sf::Color(0, 0, 0, 128)); /* Half transparent black. */
        world_map3.setColor(sf::Color(0, 0, 0, 128)); /* Half transparent black. */
        world_map4.setColor(sf::Color(0, 0, 0, 128)); /* Half transparent black. */
        world_map1.setScale(scale_factor, scale_factor);
        world_map2.setScale(scale_factor, scale_factor);
        world_map3.setScale(scale_factor, scale_factor);
        world_map4.setScale(scale_factor, scale_factor);
        world_map2.setPosition(Config::win_width / 2.f, 0.f);
        world_map3.setPosition(0.f, Config::win_height / 2.f);
        world_map4.setPosition(Config::win_width / 2.f, Config::win_height / 2.f);
    }
    /* Create message box: */
    MessageBox msg;
    bool msg_show_time = true;
    msg.setMessage("0d00:00:00.000");
    msg.setPosition((Config::win_width - msg.getWidth()) / 2.f, (Config::win_height - msg.getHeight()) / 2.f);
    Log::dbg << "Draw loop will start now...\n";

    int draw_world = 5;
    while(window.isOpen() && !exit_draw_loop) {
        draw_world++;
        if(mutex_draw.try_lock_for(std::chrono::milliseconds(100))) {
            /* Message box: --------------------------------------------------------------------- */
            msg.setMessage(VirtualTime::toString(VirtualTime::now(), true, true));
            if(!msg_show_time) {
                msg.setMargin(5.f);
                msg.setPosition(
                    std::round((Config::win_width - msg.getWidth()) / 2.f),
                    std::round((Config::win_height - msg.getHeight()) / 2.f)
                );
                msg_show_time = true;
            }
            /* Pre-draw loop: ------------------------------------------------------------------- */
            mv1.drawViews();
            mv2.drawViews();
            if(draw_world >= 5) {
                draw_world = 0;
                world->display(World::Layer::REVISIT_TIME_ACTUAL);
                mv3.drawViews();
                world->display(World::Layer::REVISIT_TIME_UTOPIA);
                mv4.drawViews();
            }
            /* The rest is not dependent on objects managed by the control loop: */
            mutex_draw.unlock();

        } else {
            /* Message box: --------------------------------------------------------------------- */
            if(mutex_control.try_lock_for(std::chrono::milliseconds(10))) {
                bool waiting = false;
                std::string agent_list = "";
                for(auto ci : control_info) {
                    if(ci.planning) {
                        waiting = true;
                        if(agent_list.length() < 50) {
                            agent_list += " " + ci.agent_id;
                        } else {
                            break;
                        }
                    }
                }
                if(waiting) {
                    msg.setMessage("WAITING FOR..." + agent_list);
                    msg.setMargin(10.f);
                    msg.setPosition(
                        std::round((Config::win_width - msg.getWidth()) / 2.f),
                        std::round((Config::win_height - msg.getHeight()) / 2.f)
                    );
                    msg_show_time = false;
                }
                mutex_control.unlock();
            }
        }
        /* Draw loop: --------------------------------------------------------------------------- */
        window.clear();
        window.draw(mv1);
        window.draw(mv2);
        window.draw(mv3);
        window.draw(mv4);

        window.draw(world_map1);
        window.draw(world_map2);
        window.draw(world_map3);
        window.draw(world_map4);
        window.draw(msg);
        window.display();

        /* Event loop: -------------------------------------------------------------------------- */
        handleEvents(window);
    }
    exit_control_loop = true;
    Log::dbg << "Exiting draw thread.\n";
}

void control_loop(void)
{
    mutex_draw.lock();
    mutex_control.lock();
    Log::dbg << "Control loop started...\n";

    /* Create agents: --------------------------------------------------------------------------- */
    if(Config::load_agents_from_yaml) {
        AgentBuilder agent_builder;
        Log::dbg << "Loading agents from YAML file (\'" << Config::system_yml << "\')...\n";
        auto abset = agent_builder.load(Config::system_yml);
        for(auto ab : abset) {
            if(agents.size() < Config::n_agents) {
                auto aptr = std::make_shared<Agent>(&ab);
                agents.push_back(aptr);
                control_info.push_back({false, aptr->getId()});
            } else {
                break;
            }
        }
        if(agents.size() != Config::n_agents) {
            Log::err << "Error loading agents from YAML file. Wrong number of agents. " << Config::n_agents << " agents were expected.\n";
            Log::err << "Agents found in file: " << abset.size() << ". Agents successfully loaded: " << agents.size() << ".\n";
            std::exit(3);
        }
    } else {
        for(unsigned int i = 0; i < Config::n_agents; i++) {
            AgentBuilder ab;
            ab.generateAndStore("A" + std::to_string(i));
            auto aptr = std::make_shared<Agent>(&ab);
            agents.push_back(aptr);
            control_info.push_back({false, aptr->getId()});
        }
    }

    /* Configure agent links: ------------------------------------------------------------------- */
    for(auto a : agents) {
        a->getLink()->setAgents(agents);
    }

    /* Create a global aggregated environment model: -------------------------------------------- */
    world = std::make_shared<World>();
    world->addAgent(agents);

    std::thread thread_draw;
    if(Config::enable_graphics) {
        Log::dbg << "Starting draw thread.\n";
        exit_draw_loop = false;
        thread_draw = std::thread(draw_loop);
    }
    mutex_draw.unlock();
    mutex_control.unlock();
    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    Log::dbg << "Starting control loop.\n";
    ReportSet::getInstance().outputAllHeaders();
    int update_world_metrics = 0;

    while(!VirtualTime::finished() && !exit_control_loop) {
        /* Update loop: ------------------------------------------------------------------------- */
        mutex_control.lock();
        if(run_sandbox) {
            mutex_control.unlock();
            mutex_draw.lock();     /* Synchronise with draw loop. */

            /* Define a lambda for the plan function (as a wrapper): */
            auto agent_plan = [] (const std::shared_ptr<Agent>& a, int i) {
                mutex_control.lock();
                control_info[i].planning = true;
                mutex_control.unlock();

                a->plan(); /* May start GA scheduler. */

                mutex_control.lock();
                control_info[i].planning = false;
                mutex_control.unlock();
            };
            /* Step time: */
            VirtualTime::step();

            /* Step agents: */
            std::for_each(agents.begin(), agents.end(), [](const std::shared_ptr<Agent>& a) { a->updatePosition(); });
            if(Config::parallel_planners >= 2) {
                #pragma omp parallel for num_threads(Config::parallel_planners)
                for(unsigned int i = 0; i < agents.size(); i++) {
                    agent_plan(agents.at(i), i);
                }
            } else {
                for(unsigned int i = 0; i < agents.size(); i++) {
                    agent_plan(agents.at(i), i);
                }
            }
            if(Config::parallel_agent_step) {
                std::for_each(agents.begin(), agents.end(), [](const std::shared_ptr<Agent>& a) { a->stepSequential(); });
                #pragma omp parallel for
                for(unsigned int i = 0; i < agents.size(); i++) {
                    agents[i]->stepParallel();
                }
            } else {
                std::for_each(agents.begin(), agents.end(), [](const std::shared_ptr<Agent>& a) { a->step(); });
            }

            /*  Step world:
             *  Note that this updates world cells with new revisit times (but does not move them to
             *  graphical objects).
             **/
            world->step();
            mutex_draw.unlock();   /* System can be drawn now. */

            /* Report world values: */
            if(update_world_metrics % 10 == 0) {
                if(Config::enable_graphics) {
                    std::this_thread::sleep_for(std::chrono::milliseconds(100));
                }
                world->computeMetrics();    /* This only reports to file. */
                ReportSet::getInstance().outputAll();
            } else {
                if(Config::enable_graphics) {
                    std::this_thread::sleep_for(std::chrono::milliseconds(1));
                }
            }
            update_world_metrics++;
        } else {
            mutex_control.unlock();
            /* Yield CPU. */
            if(Config::enable_graphics) {
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            }
        }
    }
    world->computeMetrics(true);    /* Make the last measurements. */
    ReportSet::getInstance().outputAll();
    if(Config::enable_graphics) {
        exit_draw_loop = true;
        thread_draw.join();
    }
    if(VirtualTime::finished()) {
        Log::dbg << "Simulation reached end time. Exiting control thread.\n";
    } else {
        Log::dbg << "Stopping simulation by user request. Exiting control thread.\n";
    }
}

int main(int argc, char** argv)
{
    Init::doInit();
    Config::loadCmdArgs(argc, argv);
    Init::createOutputDirectories();

    if(Config::mode != SandboxMode::SIMULATE) {
        switch(Config::mode) {
            case SandboxMode::TEST_PAYOFF:
                testModePayoff();
                std::exit(0);
                break;
            case SandboxMode::PARSE_TLE_FILE:
                parseTLEFile();
                std::exit(0);
            case SandboxMode::SIMULATE:
            case SandboxMode::RANDOM:
            default:
                break;
        }
    }

    std::vector<std::shared_ptr<Agent> >* release_resources = new std::vector<std::shared_ptr<Agent> >();
    try {
        exit_control_loop = false;
        std::thread thread_control(control_loop);
        thread_control.join();
    } catch(const std::exception& e) {
        /* Attempt to release resources: */
        agents.swap(*release_resources);
        delete release_resources;
        Log::err << "Attempted to exit clean after a fatal failure:\n";
        Log::err << e.what() << "\n";
    }
    return 0;
}

void handleEvents(sf::RenderWindow& w)
{
    sf::Event event;
    while(w.pollEvent(event)) {
        if(event.type == sf::Event::Closed) {
            w.close();
        }
        if(event.type == sf::Event::KeyPressed) {
            if(event.key.code == sf::Keyboard::Escape) {
                Log::dbg << "The escape key was pressed. Exiting." << std::endl;
                w.close();
            }
            if(event.key.code == sf::Keyboard::Space) {
                mutex_control.lock();
                if(run_sandbox) {
                    Log::dbg << "Stopping world.\n";
                    run_sandbox = false;
                } else {
                    Log::dbg << "Resuming world.\n";
                    run_sandbox = true;
                }
                mutex_control.unlock();
            }
        }
    }
}

void parseTLEFile(void)
{
    Log::dbg << "**************************************************\n";
    Log::dbg << "-- Entering TLE file parse mode for: \'" << Config::tle_file << "\'...\n";
    std::ifstream tle_file(Config::tle_file, std::ios::in);
    if(tle_file.is_open()) {
        bool valid = true;
        int line_count = 0;
        std::vector<TLE> tle_collection;
        std::string tle0, tle1, tle2;
        char line[100];
        while(tle_file.getline(line, 100)) {
            if(line_count == 0) {
                tle0 = Utils::trim(std::string(line));
            } else if(line_count == 1) {
                tle1 = Utils::trim(std::string(line));
            } else if(line_count == 2) {
                tle2 = Utils::trim(std::string(line));
                TLE tle_obj;
                try {
                    tle_obj.setTLE(tle0, tle1, tle2);
                    Utils::removeWhitespace(tle_obj.sat_name);
                    tle_collection.push_back(tle_obj);
                    Log::dbg << "-- Parsing " << tle_collection.back().sat_name << " done.\n";
                } catch(const std::exception& e) {
                    Log::err << "-- Unable to parse TLE. Aborting...\n";
                    Log::err << "-- LINE0: \'" << tle0 << "\'\n";
                    Log::err << "-- LINE1: \'" << tle1 << "\'\n";
                    Log::err << "-- LINE2: \'" << tle2 << "\'\n";
                    valid = false;
                    break;
                }
            }
            line_count++;
            line_count %= 3;
        }
        if(tle_collection.size() == 0) {
            Log::warn << "-- Parsing the file returned 0 TLE objects.\n";
        } else {
            Log::dbg << "-- Parsing the file returned " << tle_collection.size() << " TLE objects.\n";
            for(auto& tle_obj : tle_collection) {
                AgentBuilder ab(tle_obj);
            }
        }
        if(valid) {
            Log::dbg << "-- Parsing completed. Results are stored in \'" << Config::data_path << "system.yml\'\n";
        } else {
            Log::warn << "-- Some TLE lines could not be parsed.\n";
            Log::warn << "-- Results in \'" << Config::data_path << "system.yml\' may be incomplete.\n";
        }
    } else {
        Log::err << "-- Unable to open TLE collection file: \'" << Config::tle_file << "\'. Check permissions and path.\n";
    }
    Log::dbg << "**************************************************\n";
}

void testModePayoff(void)
{
    Log::dbg << "**************************************************\n";
    Log::dbg << "-- Entering test mode: PAYOFF...\n";
    constexpr float min_payoff = 1e-6;
    constexpr float min_display_payoff = 1e-2;
    int max_iter = 100000;
    Log::dbg << "-- Stopping when payoff delta < " << min_payoff << ", or iterations > " << max_iter << ".\n";
    Log::dbg << "-- Displaying points for delta min = " << min_display_payoff << ".\n";
    switch(Config::payoff_model) {
        case PayoffModel::SIGMOID:
            Log::dbg << "-- Payoff model: SIGMOID.\n";
            Log::dbg << "-- kg = " << Config::payoff_steepness << ".\n";
            break;
        case PayoffModel::LINEAR:
            Log::dbg << "-- Payoff model: LINEAR.\n";
            Log::dbg << "-- Pmid = " << Config::payoff_mid << ".\n";
            Log::dbg << "-- Gmin = " << Config::goal_min << ".\n";
            Log::dbg << "-- Gmax = " << Config::goal_max << ".\n";
            break;
        case PayoffModel::CONSTANT_SLOPE:
            Log::dbg << "-- Payoff model: CONSTANT_SLOPE.\n";
            Log::dbg << "-- Gmin  = " << Config::goal_min << ".\n";
            Log::dbg << "-- Slope = " << Config::payoff_slope << ".\n";
            Log::dbg << "-- Max. payoff   = " << Config::max_payoff << ".\n";
            Log::dbg << "-- Sched. window = " << Config::agent_planning_window << " = "
                << VirtualTime::toString(Config::agent_planning_window * Config::time_step, false) << ".\n";
            Log::dbg << "-- Interl. wind. = " << Config::agent_replanning_window << " = "
                << VirtualTime::toString(Config::agent_replanning_window * Config::time_step, false) << ".\n";
            if(Config::goal_min > Config::time_step) {
                max_iter = (int)((Config::duration + Config::goal_min) / Config::time_step);
            } else {
                max_iter = (int)(Config::duration / Config::time_step);
            }
            Log::dbg << "-- Stopping at " << max_iter << " iterations.\n";
            break;
        case PayoffModel::QUADRATIC:
            Log::dbg << "-- Payoff model: QUADRATIC.\n";
            Log::dbg << "-- Gmin = " << Config::goal_min << ".\n";
            break;
    }
    int i = 0;
    float payoff = 0.f, delta_payoff, prev_disp_po = 0.f, rt;
    do {
        rt = i * Config::time_step;
        delta_payoff = payoff;
        payoff = PayoffFunctions::payoff(rt);
        delta_payoff = payoff - delta_payoff;
        if(prev_disp_po == 0.f || payoff - prev_disp_po >= min_display_payoff) {
            Log::dbg << "-- " << std::fixed << std::setw(6) << i;
            Log::dbg << ": Revisit time: [" << std::setprecision(6) << rt << "] = " << VirtualTime::toString(rt, false);
            Log::dbg << " ---> Payoff: " << std::setprecision(6) << payoff << "\n";
            prev_disp_po = payoff;
        }
        i++;
    } while((rt < Config::goal_target || delta_payoff > min_payoff) && i < max_iter);
    Log::dbg << "-- Test completed.\n";
    Log::dbg << "**************************************************\n";
}
