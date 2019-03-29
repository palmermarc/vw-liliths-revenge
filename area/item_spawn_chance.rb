require "json"

# Area files
files = Dir['./*.json']

def SetSpawnChance(item, chance)
    item["Spawn_chance"] = chance
end

files.each do |fileName|
    data = JSON.parse File.read(fileName)

    data["Objects"].each do |i|

        if i["Wear_Flags"].include? 8
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

        if i["Wear_Flags"].include? 16
            i["Affect_Data"].each do |a|
                case a["Location"]
                when 12
                    SetSpawnChance a, 50
                end
            end
        end

        if i["Wear_Flags"].include? 128
            i["Affect_Data"].each do |a|
                case a["Location"]
                when 1
                    SetSpawnChance a, 50
                when 18,19
                    SetSpawnChance a, 75
                end
            end
        end

        if i["Wear_Flags"].include? 256
            i["Affect_Data"].each do |a|
                case a["Location"]
                when 5
                    SetSpawnChance a, 50
                when 19
                    SetSpawnChance a, 75
                end
            end
        end

        if i["Wear_Flags"].include? 512
            i["Affect_Data"].each do |a|
                case a["Location"]
                when 1
                    SetSpawnChance a, 50
                when 5
                    SetSpawnChance a, 75
                end
            end
        end

        if i["Wear_Flags"].include? 4096
            i["Affect_Data"].each do |a|
                case a["Location"]
                when 1
                    SetSpawnChance a, 50
                when 18,19
                    SetSpawnChance a, 75
                end
            end
        end

        if i["Wear_Flags"].include? 8196
            i["Affect_Data"].each do |a|
                case a["Location"]
                when 18,19
                    SetSpawnChance a, 85
                end
            end
        end

        if i["Wear_Flags"].include? 32768
            i["Affect_Data"].each do |a|
                case a["Location"]
                when 14
                    SetSpawnChance a, 50
                when 18,19
                    SetSpawnChance a, 75
                end
            end
        end

        if i["Wear_Flags"].include? 65536
            i["Affect_Data"].each do |a|
                case a["Location"]
                when 18,19
                    SetSpawnChance a, 85
                end
            end
        end

        if i["Wear_Flags"].include? 2 or i["Wear_Flags"].include? 4
            i["Affect_Data"].each do |a|
                case a["Location"]
                when 1..5
                    SetSpawnChance a, 50
                when 24
                    SetSpawnChance a, 10
                end
            end
        end

        if i["Wear_Flags"].include? 32 or i["Wear_Flags"].include? 64
            i["Affect_Data"].each do |a|
                case a["Location"]
                when 14
                    SetSpawnChance a, 50
                end
            end
        end

        if i["Wear_Flags"].include? 1024 or i["Wear_Flags"].include? 2048
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