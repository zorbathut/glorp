
local params, FrameTypes = ...
local platform, mode = params.platform, params.mode

local glv = tonumber(gl.GetString("VERSION"):match("^([^ ]*).*$"))

print(gl.GetString("VERSION"))
print(gl.GetString("VERSION"):match("^([^ ]*).*$"))
print(glv)

local disable_fonts_for_ogl_1 = false
if glv and glv < 2 then
  disable_fonts_for_ogl_1 = true
end

FrameTypes.Button = {}
function FrameTypes.Button:Key(button, ascii, event)
  if not button then return true end
  if button ~= "mouse_left" and not button:match("finger_%d+") then return true end  -- don't know don't care
  
  if event == "press" or event == "press_double" then
    self.button_down = true
  elseif event == "release" and self.button_down then
    self.button_down = false
    if self.Click then self:Click() end
  end
  
  if event == "press_repeat" then
    if self.Repeat then self:Repeat() end
  end
end
function FrameTypes.Button:MouseOut()
  self.button_down = false
end

local function LoadFont(name)
  local tex = Texture(name)
  local fil = loadfile(name .. ".lua")
  
  local env = {}
  setfenv(fil, env)
  
  fil()
  
  return {tex = tex, dat = env}
end
local TextDistanceFont = LoadFont("font")
--assert(false)

local text_shader
if not disable_fonts_for_ogl_1 then
  local vertex = glutil.Shader("VERTEX", [[
    void main()
    {
      gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
      gl_TexCoord[0]  = gl_MultiTexCoord0;
      
      gl_FrontColor = gl_Color;
      gl_BackColor = gl_Color;
    }
  ]])
  local fragment = glutil.Shader("FRAGMENT", [[
    uniform sampler2D tex;
    
    uniform vec2 texshift;
    
    void main()
    {
      float alph = texture2D(tex, gl_TexCoord[0].st).r;
      
      float shift = max(length(dFdx(gl_TexCoord[0].st) * texshift), length(dFdy(gl_TexCoord[0].st) * texshift)) / 2.0;
      
      float dens = smoothstep(0.5 - shift, 0.5 + shift, alph);
      
      gl_FragColor = vec4(gl_Color.rgb, gl_Color.a * dens);
    }
  ]])
  text_shader = glutil.Program()
  glutil.AttachShader(text_shader, vertex)
  glutil.AttachShader(text_shader, fragment)
  glutil.LinkProgram(text_shader)
end

local text_shader_outline -- hacky fix later
if not disable_fonts_for_ogl_1 then
  local vertex = glutil.Shader("VERTEX", [[
    void main()
    {
      gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
      gl_TexCoord[0]  = gl_MultiTexCoord0;
      
      gl_FrontColor = gl_Color;
      gl_BackColor = gl_Color;
    }
  ]])
  local fragment = glutil.Shader("FRAGMENT", [[
    uniform sampler2D tex;
    
    uniform vec2 texshift;
    
    void main()
    {
      float alph = texture2D(tex, gl_TexCoord[0].st).r;
      
      float shift = max(length(dFdx(gl_TexCoord[0].st) * texshift), length(dFdy(gl_TexCoord[0].st) * texshift)) / 2.0;
      
      
      if(alph > 0.4) {
        float dens = smoothstep(0.5 - shift, 0.5 + shift, alph);
        
        gl_FragColor = vec4(gl_Color.rgb * dens, gl_Color.a);
      } else {
        float shadow = smoothstep(0.2 - shift, 0.2 + shift, alph);
        
        gl_FragColor = vec4(0, 0, 0, gl_Color.a * shadow);
      }
    }
  ]])
  text_shader_outline = glutil.Program()
  glutil.AttachShader(text_shader_outline, vertex)
  glutil.AttachShader(text_shader_outline, fragment)
  glutil.LinkProgram(text_shader_outline)
end

