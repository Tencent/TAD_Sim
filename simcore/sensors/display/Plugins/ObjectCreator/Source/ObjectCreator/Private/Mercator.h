#pragma once
#include "CoreMinimal.h"
#include <utility>

double tilesize();
std::pair<int, int> xy2tile(double x, double y);
std::pair<double, double> tilec(int x, int y);
std::pair<double, double> tileo(int x, int y);
