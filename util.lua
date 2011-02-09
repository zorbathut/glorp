
local mode, platform = ...

runfile("util_al.lua", _G)
runfile("util_gl.lua", _G)

math.randomseed(os.time())

function rotate_mod(x, m)
  while x <= 0 do x = x + m end
  while x > m do x = x - m end
  return x
end
function rotate(x, y, rot)
  local s, c = math.sin(rot), math.cos(rot)
  return x * c - y * s, y * c + x * s
end

function export_items_rw(tab, items)
  local lookup = {}
  for _, v in pairs(items) do
    lookup[v] = true
  end
  
  return setmetatable({}, {__index = function(_, k) return lookup[k] and tab[k] or nil end, __newindex = function(t, k, a) if lookup[k] then tab[k] = a else t[k] = a end end})
end
function export_items_ro(tab, items)
  local lookup = {}
  for _, v in pairs(items) do
    if not tab[v] then
      print("Can't find", v)
      assert(tab[v], v)
    end
    lookup[v] = tab[v]
  end
  
  return setmetatable({}, {__index = lookup})
end

function perfbar(r, g, b, func, ...)
  local pb = Perfbar_Init(r, g, b)
  func(...)
  pb:Destroy()
end

function io.dump(filename, contents)
  t = io.open(filename, "wb")
  t:write(contents)
  t:close()
end
function io.snatch(filename)
  t = io.open(filename, "rb")
  if not t then t = io.open("data/" .. filename, "rb") end
  if not t then return end
  local dat = t:read("*all")
  t:close()
  return dat
end
function table.wipe(tab)
  while true do
    local x = next(tab)
    if not x then break end
    tab[x] = nil
  end
end
function table.append(a, b)
  for _, v in ipairs(b) do
    table.insert(a, v)
  end
