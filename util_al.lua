
-- tables in tables exist to fix a luabind crash

local sources_active = {}
local sources = setmetatable({}, {__mode = "kv"})

alutil = {}
function alutil.Source()
  local src = AlSourceID()
  sources[src] = src
  sources_active[src] = src
  
  return src
end

function alutil.Tick()
  for k in pairs(sources) do
    if al.GetSource(k:get(), "SOURCE_STATE") ~= "PLAYING" then
      sources_active[k] = nil
    else
      sources_active[k] = true
    end
  end
end

local buffers = {}

LoadSoundBuffer = LoadSound
function LoadSound(sound)
  if not buffers[sound] then
    buffers[sound] = LoadSoundBuffer(sound)
  end
  
  return buffers[sound]
end

local sound_proto = {}
function sound_proto:Pitch(pitch)
  al.Source(self.id:get(), "PITCH", pitch)
  return self
end
function sound_proto:Gain(gain)
  al.Source(self.id:get(), "GAIN", gain)
  return self
end
function sound_proto:Play()
  al.SourcePlay(self.id:get())
  return self
end
local sound_proto_meta = {__index = sound_proto}

function CreateSound(sound)
  local source = alutil.Source()
  local buffer = LoadSound(sound)
  al.Source(source:get(), "BUFFER", buffer:get())
  local item = setmetatable({id = source, buffer = buffer}, sound_proto_meta)
  return item:Pitch(1):Gain(1)
end

function PlaySound(sound)
  return CreateSound(sound):Play()
end
