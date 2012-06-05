-- this is the main function which is called from
-- the C++ source code. We use dofile and call the
-- plugin loader so users can change that file and
-- load newly created plugins without having to
-- restart kiwibot
function main(currentLine, updatedFiles)
  dofile("lua/plugin-loader.lua")
  pluginLoader(currentLine)
end