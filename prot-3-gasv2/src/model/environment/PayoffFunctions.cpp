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
 *  Arg. #0:                   pair<float, float>  --> t0 & t1 of the potential new activity.
 *  Arg. #1: vector<vector<pair<float, float> > >  --> vector of t0 & t1 of the activities for this cell.
 *  Arg. #2:        vector<shared_ptr<Activity> >  --> Pointer to the activities (same index than arg2).
 *  Returns: the (potential) partial payoff value for this cell.
 *  ------------------------------------------------------------------------------------------------
 *  EnvCellCleanFunc === Cleaning function for one cell:
 *  Arg. #0:                         float  --> The current time.
 *  Arg. #1: vector<shared_ptr<Activity> >  --> Pointer to all the activities.
 *  Return: the list of activities that can be safely "forgotten" (i.e. removed).
 *  ------------------------------------------------------------------------------------------------
 **/

PayoffFunctions::FunctionPair PayoffFunctions::f_revisit_time_forwards;
PayoffFunctions::FunctionPair PayoffFunctions::f_revisit_time_backwards;
PayoffFunctions::FunctionPair PayoffFunctions::f_coverage;
PayoffFunctions::FunctionPair PayoffFunctions::f_latency;

float PayoffFunctions::revisit_time_norm(float rev_time)
{
    if(rev_time < 0.f) {
        return 1.f;
    }
    if(rev_time <= Config::target_revisit_time) {
        return Config::min_payoff;
    } else if(rev_time >= Config::max_revisit_time) {
        return Config::max_payoff;
    } else {
        float delta = (Config::max_payoff - Config::min_payoff) / (Config::max_revisit_time - Config::target_revisit_time);
        return delta * (rev_time - Config::target_revisit_time) + Config::min_payoff;
    }
}

void PayoffFunctions::bindPayoffFunctions(void)
{
    Log::dbg << "Binding global payoff functions.\n";

    /* Revisit time forwards: ------------------------------------------------------------------- */
    f_revisit_time_forwards.first = [](PFArg0 /* ats */, PFArg1 /* bts */, PFArg2 /* bs */) {
        return Random::getUf(0.f, 1.f);
    };
    f_revisit_time_forwards.second = [](CFArg0 /* t */, CFArg1 /* as */) {
        return CFArg1();
    };

    /* Revisit time backwards: ------------------------------------------------------------------ */
    f_revisit_time_backwards.first = [](PFArg0 ats, PFArg1 bts, PFArg2 bs) {
        /* Find the first confirmed activity with t_end closer to the new activity's t_start. */
        float t_diff = -1.f;
        // Log::warn << "Activity* " << a->getId() << " has times {" << ats.first << ", " << ats.second << "}.\n";

        std::shared_ptr<Activity> prev_act(nullptr);
        for(unsigned int i = 0; i < bs.size(); i++) {
            auto& b_act_ptr = bs[i];
            for(unsigned int j = 0; j < bts[i].size(); j++) {
                if(b_act_ptr->isFact() && b_act_ptr->isConfimed()) {
                    float t_diff_i = ats.first - bts[i][j].second;
                    // Log::warn << "Activity " << b_act_ptr->getId() << " has times {" << bts[i][j].first << ", " << bts[i][j].second << "}.\n";
                    if(t_diff == -1.f && t_diff_i >= 0.f) {
                        t_diff = t_diff_i;
                        prev_act = b_act_ptr;
                        // Log::warn << "Activity " << b_act_ptr->getId() << " is now the first previous activity (t_diff = " << t_diff << ").\n";
                    } else if(t_diff > -1.f && t_diff_i >= 0.f && t_diff_i < t_diff) {
                        t_diff = t_diff_i;
                        prev_act = b_act_ptr;
                        // Log::warn << "Activity " << b_act_ptr->getId() << " is now the previous activity (t_diff = " << t_diff << ").\n";
                    } /* ... else, we don't care. */
                }
            }
        }
        // if(prev_act) Log::warn << "t_diff = " << t_diff << ": " << *prev_act << "\n";
        /*  `prev_act` is our previous confirmed fact. For revisit time backwards, we may ignore
         *  activities that:
         *      - End (t1) before `prev_act`.
         *      - Start (t0) after `a`.
         **/
        float rvtb = revisit_time_norm(t_diff); /* Revisit time backwards. This is the worst case (normalized). */
        if(!prev_act || t_diff == -1.f) {
            // Log::warn << "There weren't facts to check with. Payoff is maximum.\n";
            return rvtb;         /* NOTE: Come back to this later. */
        } else {
            /*  Select and sort activities by their potential payoff (for this cell):
             *  The selected activities may contribute to _improving_ the revisit time, because they
             *  end after `prev_act`. Nonetheless, they are not confirmed so we are not certain that
             *  they will actually improve RT.
             **/
            std::vector<std::pair<float, std::shared_ptr<Activity> > > selected;
            for(unsigned int i = 0; i < bs.size(); i++) {
                auto& b_act_ptr = bs[i];
                for(unsigned int j = 0; j < bts[i].size(); j++) {
                    float t_diff_i = ats.first - bts[i][j].second;
                    if(t_diff_i >= 0.f && t_diff_i < t_diff && prev_act != b_act_ptr && !b_act_ptr->isDiscarded()) {
                        /* Select this activity for payoff calculation: */
                        selected.push_back(std::make_pair(revisit_time_norm(t_diff_i), b_act_ptr));
                        // Log::warn << "Activity " << *b_act_ptr << " has been selected and has t_diff_i = "
                        //     << t_diff_i << " (" << revisit_time_norm(t_diff_i) << " norm.)\n";
                    }
                }
            }
            /* Sort selected activities: */
            using Elem = std::pair<float, std::shared_ptr<Activity> >;
            struct sort_by_tdiff {
                inline bool operator() (const Elem& e1, const Elem& e2) {
                    return (e1.first > e2.first);
                }
            };
            std::sort(selected.begin(), selected.end(), sort_by_tdiff());

            /* Compute differentially-weighted revisit time: */
            for(auto& s : selected) {
                // Log::warn << "Iterating: rvtb = " << rvtb;
                rvtb -= (rvtb - revisit_time_norm(s.first)) * s.second->getConfidence();
                // Log::warn << " --> " << rvtb << "\n";
            }
            return rvtb;
        }
    };
    f_revisit_time_backwards.second = [](CFArg0 /* t */, CFArg1 /* as */) {
        return CFArg1();
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
