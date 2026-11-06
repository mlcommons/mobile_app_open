#include <cmath>
#include <iostream>
#include <map>
#include <tuple>
#include <vector>

#ifndef _STABLE_DIFFUSION_SCHEDULING_UTIL_H_
#define _STABLE_DIFFUSION_SCHEDULING_UTIL_H_

std::vector<int> get_timesteps(int start, int stop, int delta);
std::tuple<std::vector<float>, std::vector<float>> get_initial_alphas(
    std::vector<int> timesteps);

#endif