end
function math.random_choose(tab)
  return tab[math.random(#tab)]
end


function GetMouse() return GetMouseX(), GetMouseY() end

function approach(current, target, delta, dist)
  if target > current then
    return -approach(-current, -target, delta, dist)
  end
  
  if current - target <= dist then return current end
  if current - target <= dist + delta then return target + dist end
  
  return current - delta
end
function clamp(cur, min, max)
  if cur < min then return min end
  if cur > max then return max end
  return cur
end
function lerp(d, s, e)
  return s * (1 - d) + e * d
end
function delerp(d, s, e)
  return (d - s) / (e - s)
end
function remap(d, as, ae, bs, be)
  return lerp(delerp(d, as, ae), bs, be)
end
function bezier(x0, x1, x2, x3, t)
  local cx = 3 * (x1 - x0)
  local bx = 3 * (x2 - x1) - cx
  local ax = x3 - x0 - cx - bx
  return ax * t * t * t + bx * t * t + cx * t + x0
end
function cubic(x0, x1, x2, x3, t)
  local a0 = x3 - x2 - x0 + x1
  local a1 = x0 - x1 - a0
  local a2 = x2 - x0
  local a3 = x1
  
  return a0 * t * t * t + a1 * t * t + a2 * t + a3
end
function normalize(a, b, c, d, e)
  assert(not e)
  local acu = 0
  if a then acu = acu + a * a end
  if b then acu = acu + b * b end
  if c then acu = acu + c * c end
  if d then acu = acu + d * d end
  acu = math.sqrt(acu)
  if acu == 0 then acu = 1 end -- yes yes
  
  return a and a / acu, b and b / acu, c and c / acu, d and d / acu
end
function collide(one, two)
  local ax, ay = one:GetCenter()
  local bx, by = two:GetCenter()
  local asx, asy = one:GetSize()
  local bsx, bsy = two:GetSize()
  
  return math.abs(ax - bx) * 2 <= asx + bsx and math.abs(ay - by) * 2 <= asy + bsy
end
function sort2(a, b)
  if a > b then return b, a else return a, b end
end

function line_intersect(sa, ea, sb, eb)
  local x1, x2, x3, x4, y1, y2, y3, y4 = sa[1], ea[1], sb[1], eb[1], sa[2], ea[2], sb[2], eb[2]
  local denom = (y4 - y3) * (x2 - x1) - (x4 - x3) * (y2 - y1)
  if denom == 0 then
    return false
  end
  local ua = ((x4 - x3) * (y1 - y3) - (y4 - y3) * (x1 - x3)) / denom
  local ub = ((x2 - x1) * (y1 - y3) - (y2 - y1) * (x1 - x3)) / denom
  return ua >= 0 and ua <= 1 and ub >= 0 and ub <= 1
end

function PlaySound(snd, vol)
  PlaySound_Core(snd, vol or 1)
end
function ControlSound(snd, vol, loop)
  return ControlSound_Core(snd, vol or 1, loop or false)
end


function math.round(x)
  return math.floor(x + 0.5)
end

function impose_standard_ai(target)
  assert(not target.Tick)
  local tick_ai_reservoir = {}
  local clears = {}
  local clear_count = 0
  local function wipe_item(item)
    clears[item] = true
    clear_count = clear_count + 1
  end
  function target:TickStandard()
    local tar = {}
    for _, v in ipairs(tick_ai_reservoir) do
      table.insert(tar, v)
    end
    
    for _, v in ipairs(tar) do
      v(self)
    end
    
    if clear_count > 0 then
      local ntai = {}
      for _, v in ipairs(tick_ai_reservoir) do
        if not clears[v] then
          table.insert(ntai, v)
        else
          clear_count = clear_count - 1
        end
      end
      
      assert(clear_count == 0)
      
      clears = {}
      tick_ai_reservoir = ntai
    end
  end
  if not target.Tick then target.Tick = target.TickStandard end
  function target:Tick_Add(func)
    table.insert(tick_ai_reservoir, func)
  end
  function target:Tick_AddCoro(func)
    local coro
    coro = coroutine.wrap(function ()
      func(self)
      
      wipe_item(coro)
    end)
    table.insert(tick_ai_reservoir, coro)
  end
end


local function heap_left(x) return (2*x) end
local function heap_right(x) return (2*x + 1) end
local function heap_sane(heap)
  local dmp = ""
  local finishbefore = 2
  for i = 1, #heap do
    if i == finishbefore then
      print(dmp)
      dmp = ""
      finishbefore = finishbefore * 2
    end
    dmp = dmp .. string.format("%f ", heap[i].c)
  end
  print(dmp)
  print("")
  for i = 1, #heap do
    --[[ assert(not heap[heap_left(i)] or heap[i].c <= heap[heap_left(i)].c) ]]
    --[[ assert(not heap[heap_right(i)] or heap[i].c <= heap[heap_right(i)].c) ]]
  end
end
function heap_insert(heap, item)
  --[[ assert(item) ]]
  table.insert(heap, item)
  local pt = #heap
  while pt > 1 do
    local ptd2 = math.floor(pt / 2)
    if heap[ptd2].c <= heap[pt].c then
      break
    end
    local tmp = heap[pt]
    heap[pt] = heap[ptd2]
    heap[ptd2] = tmp
    pt = ptd2
  end
  --heap_sane(heap)
end
function heap_extract(heap)
  local rv = heap[1]
  if #heap == 1 then table.remove(heap) return rv end
  heap[1] = table.remove(heap)
  local idx = 1
  while idx < #heap do
    local minix = idx
    --local hl, hr = heap_left(idx), heap_right(idx)
    local hl, hr = 2*idx, 2*idx+1 -- these had better be equivalent to the line above one
    if heap[hl] and heap[hl].c < heap[minix].c then minix = hl end
    if heap[hr] and heap[hr].c < heap[minix].c then minix = hr end
    if minix ~= idx then
      local tx = heap[minix]
      heap[minix] = heap[idx]
      heap[idx] = tx
      idx = minix
    else
      break
    end
  end
  --heap_sane(heap)
  return rv
end




do
  local sms = ShowMouseCursor
  local shown = true
  function ShowMouseCursor(flag)
    sms(flag)
    shown = flag
  end
  function MouseCursorShown()
    return shown
  end
end
do
  local lms = LockMouseCursor
  local locked = false
  function LockMouseCursor(flag)
    lms(flag)
    locked = flag
  end
  function MouseCursorLocked()
    return locked
  end
end



-- FIX THIS LATER (again)
--if not mode then
--  local ambient = ControlSound("Kirsty Hawkshaw - In Between", nil, true)
--end
