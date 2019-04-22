require "json"

# Area files
files = Dir['./*.json']

def SetSpawnChance(item, chance)
    item["Spawn_chance"] = chance
end

files.each do |fileName|
    data = JSON.parse File.read(fileName)

    data["Resets"].each do |i|
        if ["E", "G"].include? i["Command"]
            i["Arg2"] = 75
        end

    end

    File.open(fileName, "w") do |f|
      f.write(JSON.pretty_generate(data))
    end
end