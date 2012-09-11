function managePlugins(request, botName)

  local function listPlugins(request)
    local plugin_list = ""
    local first_plugin = false
    for i,plugin in ipairs(plugins) do
      local name = plugin.name
      if name ~= "Plugin Manager" then
        if first_plugin == false  then
          plugin_list = plugin_list .. plugin.name
	  first_plugin = true
        else
          plugin_list = plugin_list .. ", " .. plugin.name
        end
      end
    end
    sendLuaMessage("Plugins loaded: " .. plugin_list)
  end

  local function unloadPlugins(request)

    local _,x = string.find(request, "plugin remove")
    --Substring takes into account inclusive matching & space between words
    local pluginName = string.sub(request,x+2)
    --Trim whitespace
    pluginName = pluginName:gsub("^%s*(.-)%s*$", "%1")

    local pluginFound = false
    for i,plugin in ipairs(plugins) do
      if(plugin.name == pluginName) then
        pluginFound = true
      end      
    end

    if pluginFound then
      sendLuaMessage("Unloading plugin " .. pluginName)
    else
      sendLuaMessage("No such plugin " .. pluginName)
    end
  end

  if (string.find(request, "plugin list")) then
    listPlugins(request)
  elseif (string.find(request, "plugin remove")) then
    unloadPlugins(request)
  end
end

function saveData()
  print ("Saving plugin-manager.lua data...")
end

return {name="Plugin Manager", description="For managing plugins within IRC", 
        parseFunction=managePlugins, saveDataFunction=saveData}
