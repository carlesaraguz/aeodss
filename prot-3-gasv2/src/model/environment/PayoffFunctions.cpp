/***********************************************************************************************//**
 *  Static class to provide various payoff functions.
 *  @class      PayoffFunctions
 *  @authors    Carles Araguz (CA), carles.araguz@upc.edu
 *  @date       2018-sep-11
 *  @version    0.1
 *  @copyright  This file is part of a project developed by Nano-Satellite and Payload Laboratory
 *              (NanoSat Lab) at Technical University of Catalonia - UPC BarcelonaTech.
 **************************************************************************************************/

#include "PayoffFunctions.hpp"
#include "Activity.hpp"

CREATE_LOGGER(PayoffFunctions)

/*  REMARKS:
 *  ------------------------------------------------------------------------------------------------
 *  EnvCellPayoffFunc === Payoff function for one cell:
 *  Arg. #0:                   pair<double, double>  --> t0 & t1 of the potential new activity.
 *  Arg. #1: vector<vector<pair<double, double> > >  --> vector of t0 & t1 of the activities for this cell.
 *  Arg. #2:          vector<shared_ptr<Activity> >  --> Pointer to the activities (same index than arg2).
 *  Returns: the (potential) partial payoff value for this cell.
 *  ------------------------------------------------------------------------------------------------
 *  EnvCellCleanFunc === Cleaning function for one cell:
 *  Arg. #0:                        double  --> The current time.
 *  Arg. #1: vector<shared_ptr<Activity> >  --> Pointer to all the activities.
 *  Return: the list of activities that can be safely "forgotten" (i.e. removed).
 *  ------------------------------------------------------------------------------------------------
 **/

PayoffFunctions::FunctionPair PayoffFunctions::f_revisit_time_forwards;
PayoffFunctions::FunctionPair PayoffFunctions::f_revisit_time_backwards;
PayoffFunctions::FunctionPair PayoffFunctions::f_coverage;
PayoffFunctions::FunctionPair PayoffFunctions::f_latency;

float PayoffFunctions::payoff(double rev_time)
{
    if(rev_time < 0.0) {
        Log::warn << "Computing payoff for negative revisit time of: " << VirtualTime::toString(rev_time, true)
            << ". This is unexpected but can be computed. Will continue.\n";
    }
    float po;
    switch(Config::payoff_model) {
        case PayoffModel::SIGMOID:
            po = 1.f / (1.f + std::exp(-Config::payoff_steepness * (rev_time - Config::goal_target)));
            return po;
            break;
        case PayoffModel::LINEAR:
            if(rev_time < Config::goal_min) {
                return 0.f;
            } else if(rev_time >= Config::goal_min && rev_time <= Config::goal_target) {
                po  = Config::payoff_mid * (rev_time - Config::goal_min);
                po /= Config::goal_target - Config::goal_min;
                return po;
            } else if(rev_time > Config::goal_target && rev_time <= Config::goal_max) {
                po  = (1.f - Config::payoff_mid) * (rev_time - Config::goal_target);
                po /= Config::goal_max - Config::goal_target;
                po += Config::payoff_mid;
                return po;
            } else if(rev_time > Config::goal_max) {
                return 1.f;
            }
            break;
    }
    Log::err << "Payoff computation error. Aborting.\n";
    throw std::runtime_error("Payoff computation error");
}

