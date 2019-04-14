require "json"

# Area files
files = Dir['./*.json']

def SetSpawnChance(item, chance)
    item["Spawn_chance"] = chance
end

files.each do |fileName|
    data = JSON.parse File.read(fileName)

    data["Objects"].each do |i|

        i["Extra_Flags"] = i["Extra_Flags"] - [512]
        i["Extra_Flags"] = i["Extra_Flags"] - [1024]
        i["Extra_Flags"] = i["Extra_Flags"] - [2048]

    end

    File.open(fileName, "w") do |f|
      f.write(JSON.pretty_generate(data))
    end
end