-- this is the main function which is called from
-- the C++ source code. We use dofile and call the
-- plugin loader so users can change that file and
-- load newly created plugins without having to
-- restart kiwibot
--


--These tables are global (including to all plugins)
plugins = {}
loadedPlugins = {}


function main(username, serverPart, userMessage, updatedFiles, deletedFiles)
  loadUpdatedFiles(updatedFiles)
  parseWithPlugins(username, serverPart, userMessage)
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
    table.insert(plugins, plugin)
    table.insert(loadedPlugins,plugin)
  end
end

function parseWithPlugins(username, serverPart, userMessage)
  for _,plugin in ipairs(loadedPlugins) do
    local parser = plugin.parseFunction
    parser(username, serverPart, userMessage)
  end
end