FrameTypes.Text = {}
function FrameTypes.Text:SetText(text)
  self.text = text
  self:RecalculateBounds()
end
function FrameTypes.Text:SetSize(size)
  self.size = size
  self:RecalculateBounds()  -- technically just a multiplication, but lazy
end
function FrameTypes.Text:SetBorder(border)
  self.border = border
end
function FrameTypes.Text:RecalculateBounds()
  local font = TextDistanceFont
  
  self:SetHeight(self.size)
  self._scale = self.size / font.dat.height
  
  local wid = 0
  for i = 1, #self.text do
    local letter = self.text:byte(i)
    local kar = font.dat.characters[letter]
    
    if kar then
      wid = wid + kar.w
    end
  end
  
  self:SetWidth(wid * self._scale)
end
function FrameTypes.Text:SetColor(r, g, b, a)
  self.r, self.g, self.b, self.a = r, g, b, a
end
function FrameTypes.Text:Draw()
  local font = TextDistanceFont
  
  local iw, ih = font.tex:GetInternalWidth(), font.tex:GetInternalHeight()
  
  local vertices = {}
  local texes = {}
  
  
  gl.Color(self.r or 1, self.g or 0.5, self.b or 0, self.a or 1)
  
  local sx, sy = self:GetLeft(), self:GetTop() + font.dat.ascend * self._scale
  for i = 1, #self.text do
    local letter = self.text:byte(i)
    local kar = font.dat.characters[letter]
    
    if kar then
      local dx, dy = (kar.ex - kar.sx) * self._scale, (kar.ey - kar.sy) * self._scale
      
      local vsx, vsy, vex, vey = sx + kar.ox * self._scale, sy + kar.oy * self._scale, sx + kar.ox * self._scale + dx, sy + kar.oy * self._scale + dy
      local tsx, tsy, tex, tey = kar.sx / iw, kar.sy / ih, kar.ex / iw, kar.ey / ih
      
      table.insert(vertices, vsx)
      table.insert(vertices, vsy)
      table.insert(texes, tsx)
      table.insert(texes, tsy)
      
      table.insert(vertices, vex)
      table.insert(vertices, vsy)
      table.insert(texes, tex)
      table.insert(texes, tsy)
      
      table.insert(vertices, vex)
      table.insert(vertices, vey)
      table.insert(texes, tex)
      table.insert(texes, tey)
      
      table.insert(vertices, vsx)
      table.insert(vertices, vey)
      table.insert(texes, tsx)
      table.insert(texes, tey)
      
      sx = sx + kar.w * self._scale
    end
  end
  
  glutil.UseProgram(self.border and text_shader_outline or text_shader)
  font.tex:SetTexture()
  gl.TexParameter("TEXTURE_2D", "TEXTURE_MAG_FILTER", "LINEAR");
  glutil.UniformI(text_shader, "tex", 0)
  glutil.UniformF(text_shader, "texshift", iw * font.dat.distslope, ih * font.dat.distslope)
  glutil.RenderArray("QUADS", 2, vertices, nil, nil, 2, texes)
  SetNoTexture()
  glutil.UseProgram(nil)
end
function FrameTypes.Text:_Init()
  self.size = 50
  self:SetText("")
end

FrameTypes.Text_Multiline = {}
function FrameTypes.Text_Multiline:SetText(text)
  self.text = text
  self:RecreateSubtext()
end
function FrameTypes.Text_Multiline:SetSize(size)
  self.size = size
  self:RecreateSubtext()
end
function FrameTypes.Text_Multiline:SetColor(r, g, b, a)
  self.r, self.g, self.b, self.a = r, g, b, a
  self:RecreateSubtext()
end
function FrameTypes.Text_Multiline:RecreateSubtext()
  self.__textLayout:Invalidate()
