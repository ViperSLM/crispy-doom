-- Global tables
DoomEnums = {}
DoomStructs = {}

-- Music enum
DoomEnums.Music = {
    mus_None = 0,
    -- Doom
    mus_e1m1 = 1,mus_e1m2 = 2,mus_e1m3 = 3,mus_e1m4 = 4,
    mus_e1m5 = 5,mus_e1m6 = 6,mus_e1m7 = 7,mus_e1m8 = 8,
    mus_e1m9 = 9,mus_e2m1 = 10,mus_e2m2 = 11,mus_e2m3 = 12,
    mus_e2m4 = 13,mus_e2m5 = 14,mus_e2m6 = 15,mus_e2m7 = 16,
    mus_e2m8 = 17,mus_e2m9 = 18,mus_e3m1 = 19,mus_e3m2 = 20,
    mus_e3m3 = 21,mus_e3m4 = 22,mus_e3m5 = 23,mus_e3m6 = 24,
    mus_e3m7 = 25,mus_e3m8 = 26,mus_e3m9 = 27,
    -- Doom Episode 4
    mus_e4m1 = 28,mus_e4m2 = 29,mus_e4m3 = 30,mus_e4m4 = 31,
    mus_e4m5 = 32,mus_e4m6 = 33,mus_e4m7 = 34,mus_e4m8 = 35,
    mus_e4m9 = 36,
    -- Sigil
    mus_e5m1 = 37,mus_e5m2 = 38,mus_e5m3 = 39,mus_e5m4 = 40,
    mus_e5m5 = 41,mus_e5m6 = 42,mus_e5m7 = 43,mus_e5m8 = 44,
    mus_e5m9 = 45,
    -- Sigil II
    mus_e6m1 = 46,mus_e6m2 = 47,mus_e6m3 = 48,mus_e6m4 = 49,
    mus_e6m5 = 50,mus_e6m6 = 51,mus_e6m7 = 52,mus_e6m8 = 53,
    mus_e6m9 = 54,mus_sigint = 55,mus_sg2int = 56,
    -- Doom screens (intro, intermission, etc.)
    mus_inter = 57,mus_intro = 58,mus_bunny = 59,mus_victor = 60,
    mus_introa = 61,
    -- Doom II
    mus_runnin = 62,mus_stalks = 63,mus_countd = 64,mus_betwee = 65,
    mus_doom = 66,mus_the_da = 67,mus_shawn = 68,mus_ddtblu = 69,
    mus_in_cit = 70,mus_dead = 71,mus_stlks2 = 72,mus_theda2 = 73,
    mus_doom2 = 74,mus_ddtbl2 = 75,mus_runni2 = 76,mus_dead2 = 77,
    mus_stlks3 = 78,mus_romero = 79,mus_shawn2 = 80,mus_messag = 81,
    mus_count2 = 82,mus_ddtbl3 = 83,mus_ampie = 84,mus_theda3 = 85,
    mus_adrian = 86,mus_messg2 = 87,mus_romer2 = 88,mus_tense = 89,
    mus_shawn3 = 90,mus_openin = 91,mus_evil = 92,mus_ultima = 93,
    mus_read_m = 94,mus_dm2ttl = 95,mus_dm2int = 96,
    -- NRFTL
    mus_nrftl1 = 97,
    mus_nrftl2 = 98,
    mus_nrftl3 = 99,
    mus_nrftl4 = 100,
    mus_nrftl5 = 101,
    mus_nrftl6 = 102,
    mus_nrftl7 = 103,
    mus_nrftl8 = 104,
    mus_nrftl9 = 105,
    NUMMUSIC = 106,
    mus_musinfo = 107
}

