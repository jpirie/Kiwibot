-- global variables for plugin control
require"lfs"

function pluginLoader (currentLine)
    for file in lfs.dir("./lua/plugins/") do
      if file ~= "." and file ~= ".." then
        local parser = dofile("./lua/plugins/".. file)
        parser(currentLine)
      end
    end
end


