local combat = createCombatObject()
setCombatParam(combat, COMBAT_PARAM_DISTANCEEFFECT, 0)
setCombatParam(combat, COMBAT_PARAM_CREATEITEM, FIREEXPLOSION_ITEM)
setCombatParam(combat, COMBAT_PARAM_AGGRESSIVE, 0)

function onCastSpell(cid, var)
	return doCombat(cid, combat, var)
end
