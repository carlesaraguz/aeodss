function [po, side] = payoff_linear(mid_val, min_rt, max_rt, goal_rt, rt)
    po  = (rt < min_rt) * 0;
    po += (rt >= min_rt & rt <= max_rt & rt < goal_rt)  .* (mid_val * (rt - min_rt) ./ (goal_rt - min_rt));
    po += (rt >= min_rt & rt <= max_rt & rt >= goal_rt) .* (mid_val + (1 - mid_val) * (rt - goal_rt) ./ (max_rt - goal_rt));
    po += (rt > max_rt) * 1;
    side = rt > goal_rt;
endfunction
