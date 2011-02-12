
local mode, platform = ...

if true then
  local function testerror(bef, nam)
    local err = al.GetError()
    if err ~= "NO_ERROR" then
      print("AL ERROR: ", err, bef, nam)
      if mode then assert(err == "NO_ERROR", err ..  "   " .. bef .. " " .. nam) end -- fuckyou
    end
  end
  for k, v in pairs(al) do
    local tk = k
    if tk ~= "GetError" then
      al[tk] = function (...)
        testerror("before", k)
        return (function (...)
          testerror("after", k)
          return ...
        end)(v(...))
      end
    end
  end
end

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
function sound_proto:Looping(gain)
  al.Source(self.id:get(), "LOOPING", gain and "TRUE" or "FALSE")
  return self
end
function sound_proto:Position(x, y, z)
  al.Source(self.id:get(), "POSITION", {x, y, z})
  return self
end
function sound_proto:Velocity(x, y, z)
  al.Source(self.id:get(), "VELOCITY", {x, y, z})
  return self
end
function sound_proto:MixerOverride(channel, volume)
  al.Source(self.id:get(), "EXT_MIXER_OVERRIDE_FLAG", "TRUE")
  al.Source(self.id:get(), "EXT_MIXER_OVERRIDE", {channel, volume})
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

al.Listener("POSITION", {0, 0, 0})
al.DistanceModel("NONE")
