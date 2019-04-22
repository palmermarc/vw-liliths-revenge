require "json"

# Starting area files

# Realistically, this should be a hash that also stores the number for the SetLevel method below
#files = ["dwarven.json", "grave.json", "haon.json", "mobfact.json", "prehistoria.json", "tarzaroth.json", "wolfden.json"] #2k
files = ["canyon.json", "catacomb.json", "catacombs.json", "drow.json", "dylan.json", "marsh.json", "pomarj.json", "shire.json", "trollden.json", "vishnaak.json"] #4k
#files = ["celene.json", "dabour.json", "kree.json", "mahntor.json", "valley.json"] #6k
#files = ["arachnos.json", "chapel.json", "daycare.json", "galaxy.json", "gnome.json", "hitower.json", "hood.json", "htd.json", "sentinel.json"] #8k
#files = ["crystalm.json", "eastern.json", "firenewt.json", "grove.json", "juargan.json", "plains.json", "vallandar.json", "zoo.json"] #10k
#files = ["mordor.json", "wyvern.json"] #12k 
#files = ["hc.json"] #14k 
#files = ["icycaver.json", "notale.json"] #16k
#files = ["tanu.json", "ultima.json"] #20k
#files = ["bloodshrine.json", "pyramid.json", "tree.json"] #24k
#files = ["antharia.json", "solace.json"] #26k
#files = ["krytos.json"] #28k
#files = ["camelot.json"] #30k
#files = ["azurkeep.json"] #32k
#files = ["air.json"] #40k
#files = ["olympus.json"] #44k
#files = ["midgaard.json"] #50k

def SetLevel(level, mob)
    case level
    when 6
        mob["Level"] = 120
        mob["Exp_Level"] = 6000
        mob["Hitroll"] = 150
        mob["Damroll"] = 150
        mob["AC"] = 800
        mob["HP"] = 4800
    when 8
        mob["Level"] = 135
        mob["Exp_Level"] = 8000
        mob["Hitroll"] = 200
        mob["Damroll"] = 150
        mob["AC"] = 1500
        mob["HP"] = 9500
    when 10
        mob["Level"] = 145
        mob["Exp_Level"] = 10000
        mob["Hitroll"] = 700
        mob["Damroll"] = 400
        mob["AC"] = 2760
        mob["HP"] = 12834
    when 12
        mob["Level"] = 155
        mob["Exp_Level"] = 12000
        mob["Hitroll"] = 1300
        mob["Damroll"] = 650
        mob["AC"] = 5720
        mob["HP"] = 16168
    when 14
        mob["Level"] = 166
        mob["Exp_Level"] = 14000
        mob["Hitroll"] = 1800
        mob["Damroll"] = 900
        mob["AC"] = 7680
        mob["HP"] = 19502
    when 16
        mob["Level"] = 189
        mob["Exp_Level"] = 16000
        mob["Hitroll"] = 2300
        mob["Damroll"] = 1150
        mob["AC"] = 9640
        mob["HP"] = 22836
    when 20
        mob["Level"] = 235
        mob["Exp_Level"] = 20000
        mob["Hitroll"] = 3300
        mob["Damroll"] = 1650
        mob["AC"] = 13560
        mob["HP"] = 29504
    when 24
        mob["Level"] = 281
        mob["Exp_Level"] = 24000
        mob["Hitroll"] = 4300
        mob["Damroll"] = 2150
        mob["AC"] = 17480
        mob["HP"] = 36172
    when 26
        mob["Level"] = 304
        mob["Exp_Level"] = 26000
        mob["Hitroll"] = 4800
        mob["Damroll"] = 2400
        mob["AC"] = 19440
        mob["HP"] = 39506
    when 28
        mob["Level"] = 327
        mob["Exp_Level"] = 28000
        mob["Hitroll"] = 5300
        mob["Damroll"] = 2650
        mob["AC"] = 21400
        mob["HP"] = 42840
    when 30
        mob["Level"] = 350
        mob["Exp_Level"] = 30000
        mob["Hitroll"] = 5800
        mob["Damroll"] = 2900
        mob["AC"] = 23360
        mob["HP"] = 46174
    when 32 
        mob["Level"] = 373
        mob["Exp_Level"] = 32000
        mob["Hitroll"] = 6300
        mob["Damroll"] = 3150
        mob["AC"] = 25320
        mob["HP"] = 49508
    when 40
        mob["Level"] = 455
        mob["Exp_Level"] = 40000
        mob["Hitroll"] = 8300
        mob["Damroll"] = 4150
        mob["AC"] = 33160
        mob["HP"] = 62844
    when 44 
        mob["Level"] = 519
        mob["Exp_Level"] = 44000
        mob["Hitroll"] = 9300
        mob["Damroll"] = 4650
        mob["AC"] = 37080
        mob["HP"] = 69512
    when 50
        mob["Level"] = 600
        mob["Exp_Level"] = 50000
        mob["Hitroll"] = 10000
        mob["Damroll"] = 5000
        mob["AC"] = 40000
        mob["HP"] = 79514
    end
end

files.each do |fileName|
    file = File.read(fileName)
    data = JSON.parse(file)

    data["ItemRequirement"] = "4000"

    #data["Mobiles"].each do |m|
        #SetLevel(50, m)
    #end

    File.open(fileName, "w") do |f|
      f.write(JSON.pretty_generate(data))
    end
end