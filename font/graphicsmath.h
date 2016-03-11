#pragma once

#include <list>
#include "position.h"
#include "configmanager.h"


namespace gMath{

	typedef Position point;
	std::list<point> addLine(float x1, float x2);
	double function(lua_State *L, double x);

};
