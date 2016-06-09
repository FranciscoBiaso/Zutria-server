local combat = createCombatObject()

setCombatParam(combat, COMBAT_PARAM_TYPE, COMBAT_FIREDAMAGE)
setCombatParam(combat, COMBAT_PARAM_EFFECT, 19)
setCombatParam(combat, COMBAT_PARAM_DISTANCEEFFECT, 1)
setCombatParam(combat, COMBAT_PARAM_CREATEITEM, 595)
setCombatParam(combat, COMBAT_PARAM_COORDINATE_SYSTEM, CARTESIAN_COORDINATE_SYSTEM)
setCombatParam(combat, COMBAT_PARAM_MAGIC_INTERVALS, 12)
setCombatParam(combat, COMBAT_PARAM_RANDOMGROWTH, TRUE)

-- those numbers represents the percentage of intensive fire
-- and how they grow with a big number, a big growth is did.
-- With a small number, small fires has big probability to grow.
stokerArea = {
{0,  0,  0,   0,  20,  0,  0,  0,  0},
{0,  0,  0,   30, 40,  30, 0,  0,  0},
{0,  0,  50,  60, 70,  60, 50, 0,  0},
{0,  30, 60,  80, 90,  80, 60, 30, 0},
{20, 40, 70,  90, 01,  90, 70, 40, 20},
{0,  30, 60,  80, 90,  80, 60, 30, 0},
{0,  0,  50,  60, 70,  60, 50, 0,  0},
{0,  0,  0,   30, 40,  30, 0,  0,  0},
{0,  0,  0,   0,  20,  0,  0,  0,  0},
}

local area = createCombatArea(stokerArea)
setCombatArea(combat, area)

function onCastSpell(cid, var)
	return doCombat(cid, combat, var)
end