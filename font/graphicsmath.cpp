#include "graphicsmath.h"
extern ConfigManager g_config;
#define PI 3.141516

namespace gMath{

	void function(lua_State *L,double tetha, double *x, double *y)
	{
		int isNumber;

		lua_getglobal(L, "f");
		lua_pushnumber(L, tetha);

		if (lua_pcall(L, 1, 2, 0) != 0)
			std::cout << "error running function `f':" << lua_tostring(L, -1)<< std::endl;

		*y = lua_tonumber(L, -1);
		lua_pop(L, 1);
		*x = lua_tonumber(L, -1);
		lua_pop(L, 1);
	}

	std::list<point> addLine(float x1, float x2)
	{
		std::list<point> line;
		double x = -1, y = -1;
		for (int tetha = 0; tetha <= 3 * 360; tetha+=1)
		{
			double xOld, yOld;
			xOld = x;
			yOld = y;
			function(g_config.getLuaState(), tetha * PI / 180.0, &x, &y);
			if ((int)x != (int)xOld || (int)y != (int)yOld)
				line.push_front(point(x, y, 0));
		}
		return line;
	}

};