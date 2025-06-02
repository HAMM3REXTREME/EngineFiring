-- Create and load SoundBank
local mainSamples = SoundBank.new()
local files = {
    "assets/audio/tick_library/note_64.wav",
    "assets/audio/tick_library/note_65.wav",
    "assets/audio/tick_library/note_66.wav"
}

for i, file in ipairs(files) do
    mainSamples:addFromWav(file) -- single file each time
end
-- Create Engine instance
local engineDef = Engine.new("Example",
    {0,1},
    5.4)

-- Create EngineSoundGenerator instance
local engineSoundGen = EngineSoundGenerator.new(mainSamples, engineDef, 1000.0, 0.5, 0, 0)

-- Return these objects so C++ can get them
return {
    mainSamples = mainSamples,
    engineDef = engineDef,
    engineSoundGen = engineSoundGen
}

