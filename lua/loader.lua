-- this is the main function which is called from
-- the C++ source code. We use dofile and call the
-- plugin loader so users can change that file and
-- load newly created plugins without having to
-- restart kiwibot
--

plugins = {}
loadedPlugins = {}

function main(currentLine, updatedFiles)
  loadUpdatedFiles(updatedFiles)
  parseWithPlugins(currentLine)
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
    table.insert(plugins,plugin)
  end
end

function parseWithPlugins(currentLine)
  for _,plugin in ipairs(plugins) do 
    local parser = plugin.parseFunction
    parser(currentLine)
  end
end
