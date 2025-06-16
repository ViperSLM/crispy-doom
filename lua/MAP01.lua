-- MAP01 Test script
--LoadScript("DOOM")

function OnMapLoad()
	print("Hello from MAP01!")
	--Doom.G_ExitLevel()
end

-- Activates when switch is pressed
-- Parameters (passed from Doom Engine):
-- 1. LineDef tag number

MAP01 = {}
MAP01.Switches = {
	DoorTag = 1,
	DoorCode = {true, false, false, true, true},
	CorrectCode = function(input)
		local self = MAP01.Switches
		if #input ~= #self.DoorCode then -- Same length?
			return false
		end

		-- Compare input
		for i = 1, #self.DoorCode do
			if input[i] ~= self.DoorCode[i] then
				return false
			end
		end

		return true
	end,

	CheckDoor = function()
		local self = MAP01.Switches
		local result = {
			self.Switch1.state,
			self.Switch2.state,
			self.Switch3.state,
			self.Switch4.state,
			self.Switch5.state
		}

		if (self.CorrectCode(result)) then
			Doom.SectorLightLevel(7, 255)
			Doom.OpenDoor(1)
		else
			Doom.SectorLightLevel(7, 192)
			Doom.CloseDoor(1)
		end

		-- Match?


		-- if result == self.DoorCode then
		-- 	Doom.OpenDoor(1)
		-- else
		-- 	Doom.CloseDoor(1)
		-- end
	end,

	Switch1 = {
		state = false,
		tag = 2,
		Toggle = function(tag)
			local self = MAP01.Switches.Switch1
			if tag == self.tag then
				if self.state == false then
				Doom.SectorLightLevel(self.tag, 255)
				Doom.PrintHUD("Button toggled ON")
				self.state = true
			else
				Doom.SectorLightLevel(self.tag, 128)
				Doom.PrintHUD("Button toggled OFF")
				self.state = false
			end
			end
		end,
	},

	Switch2 = {
		state = false,
		tag = 3,
		Toggle = function(tag)
			local self = MAP01.Switches.Switch2
			if tag == self.tag then
				if self.state == false then
				Doom.SectorLightLevel(self.tag, 255)
				Doom.PrintHUD("Button toggled ON")
				self.state = true
			else
				Doom.SectorLightLevel(self.tag, 128)
				Doom.PrintHUD("Button toggled OFF")
				self.state = false
			end
			end
		end,
	},

	Switch3 = {
		state = false,
		tag = 4,
		Toggle = function(tag)
			local self = MAP01.Switches.Switch3
			if tag == self.tag then
				if self.state == false then
				Doom.SectorLightLevel(self.tag, 255)
				Doom.PrintHUD("Button toggled ON")
				self.state = true
			else
				Doom.SectorLightLevel(self.tag, 128)
				Doom.PrintHUD("Button toggled OFF")
				self.state = false
			end
			end
		end,
	},

	Switch4 = {
		state = false,
		tag = 5,
		Toggle = function(tag)
			local self = MAP01.Switches.Switch4
			if tag == self.tag then
				if self.state == false then
				Doom.SectorLightLevel(self.tag, 255)
				Doom.PrintHUD("Button toggled ON")
				self.state = true
			else
				Doom.SectorLightLevel(self.tag, 128)
				Doom.PrintHUD("Button toggled OFF")
				self.state = false
			end
			end
		end,
	},

	Switch5 = {
		state = false,
		tag = 6,
		Toggle = function(tag)
			local self = MAP01.Switches.Switch5
			if tag == self.tag then
				if self.state == false then
				Doom.SectorLightLevel(self.tag, 255)
				Doom.PrintHUD("Button toggled ON")
				self.state = true
			else
				Doom.SectorLightLevel(self.tag, 128)
				Doom.PrintHUD("Button toggled OFF")
				self.state = false
			end
			end
		end,
	},
}

local d2music = {
	DoomEnums.Music.mus_dm2ttl,
	DoomEnums.Music.mus_dm2int,
	DoomEnums.Music.mus_read_m,
	DoomEnums.Music.mus_runnin,
	DoomEnums.Music.mus_stalks,
	DoomEnums.Music.mus_countd,
	DoomEnums.Music.mus_betwee,
	DoomEnums.Music.mus_doom,
	DoomEnums.Music.mus_the_da,
	DoomEnums.Music.mus_shawn,
	DoomEnums.Music.mus_ddtblu,
	DoomEnums.Music.mus_in_cit,
	DoomEnums.Music.mus_dead,
	DoomEnums.Music.mus_romero,
	DoomEnums.Music.mus_messag,
	DoomEnums.Music.mus_ampie,
	DoomEnums.Music.mus_adrian,
	DoomEnums.Music.mus_tense,
	DoomEnums.Music.mus_openin,
	DoomEnums.Music.mus_evil,
	DoomEnums.Music.mus_ultima,
}

function OnSwitchActivate(tag)
	Doom.PrintHUD("Linedef Tag: " .. tag)

	MAP01.Switches.Switch1.Toggle(tag)
	MAP01.Switches.Switch2.Toggle(tag)
	MAP01.Switches.Switch3.Toggle(tag)
	MAP01.Switches.Switch4.Toggle(tag)
	MAP01.Switches.Switch5.Toggle(tag)
	MAP01.Switches.CheckDoor()

	if tag == 0 then
		-- local rng = math.random(62, 96)
		-- Doom.S_ChangeMusic(rng, true)
		local actorx,actory,actorz,angle,mobjtype,memaddr = Doom.P_SpawnMobj(568,-260,DoomEnums.MobjType.MT_POSSESSED)
		local npc = DoomStructs.Mobj.new(actorx,actory,actorz,angle,mobjtype,memaddr)

		npc.printInfo()
	end
end
