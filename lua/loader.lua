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
-- Description: this is the main function which is called from the C++
--              source code. We use dofile and call the plugin loader
--              so users can change that file and load newly created
--              plugins without having to restart kiwibot
----------------------------------------------------------------------

--These tables are global (including to all plugins)
plugins = {}
loadedPlugins = {}

function main(username, serverPart, userMessage, isPrivateMessage, updatedFiles, deletedFiles)
  loadUpdatedFiles(updatedFiles)
  parseWithPlugins(username, serverPart, userMessage, isPrivateMessage)
end

function savePluginData()
  for _,plugin in ipairs(loadedPlugins) do
    local saveFunction = plugin.saveDataFunction
    if (saveFunction) then
      saveFunction()
    end
  end
end

function loadPluginData()
  for _,plugin in ipairs(loadedPlugins) do
    local loadFunction = plugin.loadDataFunction
    if (loadFunction) then
      loadFunction()
    end
  end
end

function loadUpdatedFiles(updatedFiles)
  for _,file in ipairs(updatedFiles) do
    local plugin = dofile(file)
    --Remove older version of updated plugins
     for i,currentPlugin in ipairs(plugins) do
       if plugin.name == currentPlugin.name then
         table.remove(plugins,i)
       end
     end
     for i,currentPlugin in ipairs(loadedPlugins) do
       if plugin.name == currentPlugin.name then
         table.remove(loadedPlugins,i)
       end
     end
    table.insert(plugins, plugin)
    table.insert(loadedPlugins,plugin)
  end
end

function parseWithPlugins(username, serverPart, userMessage, isPrivateMessage)
  for _,plugin in ipairs(loadedPlugins) do
    local parser = plugin.parseFunction
    parser(username, serverPart, userMessage, isPrivateMessage)
  end
end
