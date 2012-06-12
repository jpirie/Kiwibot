
function managePlugins(request)
  request = request:lower()
  if (string.find(request, "plugin list")) then
    local plugin_list = ""
    for i,plugin in ipairs(plugins) do
      local name = plugin.name
      if name ~= "Plugin Manager" then
        if i == 1  then
          plugin_list = plugin_list .. plugin.name 
        else
          plugin_list = plugin_list .. ", " .. plugin.name 
        end
      end
    end
    sendLuaMessage("Plugins loaded: " .. plugin_list)
  end
end
return {name="Plugin Manager", description="For managing plugins within IRC", parseFunction=managePlugins}
