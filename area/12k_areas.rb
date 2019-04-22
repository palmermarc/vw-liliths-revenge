require "json"

# Area Files
files = ["mordor.json", "wyvern.json"]

def SetMinMax(object, min, max)
    object["Min_modifier"] = min
    object["Max_modifier"] = max
  end

files.each do |fileName|
    file = File.read(fileName)
    data = JSON.parse(file)

    data["Objects"].each do |i|
        if i["Item_Type"] === 5
            i["Affect_Data"].each do |a|
                case a["Location"]
                when 18
                    SetMinMax a, 130, 358
                when 19
                    SetMinMax a, 66, 180
                when 28
                    SetMinMax a, 5, 13
                end
            end
        end
        if i["Item_Type"] === 9
            if i["Wear_Flags"].include? 2 or i["Wear_Flags"].include? 4
                i["Affect_Data"].each do |a|
                    case a["Location"]
                    when 1..5
                        SetMinMax a, 9, 24
                    when 12, 14
                        SetMinMax a, 80, 220
                    when 13
                        SetMinMax a, 96, 264
                    when 24
                        SetMinMax a, 0, 0
                    end
                end
            end
            if i["Wear_Flags"].include? 1024 or i["Wear_Flags"].include? 2048
                i["Affect_Data"].each do |a|
                    case a["Location"]
                    when 1..5
                        SetMinMax a, 9, 24
                    when 12, 14
                        SetMinMax a, 80, 220
                    when 13
                        SetMinMax a, 96, 264
                    end
                end
            end
        end
        if i["Item_Type"] === 37
            if i["Wear_Flags"].include? 8
                i["Affect_Data"].each do |a|
                    case a["Location"]
                    when 1, 2, 5
                        SetMinMax a, 9, 24
                    when 13
                        SetMinMax a, 96, 264
                    when 30
                        SetMinMax a, 858, 2002
                    end
                end
            end
            if i["Wear_Flags"].include? 16
                i["Affect_Data"].each do |a|
                    case a["Location"]
                    when 3
                        SetMinMax a, 9, 24
                    when 12
                        SetMinMax a, 80, 220
                    when 30
                        SetMinMax a, 858, 2002
                    end
                end
            end

            if i["Wear_Flags"].include? 32 or i["Wear_Flags"].include? 64
                i["Affect_Data"].each do |a|
                    case a["Location"]
                    when 2
                        SetMinMax a, 9, 24
                    when 14
                        SetMinMax a, 80, 220
                    when 30
                        SetMinMax a, 858, 2002
                    end
                end
            end
        end
        if i["Item_Type"] === 38
            if i["Wear_Flags"].include? 8
                i["Affect_Data"].each do |a|
                    case a["Location"]
                    when 1, 2, 5
                        SetMinMax a, 9, 24
                    when 13
                        SetMinMax a, 96, 264
                    when 17, 30
                        SetMinMax a, 429, 1001
                    end
                end
            end
            if i["Wear_Flags"].include? 16
                i["Affect_Data"].each do |a|
                    case a["Location"]
                    when 3
                        SetMinMax a, 9, 24
                    when 12
                        SetMinMax a, 80, 220
                    when 17, 30
                        SetMinMax a, 429, 1001
                    end
                end
            end
            if i["Wear_Flags"].include? 32 or i["Wear_Flags"].include? 64
                i["Affect_Data"].each do |a|
                    case a["Location"]
                    when 2
                        SetMinMax a, 9, 24
                    when 14
                        SetMinMax a, 80, 220
                    when 17, 30
                        SetMinMax a, 429, 1001
                    end
                end
            end
        end
        if i["Item_Type"] === 39
            if i["Wear_Flags"].include? 8
                i["Affect_Data"].each do |a|
                    case a["Location"]
                    when 1, 2, 5
                        SetMinMax a, 9, 24
                    when 13
                        SetMinMax a, 96, 264
                    when 17
                        SetMinMax a, 858, 2002
                    end
                end
            end
            if i["Wear_Flags"].include? 16
                i["Affect_Data"].each do |a|
                    case a["Location"]
                    when 3
                        SetMinMax a, 9, 24
                    when 12
                        SetMinMax a, 80, 220
                    when 17
                        SetMinMax a, 858, 2002
                    end
                end
            end
            if i["Wear_Flags"].include? 32 or i["Wear_Flags"].include? 64
                i["Affect_Data"].each do |a|
                    case a["Location"]
                    when 2
                        SetMinMax a, 9, 24
                    when 14
                        SetMinMax a, 80, 220
                    when 17
                        SetMinMax a, 858, 2002
                    end
                end
            end
        end
        if [37, 38, 39].include? i["Item_Type"]
            if i["Wear_Flags"].include? 128
                i["Affect_Data"].each do |a|
                    case a["Location"]
                    when 1
                        SetMinMax a, 9, 24
                    when 18
                        SetMinMax a, 98, 269
                    when 19
                        SetMinMax a, 49, 135
                    when 28
                        SetMinMax a, 4, 10
                    end
                end
            end
            if i["Wear_Flags"].include? 256
                i["Affect_Data"].each do |a|
                    case a["Location"]
                    when 1, 5
                        SetMinMax a, 9, 24
                    when 18
                        SetMinMax a, 98, 269
                    when 19
                        SetMinMax a, 49, 135
                    end
                end
            end
            if i["Wear_Flags"].include? 4096
                i["Affect_Data"].each do |a|
                    case a["Location"]
                    when 1
                        SetMinMax a, 9, 24
                    when 18
                        SetMinMax a, 98, 269
                    when 19
                        SetMinMax a, 49, 135
                    when 28
                        SetMinMax a, 4, 10
                    end
                end
            end
            if i["Wear_Flags"].include? 32768
                i["Affect_Data"].each do |a|
                    case a["Location"]
                    when 3
                        SetMinMax a, 9, 24
                    when 12
                        SetMinMax a, 80, 220
                    when 18
                        SetMinMax a, 98, 269
                    when 19
                        SetMinMax a, 49, 135
                    end
                end
            end
        end
        if i["Item_Type"] === 40
            i["Affect_Data"].each do |a|
                case a["Location"]
                when 18
                    SetMinMax a, 390, 1073
                when 19
                    SetMinMax a, 196, 538
                when 28
                    SetMinMax a, 12, 30
                end
            end
        end
        if i["Item_Type"] === 41
            i["Affect_Data"].each do |a|
                case a["Location"]
                when 18
                    SetMinMax a, 130, 358
                when 19
                    SetMinMax a, 66, 180
                when 28
                    SetMinMax a, 5, 13
                end
            end
        end
        if i["Item_Type"] === 42
            i["Affect_Data"].each do |a|
                case a["Location"]
                when 1, 5
                    SetMinMax a, 9, 24
                when 17
                    SetMinMax a, 1073, 2145
                when 29
                    SetMinMax a, 10, 45
                end
            end
        end
    end

    File.open(fileName, "w") do |f|
      f.write(JSON.pretty_generate(data))
    end
end
