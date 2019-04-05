require "json"

# Area files
files = Dir['./*.json']

def SetSpawnChance(item, chance)
    item["Spawn_chance"] = chance
end

items = Array.new

files.each do |fileName|
    data = JSON.parse File.read(fileName)

    data["Objects"].each do |i|
        if i["Item_Type"] == 40
            items.push i["Vnum"]
        end
    end

    File.open(fileName, "w") do |f|
      f.write(JSON.pretty_generate(data))
    end
end