#ifndef solver_h
#define solver_h

#include <iostream>
#include <vector>
#include <map>
#include <string>
#include <set>
#include "placer.h"

//this function is used to call c functions. This makes it easier to write up
// make file
#ifdef ECF
void APSolve(std::vector<int>& ArrAp, std::vector<int>& ArrAi,
	std::vector<dataType>& ArrAx, std::vector<dataType>& Arrb,
	std::vector<dataType>& ArrX, int n);
#endif
#endif // !solver_h
