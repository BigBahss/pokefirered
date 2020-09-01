MtSilver_PokemonCenter_1F_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, MtSilver_PokemonCenter_1F_OnTransition
	map_script MAP_SCRIPT_ON_RESUME, CableClub_OnResume
	.byte 0

MtSilver_PokemonCenter_1F_OnTransition::
	setrespawn SPAWN_MT_SILVER_EXTERIOR
	end

MtSilver_PokemonCenter_1F_EventScript_Nurse::
	lock
	faceplayer
	call EventScript_PkmnCenterNurse
	release
	end

Script_IVChecker::
	lock
	faceplayer
	getpartymonname 0, 0
	msgbox Text_IVChecker_1, MSGBOX_DEFAULT
	specialvar VAR_RESULT, GetHpIV
	getnumberstring 0, VAR_RESULT
	specialvar VAR_RESULT, GetAtkIV
	getnumberstring 1, VAR_RESULT
	specialvar VAR_RESULT, GetDefIV
	getnumberstring 2, VAR_RESULT
	msgbox Text_IVChecker_2, MSGBOX_DEFAULT
	specialvar VAR_RESULT, GetSpAtkIV
	getnumberstring 0, VAR_RESULT
	specialvar VAR_RESULT, GetSpDefIV
	getnumberstring 1, VAR_RESULT
	specialvar VAR_RESULT, GetSpeedIV
	getnumberstring 2, VAR_RESULT
	msgbox Text_IVChecker_3, MSGBOX_DEFAULT
	release
	end

Text_IVChecker_1:
	.string "Your {STR_VAR_1}...$"

Text_IVChecker_2:
	.string "Its HP IV is {STR_VAR_1}.\p"
	.string "Its Attack IV is {STR_VAR_2}.\p"
	.string "Its Defense IV is {STR_VAR_3}.$"

Text_IVChecker_3:
	.string "Its Special Attack IV is {STR_VAR_1}.\p"
	.string "Its Special Defense IV is {STR_VAR_2}.\p"
	.string "Its Speed IV is {STR_VAR_3}.\n"
	.string "You're welcome.$"
