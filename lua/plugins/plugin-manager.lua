----------------------------------------------------------------------
-- Copyright 2012 2013 William Gatens
--
-- Kiwibot is a free software: you can redistribute it and/or modify
-- it under the terms of the GNU General Public License as published by
-- the Free Software Foundation, either version 3 of the License, or
-- (at your option) any later version.
--
-- Kiwibot is distributed in the hope that it will be useful,
-- but WITHOUT ANY WARRANTY; without even the implied warranty of
-- MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
-- GNU General Public License for more details.
--
-- You should have received a copy of the GNU General Public License
-- along with Kiwibot.  If not, see <http://www.gnu.org/licenses/>.
--
-- Description: Plugin manager for lua. Allows dynamically loading and
--              unloading plugins at run-time.
----------------------------------------------------------------------

function managePlugins(username, serverPart, userMessage, isPrivateMessage)
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
    sendLuaMessageToSource(username, "Plugins loaded: " .. plugin_list, isPrivateMessage)
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
      sendLuaChannelMessage("Unloading plugin " .. pluginName)
    else
      sendLuaMessageToSource(username, "No such plugin " .. pluginName, isPrivateMessage)
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
        sendLuaMessageToSource(username, "Plugin " .. pluginName .. " already loaded", isPrivateMessage)
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
        sendLuaChannelMessage("Loading plugin " .. pluginName)
      else
        sendLuaChannelMessage("No such plugin " .. pluginName)
      end
    end
  end

  local function pluginHelp (userMessage)
    local _,x = string.find(userMessage, "plugin help")
    --Substring takes into account inclusive matching & space between words
    local pluginName = string.sub(userMessage,x+2)
    --Trim whitespace
    pluginName = pluginName:gsub("^%s*(.-)%s*$", "%1")

    local pluginFound = false
    local foundPlugin = nil
    for i,plugin in ipairs(plugins) do
      if(plugin.name == pluginName) then
        pluginFound = true
        foundPlugin = plugin
      end
    end

    if pluginFound then
      local documentation = foundPlugin.doc
      if documentation then
       sendLuaPrivateMessage(username, "Documentation for " .. pluginName) 
       for command,usage in pairs(documentation) do
          sendLuaPrivateMessage(username, tostring(command) .. "\t" .. tostring(usage))
       end
      else  
       sendLuaPrivateMessage(username, "No documentation found for " .. pluginName) 
      end
    else
      sendLuaChannelMessage("No such plugin " .. pluginName)
    end

  end

  if (string.find(userMessage, "plugin list")) then
    listPlugins(userMessage)
  elseif (string.find(userMessage, "plugin load")) then
    loadPlugins(userMessage)
  elseif (string.find(userMessage, "plugin unload")) then
    unloadPlugins(userMessage)
  elseif (string.find(userMessage, "plugin help")) then
    pluginHelp(userMessage)
  end
end

return {name="Plugin Manager", description="For managing plugins within IRC",
parseFunction=managePlugins, saveDataFunction=saveData, loadDataFunction=loadManagerData}
