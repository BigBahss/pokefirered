Route28_House_MapScripts::
	.byte 0

Route28_House_EventScript_Woman::
	lock
	faceplayer
	waitse
	msgbox Route28_House_Text_Woman
	release
	end

Route28_House_EventScript_Fearow::
	lock
	faceplayer
	waitse
	playmoncry SPECIES_FEAROW, 0
	msgbox Route28_House_Text_Fearow
	waitmoncry
	release
	end