end
function FrameTypes.Text_Multiline:RecreateSubtextCore()
  local font = TextDistanceFont
  
  -- could probably be more efficient
  local width = self:GetWidth()
  
  for _, v in ipairs(self._subtext) do
    v:Detach()
  end
  self._subtext = {}
  
  local scale = self.size / font.dat.height
  
  local line = 0
  local function testLine(text)
    local wid = 0
    for i = 1, #text do
      local letter = text:byte(i)
      local kar = font.dat.characters[letter]
      
      if kar then
        wid = wid + kar.w * scale
      end
    end
    
    return wid <= width
  end
  local function insertLine(text)
    local fram = CreateFrame("Text", self)
    fram:SetSize(self.size)
    fram:SetText(text)
    fram:SetColor(self.r, self.g, self.b, self.a)
    fram:SetPoint("TOPLEFT", self, "TOPLEFT", 0, line * (font.dat.height + font.dat.padding) * scale)
    table.insert(self._subtext, fram)
    line = line + 1
  end
  local function attemptLine(text)
    if testLine(text) then
      insertLine(text)
      return true
    else
      return false
    end
  end
  local textleft = self.text
  local function processChunk()
    -- exists just because we don't have continue
    
    -- First, see if we can fit a line in
    local aline = textleft:match("^([^\n]*)\n")
    if aline then
      if attemptLine(aline) then
        textleft = textleft:sub(#aline + 2)
        return
      end
    end
    
    -- See if we can just fit everything
    if attemptLine(textleft) then
      textleft = ""
      return
    end
    
    -- Okay, we can't. Take that line and start chopping words off the end.
    if not aline then aline = textleft end -- We might not have had a line, in which case we should just use everything.
    while true do
      aline = aline:match("^(.+)[ \t][^ \t]*$")
      if not aline then break end -- Couldn't strip a word.
      if aline and attemptLine(aline) then
        textleft = textleft:sub(#aline + 2)
        return
      end
    end
    
    -- We can't fit an entire word either. Just add as many letters as we can, with at least one.
    for ct = 2, #textleft do
      aline = textleft:sub(1, ct)
      if not testLine(aline) then
        insertLine(textleft:sub(1, ct - 1))
        textleft = textleft:sub(ct)
        return
      end
    end
    
    -- Maybe we only have one letter left, and it doesn't fit.
    assert(#textleft == 1)
    insertLine(textleft)
    textleft = ""
  end
  while #textleft > 0 do
    processChunk()
  end
  
  if line > 0 then
    return (line * (font.dat.height + font.dat.padding) - font.dat.padding) * scale
  else
    return 0
  end
end
function FrameTypes.Text_Multiline:PreDraw()
  self.__textLayout:Get() -- does all the processing to rebuild the text layout
end
function FrameTypes.Text_Multiline:_Init()
  self._subtext = {}
  self.size = 50
  
  local widthHandle = self:GetHandle("x", "size")
  
  self.__textLayout = CreateNode(self.__name .. " text layout")
  self.__textLayout:Set({widthHandle}, function () return self:RecreateSubtextCore() end)
  
  self:SetHandle("y", "size", {self.__textLayout}, function () return self.__textLayout:Get() end)
  
  self:SetText("")
end


FrameTypes.Texture = {}
function FrameTypes.Texture:SetTexture(tex, preserve_dimensions)
  assert(tex)
  if type(tex) == "string" then tex = Texture(tex) end
  self.tex = tex
  if not preserve_dimensions then
    self:SetWidth(tex:GetWidth())
    self:SetHeight(tex:GetHeight())
  end
end
function FrameTypes.Texture:Draw()
  if self.tex then glutil.RenderBoundedSprite(self.tex, {self:GetBounds()}, self._sprite_r, self._sprite_g, self._sprite_b, self._sprite_a) end
end
function FrameTypes.Texture:SetColor(r, g, b, a)
  self._sprite_r, self._sprite_g, self._sprite_b, self._sprite_a = r, g, b, a
end
function FrameTypes.Texture:GetColor()
  return self._sprite_r, self._sprite_g, self._sprite_b, self._sprite_a
end

FrameTypes.Drag = {}
function FrameTypes.Drag:Tick()
  if self._dragging then
    if not IsKeyDown("mouse_left") then
      self._dragging = false
      if self.DragEnded then self:DragEnded() end
    else
      local nx, ny = GetMouse()
      if self.Drag then self:Drag(nx - self.drag_x, ny - self.drag_y) end
      self.drag_x, self.drag_y = nx, ny
    end
  end
end
function FrameTypes.Drag:Key(key, _, event)
  if key == "mouse_left" and event == "press" then
    self._dragging = true
    self.drag_x, self.drag_y = GetMouse()
    if self.DragStarted then self:DragStarted() end
  end
end

FrameTypes.Scroll = {}
function FrameTypes.Scroll:Tick()
  if self.scroll_position ~= self.scroll_position_last then
    local scrollheight = self.scrollbar:GetHeight() - 2
    local dataheight = self.internal:GetHeight()
    local widgetheight
    local widgetofs
    
    self.scroll_multiplier = dataheight / scrollheight
    
    if dataheight < scrollheight then
      self.scroll_position = 0
      widgetheight = scrollheight
      widgetofs = 0
    else
      self.scroll_position = clamp(self.scroll_position, 0, dataheight - scrollheight)
      widgetheight = scrollheight * scrollheight / dataheight
      widgetofs = scrollheight * self.scroll_position / dataheight
    end
    
    self.scroll_widget:SetHeight(widgetheight)
    self.scroll_widget:SetPoint("TOP", self.scrollbar, "TOP", nil, 1 + widgetofs)
    
    self.internal:SetPoint("TOPLEFT", self, "TOPLEFT", nil, -self.scroll_position)
    
    self.scroll_position_last = self.scroll_position
  end
end
function FrameTypes.Scroll:_Init(name)
  self.key_bounds_cull = true
  
  self.scrollbar = CreateFrame("Frame", self, name .. " (scrollbar)")
  self.scrollbar:SetPoint("TOPRIGHT", self, "TOPRIGHT")
  self.scrollbar:SetPoint("BOTTOM", self, "BOTTOM")
  self.scrollbar:SetWidth(10)
  self.scrollbar:SetBackgroundColor(0.2, 0.2, 0.2)
  
  self.scroll_widget_dragger = CreateFrame("Draggable", self.scrollbar)
  
  self.scroll_widget = CreateFrame("Frame", self.scroll_widget_dragger, name .. " (scroll widget)")
  self.scroll_widget:SetPoint("LEFT", self.scrollbar, "LEFT", 1, nil)
  self.scroll_widget:SetPoint("RIGHT", self.scrollbar, "RIGHT", -1, nil)
  self.scroll_widget:SetBackgroundColor(0.3, 0.3, 0.3)
  
  self.scroll_position = 0
  self.scroll_multiplier = 1
  
  function self.scroll_widget_dragger.Drag(_, dx, dy)
    self.scroll_position = self.scroll_position + dy * self.scroll_multiplier
  end
  
  function self:Key(key, _, event)
    if key:find("mouse_wheel") and event:find("press") then
      self.scroll_position = self.scroll_position + 20 * ((key == "mouse_wheel_up") and -1 or 1)
    end
  end
  
  self.internal = CreateFrame("Frame", self, name .. " (scroll internal frame)")
  
  self.internal:SetPoint("TOPLEFT", self, "TOPLEFT")
  self.internal:SetPoint("RIGHT", self.scrollbar, "LEFT")
  self.internal:SetPoint("BOTTOM", self, "TOPLEFT", nil, 100) -- just for a sane default
  self.internal:SetLayer(-1000) -- this is not ideal
  
  function self:DrawPre()
    gl.Enable("SCISSOR_TEST")
    local sx, sy, ex, ey = self:GetBounds()
    gl.Scissor(sx, UIParent:GetHeight() - ey, ex - sx, ey - sy)
  end
  function self:DrawPost()
    gl.Disable("SCISSOR_TEST")
  end
  
  -- just to init it
  self.scroll_widget:SetHeight(0)
  self.scroll_widget:SetPoint("TOP", self.scrollbar, "TOP", nil, 1)
end

FrameTypes.RenderSorter = {}
do
  local function GetMatrix(typ)
    return gl.Get(typ .. "_MATRIX")
  end
  local function SetMatrix(typ, dat)
    gl.MatrixMode(typ)
    gl.LoadMatrix(dat)
  end

  local collates = {}
  local RenderSort_Current
  local function RenderSort_Deactivated(shader, render)
    assert(false, "Not in a shader rendering context!")
  end
  local function RenderSort_Activated(shader, render)
    if not collates[shader] then
      collates[shader] = {}
    end
    table.insert(collates[shader], {render = render, modelview = GetMatrix("MODELVIEW"), projection = GetMatrix("PROJECTION")})
  end
  function RenderSort(...)  -- required 'cause of some of the copying we're doing to create shell environments
    return RenderSort_Current(...)
  end
  RenderSort_Current = RenderSort_Deactivated


  function handle(f)
    SetMatrix("MODELVIEW", f.modelview)
    SetMatrix("PROJECTION", f.projection)
    f.render()
  end
  
  function FrameTypes.RenderSorter:Render_New(...)
    local model = GetMatrix("MODELVIEW")
    local proj = GetMatrix("PROJECTION")
    
    assert(RenderSort_Current == RenderSort_Deactivated)
    RenderSort_Current = RenderSort_Activated
    perfbar(0.5, 0.5, 1.0, self.Render_Original, self, ...)
    assert(RenderSort_Current == RenderSort_Activated)
    RenderSort_Current = RenderSort_Deactivated
    
    perfbar(0.5, 1.0, 0.5, function ()
      for _, v in pairs(self.priorities) do
        if collates[v.shader] then
          perfbar(v.r, v.g, v.b, function ()
            glutil.UseProgram(v.shader)
            for _, f in pairs(collates[v.shader]) do
              handle(f)
            end
          end)
        end
        collates[v.shader] = nil
      end
      
      for s, v in pairs(collates) do
        print("Rendering unknown shader", s)
        perfbar(1.0, 0, 0, function ()
          glutil.UseProgram(s)
          for _, f in pairs(v) do
            handle(f)
          end
        end)
      end
      
      glutil.UseProgram()
      
      SetMatrix("MODELVIEW", model)
      SetMatrix("PROJECTION", proj)
      
      collates = {}
    end)
  end
  function FrameTypes.RenderSorter:SetShaderPriority(shader, priority, r, g, b)
    for k, v in ipairs(self.priorities) do
      if v.shader == shader then
        table.remove(self.priorities, k)
      end
    end
    
    r = r or math.random() * 0.5 + 0.5
    g = g or math.random() * 0.5 + 0.5
    b = b or math.random() * 0.5 + 0.5
    
    table.insert(self.priorities, {shader = shader, priority = priority, r = r, g = g, b = b})
    table.sort(self.priorities, function (a, b) return a.priority < b.priority end)
  end
  function FrameTypes.RenderSorter:_Init()
    self.priorities = {}
    self.Render_Original = self.Render
    self.Render = self.Render_New
    self.Render_New = nil
  end
end


if disable_fonts_for_ogl_1 then  -- disable fonts!
  FrameTypes.Text = {}
  function FrameTypes.Text:SetText(text) self:SetWidth(50) self:SetHeight(50) end
  function FrameTypes.Text:SetSize(size) end
  function FrameTypes.Text:SetColor(r, g, b, a) end
  function FrameTypes.Text:Tick()
    print("Fonts disabled!")
  end
  
  FrameTypes.Text_Multiline = FrameTypes.Text
end
