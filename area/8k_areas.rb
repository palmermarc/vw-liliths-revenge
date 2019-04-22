require "json"

# Area Files
files = ["arachnos.json", "chapel.json", "daycare.json", "galaxy.json", "gnome.json", "hitower.json", "hood.json", "htd.json", "sentinel.json"]

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
                    SetMinMax a, 20, 55
                when 19
                    SetMinMax a, 18, 49
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
                        SetMinMax a, 6, 17
                    when 12, 14
                        SetMinMax a, 53, 146
                    when 13
                        SetMinMax a, 64, 176
                    when 24
                        SetMinMax a, 0, 0
                    end
                end
            end
            if i["Wear_Flags"].include? 1024 or i["Wear_Flags"].include? 2048
                i["Affect_Data"].each do |a|
                    case a["Location"]
                    when 1..5
                        SetMinMax a, 6, 17
                    when 12, 14
                        SetMinMax a, 53, 146
                    when 13
                        SetMinMax a, 64, 176
                    end
                end
            end
        end
        if i["Item_Type"] === 37
            if i["Wear_Flags"].include? 8
                i["Affect_Data"].each do |a|
                    case a["Location"]
                    when 1, 2, 5
                        SetMinMax a, 6, 17
                    when 13
                        SetMinMax a, 64, 176
                    when 30
                        SetMinMax a, 225, 525
                    end
                end
            end
            if i["Wear_Flags"].include? 16
                i["Affect_Data"].each do |a|
                    case a["Location"]
                    when 3
                        SetMinMax a, 6, 17
                    when 12
                        SetMinMax a, 53, 146
                    when 30
                        SetMinMax a, 225, 525
                    end
                end
            end

            if i["Wear_Flags"].include? 32 or i["Wear_Flags"].include? 64
                i["Affect_Data"].each do |a|
                    case a["Location"]
                    when 2
                        SetMinMax a, 6, 17
                    when 14
                        SetMinMax a, 53, 146
                    when 30
                        SetMinMax a, 225, 525
                    end
                end
            end
        end
        if i["Item_Type"] === 38
            if i["Wear_Flags"].include? 8
                i["Affect_Data"].each do |a|
                    case a["Location"]
                    when 1, 2, 5
                        SetMinMax a, 6, 17
                    when 13
                        SetMinMax a, 64, 176
                    when 17, 30
                        SetMinMax a, 112, 262
                    end
                end
            end
            if i["Wear_Flags"].include? 16
                i["Affect_Data"].each do |a|
                    case a["Location"]
                    when 3
                        SetMinMax a, 6, 17
                    when 12
                        SetMinMax a, 53, 146
                    when 17, 30
                        SetMinMax a, 112, 262
                    end
                end
            end
            if i["Wear_Flags"].include? 32 or i["Wear_Flags"].include? 64
                i["Affect_Data"].each do |a|
                    case a["Location"]
                    when 2
                        SetMinMax a, 6, 17
                    when 14
                        SetMinMax a, 53, 146
                    when 17, 30
                        SetMinMax a, 112, 262
                    end
                end
            end
        end
        if i["Item_Type"] === 39
            if i["Wear_Flags"].include? 8
                i["Affect_Data"].each do |a|
                    case a["Location"]
                    when 1, 2, 5
                        SetMinMax a, 6, 17
                    when 13
                        SetMinMax a, 64, 176
                    when 17
                        SetMinMax a, 225, 525
                    end
                end
            end
            if i["Wear_Flags"].include? 16
                i["Affect_Data"].each do |a|
                    case a["Location"]
                    when 3
                        SetMinMax a, 6, 17
                    when 12
                        SetMinMax a, 53, 146
                    when 17
                        SetMinMax a, 225, 525
                    end
                end
            end
            if i["Wear_Flags"].include? 32 or i["Wear_Flags"].include? 64
                i["Affect_Data"].each do |a|
                    case a["Location"]
                    when 2
                        SetMinMax a, 6, 17
                    when 14
                        SetMinMax a, 53, 146
                    when 17
                        SetMinMax a, 225, 525
                    end
                end
            end
        end
        if [37, 38, 39].include? i["Item_Type"]
            if i["Wear_Flags"].include? 128
                i["Affect_Data"].each do |a|
                    case a["Location"]
                    when 1
                        SetMinMax a, 6, 17
                    when 18
                        SetMinMax a, 16, 42
                    when 19
                        SetMinMax a, 14, 37
                    when 28
                        SetMinMax a, 4, 10
                    end
                end
            end
            if i["Wear_Flags"].include? 256
                i["Affect_Data"].each do |a|
                    case a["Location"]
                    when 1, 5
                        SetMinMax a, 6, 17
                    when 18
                        SetMinMax a, 16, 42
                    when 19
                        SetMinMax a, 14, 37
                    end
                end
            end
            if i["Wear_Flags"].include? 4096
                i["Affect_Data"].each do |a|
                    case a["Location"]
                    when 1
                        SetMinMax a, 6, 17
                    when 18
                        SetMinMax a, 16, 42
                    when 19
                        SetMinMax a, 14, 37
                    when 28
                        SetMinMax a, 4, 10
                    end
                end
            end
            if i["Wear_Flags"].include? 32768
                i["Affect_Data"].each do |a|
                    case a["Location"]
                    when 3
                        SetMinMax a, 6, 17
                    when 12
                        SetMinMax a, 53, 146
                    when 18
                        SetMinMax a, 16, 42
                    when 19
                        SetMinMax a, 14, 37
                    end
                end
            end
        end
        if i["Item_Type"] === 40
            i["Affect_Data"].each do |a|
                case a["Location"]
                when 18
                    SetMinMax a, 60, 165
                when 19
                    SetMinMax a, 53, 146
                when 28
                    SetMinMax a, 12, 30
                end
            end
        end
        if i["Item_Type"] === 41
            i["Affect_Data"].each do |a|
                case a["Location"]
                when 18
                    SetMinMax a, 20, 55
                when 19
                    SetMinMax a, 18, 49
                when 28
                    SetMinMax a, 5, 13
                end
            end
        end
        if i["Item_Type"] === 42
            i["Affect_Data"].each do |a|
                case a["Location"]
                when 1, 5
                    SetMinMax a, 6, 17
                when 17
                    SetMinMax a, 282, 567
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
