-- MAP01 Test script

function OnMapLoad()
	--print("Hello from MAP01!")
	--Doom.G_ExitLevel()
end

-- Activates when switch is pressed
-- Parameters (passed from Doom Engine):
-- 1. LineDef tag number

function OnSwitchActivate(tag)
	print("Hello World!")
	Doom.G_ExitLevel()
end

-- bruh