-- Map object type enum
DoomEnums.MobjType = {
    MT_NULL = -1,
    MT_PLAYER = 0,
    MT_POSSESSED = 1,
    MT_SHOTGUY = 2,
    MT_VILE = 3,
    MT_FIRE = 4,
    MT_UNDEAD = 5,
    MT_TRACER = 6,
    MT_SMOKE = 7,
    MT_FATSO = 8,
    MT_FATSHOT = 9,
    MT_CHAINGUY = 10,
    MT_TROOP = 11,
    MT_SERGEANT = 12,
    MT_SHADOWS = 13,
    MT_HEAD = 14,
    MT_BRUISER = 15,
    MT_BRUISERSHOT = 16,
    MT_KNIGHT = 17,
    MT_SKULL = 18,
    MT_SPIDER = 19,
    MT_BABY = 20,
    MT_CYBORG = 21,
    MT_PAIN = 22,
    MT_WOLFSS = 23,
    MT_KEEN = 24,
    MT_BOSSBRAIN = 25,
    MT_BOSSSPIT = 26,
    MT_BOSSTARGET = 27,
    MT_SPAWNSHOT = 28,
    MT_SPAWNFIRE = 29,
    MT_BARREL = 30,
    MT_TROOPSHOT = 31,
    MT_HEADSHOT = 32,
    MT_ROCKET = 33,
    MT_PLASMA = 34,
    MT_BFG = 35,
    MT_ARACHPLAZ = 36,
    MT_PUFF = 37,
    MT_BLOOD = 38,
    MT_TFOG = 39,
    MT_IFOG = 40,
    MT_TELEPORTMAN = 41,
    MT_EXTRABFG = 42,
    MT_MISC0 = 43,
    MT_MISC1 = 44,
    MT_MISC2 = 45,
    MT_MISC3 = 46,
    MT_MISC4 = 47,
    MT_MISC5 = 48,
    MT_MISC6 = 49,
    MT_MISC7 = 50,
    MT_MISC8 = 51,
    MT_MISC9 = 52,
    MT_MISC10 = 53,
    MT_MISC11 = 54,
    MT_MISC12 = 55,
    MT_INV = 56,
    MT_MISC13 = 57,
    MT_INS = 58,
    MT_MISC14 = 59,
    MT_MISC15 = 60,
    MT_MISC16 = 61,
    MT_MEGA = 62,
    MT_CLIP = 63,
    MT_MISC17 = 64,
    MT_MISC18 = 65,
    MT_MISC19 = 66,
    MT_MISC20 = 67,
    MT_MISC21 = 68,
    MT_MISC22 = 69,
    MT_MISC23 = 70,
    MT_MISC24 = 71,
    MT_MISC25 = 72,
    MT_CHAINGUN = 73,
    MT_MISC26 = 74,
    MT_MISC27 = 75,
    MT_MISC28 = 76,
    MT_SHOTGUN = 77,
    MT_SUPERSHOTGUN = 78,
    MT_MISC29 = 79,
    MT_MISC30 = 80,
    MT_MISC31 = 81,
    MT_MISC32 = 82,
    MT_MISC33 = 83,
    MT_MISC34 = 84,
    MT_MISC35 = 85,
    MT_MISC36 = 86,
    MT_MISC37 = 87,
    MT_MISC38 = 88,
    MT_MISC39 = 89,
    MT_MISC40 = 90,
    MT_MISC41 = 91,
    MT_MISC42 = 92,
    MT_MISC43 = 93,
    MT_MISC44 = 94,
    MT_MISC45 = 95,
    MT_MISC46 = 96,
    MT_MISC47 = 97,
    MT_MISC48 = 98,
    MT_MISC49 = 99,
    MT_MISC50 = 100,
    MT_MISC51 = 101,
    MT_MISC52 = 102,
    MT_MISC53 = 103,
    MT_MISC54 = 104,
    MT_MISC55 = 105,
    MT_MISC56 = 106,
    MT_MISC57 = 107,
    MT_MISC58 = 108,
    MT_MISC59 = 109,
    MT_MISC60 = 110,
    MT_MISC61 = 111,
    MT_MISC62 = 112,
    MT_MISC63 = 113,
    MT_MISC64 = 114,
    MT_MISC65 = 115,
    MT_MISC66 = 116,
    MT_MISC67 = 117,
    MT_MISC68 = 118,
    MT_MISC69 = 119,
    MT_MISC70 = 120,
    MT_MISC71 = 121,
    MT_MISC72 = 122,
    MT_MISC73 = 123,
    MT_MISC74 = 124,
    MT_MISC75 = 125,
    MT_MISC76 = 126,
    MT_MISC77 = 127,
    MT_MISC78 = 128,
    MT_MISC79 = 129,
    MT_MISC80 = 130,
    MT_MISC81 = 131,
    MT_MISC82 = 132,
    MT_MISC83 = 133,
    MT_MISC84 = 134,
    MT_MISC85 = 135,
    MT_MISC86 = 136,
}

-- Map object struct
DoomStructs.Mobj = {x = 0, y = 0, z = 0, angle = 0, mobjtype = 0, memAddr = 0x00}
--DoomStructs.Mobj.__index = DoomStructs.Mobj -- Sets up method lookup

function DoomStructs.Mobj:Create(out)
    out.parent = self
    return out
end

function DoomStructs.Mobj:PrintInfo()
    --print("Hello World!")
    -- print("\nNPC Info:\nX = " ..
    --     self.x ..
    --     "\nY = " ..
    --     self.y ..
    --     "\nZ = " .. self.z .. "\nAngle = " ..
    --     self.angle .. "\nMobjType = " .. self.mobjtype .. "\nMemory Addr = " .. self.memAddr)
end


function OnGlobalMapLoad()
    --print("Monsters randomized: ".. Doom.RandomizeMonsters(false))
end

function OnGlobalMapExit()
   --Doom.ClearRandoPool();
end
