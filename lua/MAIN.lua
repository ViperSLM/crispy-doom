-- Main Lua script. Runs as the game boots up

-- Main entry point. Runs before the game-specific main
function Main()
    --local rngSeed = os.time() - os.clock() * 1000
    --print("Random seed: "..rngSeed)
    print("Hello World")
    --RandomSeed(rngSeed)
end

-- Entry point for Doom/Doom 2
function DoomMain()
    LoadScript("DOOM")
end

-- Entry point for Heretic
function HereticMain()
end

-- Entry point for Strife
function StrifeMain()
end