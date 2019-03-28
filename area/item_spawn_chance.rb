require "json"

# Area files
files = Dir['./*.json']

def SetSpawnChance(item, chance)
    object["Spawn_chance"] = chance
end

files.each do |fileName|
    data = JSON.parse File.read(fileName)

    data["Objects"].each do |i|

        if i["Wear_Flags"] === 8
            i["Affect_Data"].each do |a|
                case a["Location"]
                when 1
                    SetSpawnChance a, 50
                when 2,5
                    SetSpawnChance a, 75
                when 13
                    SetSpawnChance a, 25
                end
            end
        end

        if i["Wear_Flags"] === 16
            i["Affect_Data"].each do |a|
                case a["Location"]
                when 12
                    SetSpawnChance a, 50
                end
            end
        end

        if i["Wear_Flags"] === 128
            i["Affect_Data"].each do |a|
                case a["Location"]
                when 1
                    SetSpawnChance a, 50
                when 18,19
                    SetSpawnChance a, 75
                end
            end
        end

        if i["Wear_Flags"] === 256
            i["Affect_Data"].each do |a|
                case a["Location"]
                when 5
                    SetSpawnChance a, 50
                when 19
                    SetSpawnChance a, 75
                end
            end
        end

        if i["Wear_Flags"] === 512
            i["Affect_Data"].each do |a|
                case a["Location"]
                when 1
                    SetSpawnChance a, 50
                when 5
                    SetSpawnChance a, 75
                end
            end
        end

        if i["Wear_Flags"] === 4096
            i["Affect_Data"].each do |a|
                case a["Location"]
                when 1
                    SetSpawnChance a, 50
                when 18,19
                    SetSpawnChance a, 75
                end
            end
        end

        if i["Wear_Flags"] === 8196
            i["Affect_Data"].each do |a|
                case a["Location"]
                when 18,19
                    SetSpawnChance a, 85
                end
            end
        end

        if i["Wear_Flags"] === 32768
            i["Affect_Data"].each do |a|
                case a["Location"]
                when 14
                    SetSpawnChance a, 50
                when 18,19
                    SetSpawnChance a, 75
                end
            end
        end

        if i["Wear_Flags"] === 65536
            i["Affect_Data"].each do |a|
                case a["Location"]
                when 18,19
                    SetSpawnChance a, 85
                end
            end
        end

        if [2, 4].include? i["Wear_Flags"]
            i["Affect_Data"].each do |a|
                case a["Location"]
                when 1..5
                    SetSpawnChance a, 50
                when 24
                    SetSpawnChance a, 10
                end
            end
        end

        if [32, 64].include? i["Wear_Flags"]
            i["Affect_Data"].each do |a|
                case a["Location"]
                when 14
                    SetSpawnChance a, 50
                end
            end
        end

        if [1024, 2048].include? i["Wear_Flags"]
            i["Affect_Data"].each do |a|
                case a["Location"]
                when 1..5
                    SetSpawnChance a, 50
                end
            end
        end


    end

    File.open(fileName, "w") do |f|
      f.write(JSON.pretty_generate(data))
    end
end