void PayoffFunctions::bindPayoffFunctions(void)
{
    Log::dbg << "Binding global payoff functions.\n";

    /* Revisit time forwards: ------------------------------------------------------------------- */
    f_revisit_time_forwards.first = [](PFArg0 ats, PFArg1 bts, PFArg2 bs) {
        /*  REMINDER:
         *  ats --> std::pair<double, dlouble> --> New activity's T_start and T_end.
         *  bts --> is a vector of vector of double pairs --> The start and end times of other activities.
         *  bs  --> vector of activities.
         */
        /* Find the first confirmed activity with t_start closer to the new activity's t_end. */
        double t_diff = -1.0;               /* Worst case revisit times. Used as a reference for payoff. */
        double t_diff_fallback = -1.0;      /* Helper. */
        double t_diff_i;                    /* Revisit time (single case, not worst). */
        double t_horizon = -1.0;            /* The start time of the reference activity. */
        double t_horizon_overlap = -1.0;    /* The last end time when there's only overlaping uncediced activities. */
        /* NOTE that in `t_horizon` we can't rely upon a pointer to the activity because it could
         * have multiple start and end timed (i.e. bts[i].size() > 1).
         **/
        double tsb;
        double teb, tea = ats.second;

        std::shared_ptr<Activity> next_act(nullptr);
        bool found_confirmed_fact = false;
        for(unsigned int i = 0; i < bs.size(); i++) {
            auto& b_act_ptr = bs[i];
            for(int j = bts[i].size() - 1; j >= 0; j--) {
                tsb = bts[i][j].first;
                teb = bts[i][j].second;
                t_diff_i = tsb - tea; /* Revisit time = t_start(existing) - t_end(new). */

                if(teb < tea) {
                    /*  These activity times (and the rest) are just irrelevant for forwards revisit time.
                     *  NOTE: we're iterating backwards (j--) and activity times should be sorted.
                     **/
                    break;
                }
                if(b_act_ptr->isFact() && b_act_ptr->isConfimed()) {
                    if(tea >= tsb && tea <= teb) {
                        /* These activities are overlapping. Revisit time is 0 and so is payoff. */
                        return payoff(0.0);
                    }
                    if( (t_diff == -1.0 && t_diff_i >= 0.0) ||                      /* <--- Is the first value. */
                        (t_diff > -1.0 && t_diff_i >= 0.0 && t_diff_i < t_diff)) {  /* <--- Is a min value. */

                        /* NOTE: We get here because
                         *  (1) The activity is not overlapping with another `confirmed` activity (previous if branch)
                         *  (2) This confirmed activity ends before the new one starts.
                         **/
                        found_confirmed_fact = true;
                        t_diff = t_diff_i;
                        next_act = b_act_ptr;
                        t_horizon = tsb;
                    } /* ... else, we don't care and need to continue. */

                } else if(!found_confirmed_fact && !b_act_ptr->isFact()) {
                    /* It's not a fact but we haven't found one yet. Discarded activities are ignored. */
                    if( (t_diff_fallback == -1.0 && t_diff_i >= 0.0) ||   /* <--- Is the first fallback value. */
                        (
                            t_diff_fallback > -1.0 &&           /* Previously found a fallback time. */
                            t_diff_i >= 0.0 &&                  /* This revisit time is backwards. */
                            t_diff_i > t_diff_fallback          /* This revisit time is greater than the one we found. */
                        )) {

                        t_diff_fallback = t_diff_i;
                        next_act = b_act_ptr;
                        t_horizon = tsb;
                    } else if(t_diff_fallback == -1.0) {
                        /* We haven't found a fact nor a suitable undecided. Check if this is overlapping: */
                        if(tsb <= tea && teb > tea) {
                            /* Overlaps and is undecided: */
                            t_horizon_overlap = std::max(t_horizon_overlap, teb);
                        }
                    } /* ... else, we don't care and need to continue. */
                }
            }
        }
        if(!found_confirmed_fact) {
            t_diff = t_diff_fallback;
        }
        /*  `next_act` is our previous confirmed fact. For revisit time backwards, we may ignore
         *  activities that:
         *      - End (t1) before `next_act`.
         *      - Start (t0) after `a`.
         **/
        if((!next_act || t_diff == -1.0) && t_horizon_overlap == -1.0) {
            /* There weren't activities to check with. Payoff is minimum. */
            return 0.f;
        } else {
            if(t_horizon == -1.0 && t_diff == -1.0 && t_horizon_overlap != -1.0) {
                t_horizon = t_horizon_overlap;
                t_diff = Config::goal_target * 1e3;  /* Very high RT --> po0=1. */
            }
            /*  Select and sort activities by their potential payoff (for this cell):
             *  The selected activities may contribute to _improving_ the revisit time, because they
             *  start before `next_act`. Nonetheless, they are not confirmed so we are not certain
             *  that they will actually improve RT.
             **/

            std::vector<std::pair<double, std::shared_ptr<Activity> > > selected;
            for(unsigned int i = 0; i < bs.size(); i++) {
                auto& b_act_ptr = bs[i];
                for(int j = bts[i].size() - 1; j >= 0; j--) {
                    tsb = bts[i][j].first;
                    teb = bts[i][j].second;
                    t_diff_i = tsb - tea;
                    if(next_act != b_act_ptr && !b_act_ptr->isDiscarded() && tsb < t_horizon) {
                        /* Is not the reference task, is undecided and starts before the reference horizon time: */
                        if(tsb > tea) {
                            /* Starts after the new task ends (i.e. yields revisit time > 0). */
                            /* Select this activity for payoff calculation, as is: */
                            selected.push_back(std::make_pair(tsb, b_act_ptr));
                            /*  Other times of activity `i` will not be considered.
                             *  NOTE: we're iterating backwards (j--) and activity times should be sorted.
                             **/
                            break;
                        } else if(tsb <= tea && teb > tea) {
                            /* Overlaps with the new task starts (i.e. yields revisit time = 0). */
                            /* Select this activity but change its start time so that we can compute payoff: */
                            selected.push_back(std::make_pair(tea, b_act_ptr));
                            break; /* See the note on the `break` of the other branch. */
                        }
                    }
                }
            }
            /* Sort selected activities: */
            using Elem = std::pair<double, std::shared_ptr<Activity> >;
            struct sort_by_start {
                inline bool operator() (const Elem& e1, const Elem& e2) {
                    return (e1.first > e2.first);   /* Later start times first. */
                }
            };
            std::sort(selected.begin(), selected.end(), sort_by_start());

            /* Compute differentially-weighted revisit time: */
            float po = payoff(t_diff); /* Revisit time backwards. This is the worst case. */
            float poi, dist_po;
            for(auto& s : selected) {
                tsb = s.first;
                poi = payoff(tsb - tea);  /* Must be smaller or equal than po. */
                if(poi > po) { /* TODO: Remove this after debugging. */
                    Log::err << "Error computing intermediate payoff values (F). Aborting.\n";
                    std::exit(-1);
                }
                dist_po = po - poi;
                po -= dist_po * s.second->getConfidence();
            }
            return po;
        }
    };
    f_revisit_time_forwards.second = [](CFArg0 t, CFArg1 as) {
        std::vector<std::shared_ptr<Activity> > retval;
        for(auto& ac_ptr : as) {
            if(t - ac_ptr->getEndTime() > Config::goal_target){
                retval.push_back(ac_ptr);
            }
        }
        return retval;
    };

    /* Revisit time backwards: ------------------------------------------------------------------ */
    f_revisit_time_backwards.first = [](PFArg0 ats, PFArg1 bts, PFArg2 bs) {
        /*  REMINDER:
         *  ats --> std::pair<double, dlouble> --> New activity's T_start and T_end.
         *  bts --> is a vector of vector of double pairs --> The start and end times of other activities.
         *  bs  --> vector of activities.
         */
        /* Find the first confirmed activity with t_end closer to the new activity's t_start. */
        double t_diff = -1.0;               /* Worst case revisit times. Used as a reference for payoff. */
        double t_diff_fallback = -1.0;      /* Helper. */
        double t_diff_i;                    /* Revisit time (single case, not worst). */
        double t_horizon = -1.0;            /* The end time of the reference activity. */
        double t_horizon_overlap = -1.0;    /* The first start time when there's only overlaping uncediced activities. */
        /* NOTE that in `t_horizon` we can't rely upon a pointer to the activity because it could
         * have multiple start and end timed (i.e. bts[i].size() > 1).
         **/
        double tsb, tsa = ats.first;
        double teb;

        std::shared_ptr<Activity> prev_act(nullptr);
        bool found_confirmed_fact = false;
        for(unsigned int i = 0; i < bs.size(); i++) {
            auto& b_act_ptr = bs[i];
            for(unsigned int j = 0; j < bts[i].size(); j++) {
                tsb = bts[i][j].first;
                teb = bts[i][j].second;
                t_diff_i = tsa - teb; /* Revisit time = t_start(new) - t_end(existing). */

                if(tsb > tsa) {
                    /* These activity times (and the rest) are just irrelevant for backwards revisit time. */
                    break;
                }
                if(b_act_ptr->isFact() && b_act_ptr->isConfimed()) {
                    if(tsa >= tsb && tsa <= teb) {
                        /* These activities are overlapping. Revisit time is 0 and so is payoff. */
                        return payoff(0.0);
                    }
                    if( (t_diff == -1.0 && t_diff_i >= 0.0) ||                      /* <--- Is the first value. */
                        (t_diff > -1.0 && t_diff_i >= 0.0 && t_diff_i < t_diff)) {  /* <--- Is a min value. */

                        /* NOTE: We get here because
                         *  (1) The activity is not overlapping with another `confirmed` activity (previous if branch)
                         *  (2) This confirmed activity ends before the new one starts.
                         **/
                        found_confirmed_fact = true;
                        t_diff = t_diff_i;
                        prev_act = b_act_ptr;
                        t_horizon = teb;
                    } /* ... else, we don't care and need to continue. */

                } else if(!found_confirmed_fact && !b_act_ptr->isFact()) {
                    /* It's not a fact but we haven't found one yet. Discarded activities are ignored. */
                    if( (t_diff_fallback == -1.0 && t_diff_i >= 0.0) ||   /* <--- Is the first fallback value. */
                        (
                            t_diff_fallback > -1.0 &&           /* Previously found a fallback time. */
                            t_diff_i >= 0.0 &&                  /* This revisit time is backwards. */
                            t_diff_i > t_diff_fallback          /* This revisit time is greater than the one we found. */
                        )) {

                        t_diff_fallback = t_diff_i;
                        prev_act = b_act_ptr;
                        t_horizon = teb;
                    } else if(t_diff_fallback == -1.0) {
                        /* We haven't found a fact nor a suitable undecided. Check if this is overlapping: */
                        if(tsb < tsa && teb >= tsa) {
                            /* Overlaps and is undecided: */
                            if(t_horizon_overlap == -1.0) {
                                t_horizon_overlap = tsb;
                            } else {
                                t_horizon_overlap = std::min(t_horizon_overlap, tsb);
                            }
                        }
                    } /* ... else, we don't care and need to continue. */
                }
            }
        }
        if(!found_confirmed_fact) {
            t_diff = t_diff_fallback;
        }
        /*  `prev_act` is our previous confirmed fact. For revisit time backwards, we may ignore
         *  activities that:
         *      - End (t1) before `prev_act`.
         *      - Start (t0) after `a`.
         **/
        if((!prev_act || t_diff == -1.0) && t_horizon_overlap == -1.0) {
            /* There weren't activities to check with. Payoff is maximum. */
            return 1.f;
        } else {
            if(t_horizon == -1.0 && t_diff == -1.0 && t_horizon_overlap != -1.0) {
                t_horizon = t_horizon_overlap;
                t_diff = Config::goal_target * 1e3;  /* Very high RT --> po0=1. */
            }
            /*  Select and sort activities by their potential payoff (for this cell):
             *  The selected activities may contribute to _improving_ the revisit time, because they
             *  end after `prev_act`. Nonetheless, they are not confirmed so we are not certain that
             *  they will actually improve RT.
             **/

            std::vector<std::pair<double, std::shared_ptr<Activity> > > selected;
            for(unsigned int i = 0; i < bs.size(); i++) {
                auto& b_act_ptr = bs[i];
                for(unsigned int j = 0; j < bts[i].size(); j++) {
                    tsb = bts[i][j].first;
                    teb = bts[i][j].second;
                    t_diff_i = tsa - teb;
                    if(prev_act != b_act_ptr && !b_act_ptr->isDiscarded() && teb > t_horizon) {
                        /* Is not the reference task, is undecided and ends after the reference horizon time: */
                        if(teb < tsa) {
                            /* Ends before the new task starts (i.e. yields revisit time > 0). */
                            /* Select this activity for payoff calculation, as is: */
                            selected.push_back(std::make_pair(teb, b_act_ptr));
                            break;  /* Other times of activity `i` will not be considered. */
                        } else if(tsb < tsa && teb >= tsa) {
                            /* Overlaps with the new task starts (i.e. yields revisit time = 0). */
                            /* Select this activity but change its end time so that we can compute payoff: */
                            selected.push_back(std::make_pair(tsa, b_act_ptr));
                            break;  /* Other times of activity `i` will not be considered. */
                        }
                    }
                }
            }
            /* Sort selected activities: */
            using Elem = std::pair<double, std::shared_ptr<Activity> >;
            struct sort_by_start {
                inline bool operator() (const Elem& e1, const Elem& e2) {
                    return (e1.first < e2.first);   /* Earlier end times first. */
                }
            };
            std::sort(selected.begin(), selected.end(), sort_by_start());

            /* Compute differentially-weighted revisit time: */
            float po = payoff(t_diff); /* Revisit time backwards. This is the worst case. */
            float poi, dist_po;
            for(auto& s : selected) {
                teb = s.first;
                poi = payoff(tsa - teb);  /* Must be smaller or equal than po. */
                if(poi > po) { /* TODO: Remove this after debugging. */
                    Log::err << "Error computing intermediate payoff values (B). Aborting.\n";
                    std::exit(-1);
                }
                dist_po = po - poi;
                po -= dist_po * s.second->getConfidence();
            }
            return po;
        }
    };
    f_revisit_time_backwards.second = [](CFArg0 t, CFArg1 as) {
        std::vector<std::shared_ptr<Activity> > retval;
        for(auto& ac_ptr : as) {
            if(t - ac_ptr->getEndTime() > Config::goal_target){
                retval.push_back(ac_ptr);
            }
        }
        return retval;
    };

    /* Coverage: -------------------------------------------------------------------------------- */
    f_coverage.first = [](PFArg0 /* ats */, PFArg1 /* bts */, PFArg2 /* bs */) {
        return Random::getUf(0.f, 1.f);
    };
    f_coverage.second = [](CFArg0 /* t */, CFArg1 /* as */) {
        return CFArg1();
    };

    /* Latency: --------------------------------------------------------------------------------- */
    f_latency.first = [](PFArg0 /* ats */, PFArg1 /* bts */, PFArg2 /* bs */) {
        return Random::getUf(0.f, 1.f);
    };
    f_latency.second = [](CFArg0 /* t */, CFArg1 /* as */) {
        return CFArg1();
    };
}
