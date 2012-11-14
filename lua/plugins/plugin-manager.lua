function managePlugins(username, serverPart, userMessage)
  local function listPlugins(userMessage)
    local plugin_list = ""
    local first_plugin = false
    for i,plugin in ipairs(loadedPlugins) do
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

  local function unloadPlugins(userMessage)
    local _,x = string.find(userMessage, "plugin unload")
    --Substring takes into account inclusive matching & space between words
    local pluginName = string.sub(userMessage,x+2)
    --Trim whitespace
    pluginName = pluginName:gsub("^%s*(.-)%s*$", "%1")

    local pluginFound = false
    for i,plugin in ipairs(loadedPlugins) do
      if(plugin.name == pluginName) then
        pluginFound = true
        table.remove(loadedPlugins, i)
      end
    end

    if pluginFound then
      sendLuaMessage("Unloading plugin " .. pluginName)
    else
      sendLuaMessage("No such plugin " .. pluginName)
    end
  end

  local function loadPlugins(userMessage)
    local _,x = string.find(userMessage, "plugin load")
    --Substring takes into account inclusive matching & space between words
    local pluginName = string.sub(userMessage,x+2)
    --Trim whitespace
    pluginName = pluginName:gsub("^%s*(.-)%s*$", "%1")

    local alreadyLoaded = false
    for i,loaded_plugin in ipairs(loadedPlugins) do
      if(loaded_plugin.name == pluginName) then
        sendLuaMessage("Plugin " .. pluginName .. " already loaded")
        alreadyLoaded = true
      end 
    end

    if not alreadyLoaded then
      local pluginFound = false
      for i,plugin in ipairs(plugins) do
        if(plugin.name == pluginName) then
          pluginFound = true
          table.insert(loadedPlugins, plugin)
        end
      end

      if pluginFound then
        sendLuaMessage("Loading plugin " .. pluginName)
      else
        sendLuaMessage("No such plugin " .. pluginName)
      end
    end

  end

  if (string.find(userMessage, "plugin list")) then
    listPlugins(userMessage)
  elseif (string.find(userMessage, "plugin load")) then
    loadPlugins(userMessage)
  elseif (string.find(userMessage, "plugin unload")) then
    unloadPlugins(userMessage)
  end
end

function saveData()
  print ("Saving plugin-manager.lua data...")
  print ("done!")
end

function loadData ()
  print ("Loading plugin-manager.lua data...")
  print ("done!")
end


return {name="Plugin Manager", description="For managing plugins within IRC",
parseFunction=managePlugins, saveDataFunction=saveData, loadDataFunction=loadData}
