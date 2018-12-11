#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "merc.h"
#include "cJSON.h"

extern int top_area;
extern int top_mob_index;
extern int top_room;
extern int top_reset;
extern int top_obj_index;
extern ROOM_INDEX_DATA *room_index_hash[MAX_KEY_HASH];
extern MOB_INDEX_DATA *mob_index_hash[MAX_KEY_HASH];
extern OBJ_INDEX_DATA *obj_index_hash[MAX_KEY_HASH];
extern AREA_DATA *area_last;
extern AREA_DATA *area_first;

extern char strArea[MAX_INPUT_LENGTH];
extern FILE *fpArea;

void save_area_file_json(AREA_DATA *area)
{
    FILE *areaFile;
    cJSON *mobiles = NULL;
    cJSON *mobile = NULL;
    cJSON *rooms = NULL;
    cJSON *room = NULL;
    cJSON *objects = NULL;
    cJSON *object = NULL;
    cJSON *resets = NULL;
    cJSON *reset = NULL;
    cJSON *shops = NULL;
    cJSON *shop = NULL;
    cJSON *specials = NULL;
    cJSON *special = NULL;
    cJSON *helps = NULL;
    cJSON *help = NULL;
    cJSON *numbers = NULL;
    cJSON *number = NULL;
    cJSON *extra_flags = NULL;
    cJSON *wear_flags = NULL;
    cJSON *room_flags = NULL;
    cJSON *affect_data = NULL;
    cJSON *affect_dataSingle = NULL;
    cJSON *extra_descr_data = NULL;
    cJSON *extra_descrSingle = NULL;
    cJSON *exits = NULL;
    cJSON *fexit = NULL;
    cJSON *roomtext_data = NULL;
    cJSON *roomtextSingle = NULL;
    MOB_INDEX_DATA *pMobIndex;
    OBJ_INDEX_DATA *pObjIndex;
    ROOM_INDEX_DATA *pRoomIndex;
    ROOMTEXT_DATA *roomText;
    EXIT_DATA *pexit;
    AFFECT_DATA *paf;
    EXTRA_DESCR_DATA *ed;
    RESET_DATA *pReset;
    HELP_DATA *pHelp;
    SHOP_DATA *pShop;
    SPEC_DATA *pSpec;
    char buf[MAX_INPUT_LENGTH];
    int iHash;

    log_string("Creating Area");
    cJSON *areaData = cJSON_CreateObject();

    if (areaData == NULL)
    {
        cJSON_Delete(areaData);
        return;
    }

    cJSON_AddItemToObject(areaData, "name", cJSON_CreateString(area->name));
    cJSON_AddItemToObject(areaData, "creator", cJSON_CreateString(area->creator));

    mobiles = cJSON_CreateArray();

    cJSON_AddItemToObject(areaData, "mobiles", mobiles);

    log_string("Creating mobiles");
    for (iHash = 0; iHash < MAX_KEY_HASH; iHash++)
    {
        for (pMobIndex = mob_index_hash[iHash]; pMobIndex != NULL; pMobIndex = pMobIndex->next)
        {
            if (!str_cmp(pMobIndex->area->name, area->name))
            {
                mobile = cJSON_CreateObject();
                cJSON_AddItemToArray(mobiles, mobile);

                cJSON_AddItemToObject(mobile, "vnum", cJSON_CreateNumber(pMobIndex->vnum));
                cJSON_AddItemToObject(mobile, "name", cJSON_CreateString(pMobIndex->player_name));
                cJSON_AddItemToObject(mobile, "short_description", cJSON_CreateString(pMobIndex->short_descr));
                cJSON_AddItemToObject(mobile, "long_description", cJSON_CreateString(pMobIndex->long_descr));
                cJSON_AddItemToObject(mobile, "description", cJSON_CreateString(pMobIndex->description));
                cJSON_AddItemToObject(mobile, "act", cJSON_CreateNumber(pMobIndex->act));
                cJSON_AddItemToObject(mobile, "affected_by", cJSON_CreateNumber(pMobIndex->affected_by));
                cJSON_AddItemToObject(mobile, "alignment", cJSON_CreateNumber(pMobIndex->alignment));
                cJSON_AddItemToObject(mobile, "level", cJSON_CreateNumber(pMobIndex->level));
                cJSON_AddItemToObject(mobile, "exp_level", cJSON_CreateNumber(pMobIndex->exp_level));
                cJSON_AddItemToObject(mobile, "hitroll", cJSON_CreateNumber(pMobIndex->hitroll));
                cJSON_AddItemToObject(mobile, "damroll", cJSON_CreateNumber(pMobIndex->damplus));
                cJSON_AddItemToObject(mobile, "ac", cJSON_CreateNumber(pMobIndex->ac));
                cJSON_AddItemToObject(mobile, "hp", cJSON_CreateNumber(pMobIndex->hitplus));
                cJSON_AddItemToObject(mobile, "gold", cJSON_CreateNumber(pMobIndex->gold));
                cJSON_AddItemToObject(mobile, "sex", cJSON_CreateNumber(pMobIndex->sex));
            }
        }
    }

    objects = cJSON_CreateArray();

    cJSON_AddItemToObject(areaData, "objects", objects);

    log_string("Creating objects");
    for (iHash = 0; iHash < MAX_KEY_HASH; iHash++)
    {
        for (pObjIndex = obj_index_hash[iHash]; pObjIndex != NULL; pObjIndex = pObjIndex->next)
        {
            if (!str_cmp(pObjIndex->area->name, area->name))
            {
                object = cJSON_CreateObject();

                cJSON_AddItemToArray(objects, object);

                cJSON_AddItemToObject(object, "vnum", cJSON_CreateNumber(pObjIndex->vnum));
                cJSON_AddItemToObject(object, "name", cJSON_CreateString(pObjIndex->name));
                cJSON_AddItemToObject(object, "short_description", cJSON_CreateString(pObjIndex->short_descr));
                cJSON_AddItemToObject(object, "description", cJSON_CreateString(pObjIndex->description));
                cJSON_AddItemToObject(object, "item_type", cJSON_CreateNumber(pObjIndex->item_type));

                extra_flags = cJSON_CreateArray();

                cJSON_AddItemToObject(object, "extra_flags", extra_flags);
                for (int i = 0; i < 32; i++)
                {
                    long value = pow(2, i);
                    if (IS_SET(pObjIndex->extra_flags, value))
                    {
                        cJSON_AddItemToArray(extra_flags, cJSON_CreateNumber(value));
                    }
                }

                wear_flags = cJSON_CreateArray();

                cJSON_AddItemToObject(object, "wear_flags", wear_flags);

                for (int i = 0; i < 32; i++)
                {
                    long value = pow(2, i);
                    if (IS_SET(pObjIndex->wear_flags, value))
                    {
                        cJSON_AddItemToArray(wear_flags, cJSON_CreateNumber(value));
                    }
                }

                cJSON_AddItemToObject(object, "value0", cJSON_CreateNumber(pObjIndex->value[0]));
                cJSON_AddItemToObject(object, "value1", cJSON_CreateNumber(pObjIndex->value[1]));
                cJSON_AddItemToObject(object, "value2", cJSON_CreateNumber(pObjIndex->value[2]));
                cJSON_AddItemToObject(object, "value3", cJSON_CreateNumber(pObjIndex->value[3]));

                cJSON_AddItemToObject(object, "weight", cJSON_CreateNumber(pObjIndex->weight));
                cJSON_AddItemToObject(object, "cost", cJSON_CreateNumber(pObjIndex->cost));

                affect_data = cJSON_CreateArray();

                cJSON_AddItemToObject(object, "affect_data", affect_data);

                for (paf = pObjIndex->affected; paf != NULL; paf = paf->next)
                {
                    affect_dataSingle = cJSON_CreateObject();
                    cJSON_AddItemToArray(affect_data, affect_dataSingle);
                    cJSON_AddItemToObject(affect_dataSingle, "location", cJSON_CreateNumber(paf->location));
                    cJSON_AddItemToObject(affect_dataSingle, "modifier", cJSON_CreateNumber(paf->modifier));
                }

                extra_descr_data = cJSON_CreateArray();

                cJSON_AddItemToObject(object, "extra_descr_data", extra_descr_data);

                for (ed = pObjIndex->extra_descr; ed != NULL; ed = ed->next)
                {
                    extra_descrSingle = cJSON_CreateObject();
                    cJSON_AddItemToArray(extra_descr_data, extra_descrSingle);
                    cJSON_AddItemToObject(extra_descrSingle, "keyword", cJSON_CreateString(ed->keyword));
                    cJSON_AddItemToObject(extra_descrSingle, "description", cJSON_CreateString(ed->description));
                }

                cJSON_AddItemToObject(object, "chpoweron", cJSON_CreateString(pObjIndex->chpoweron));
                cJSON_AddItemToObject(object, "chpoweroff", cJSON_CreateString(pObjIndex->chpoweroff));
                cJSON_AddItemToObject(object, "chpoweruse", cJSON_CreateString(pObjIndex->chpoweruse));

                cJSON_AddItemToObject(object, "victpoweron", cJSON_CreateString(pObjIndex->victpoweron));
                cJSON_AddItemToObject(object, "victpoweroff", cJSON_CreateString(pObjIndex->victpoweroff));
                cJSON_AddItemToObject(object, "victpoweruse", cJSON_CreateString(pObjIndex->victpoweruse));

                cJSON_AddItemToObject(object, "spectype", cJSON_CreateNumber(pObjIndex->spectype));
                cJSON_AddItemToObject(object, "specpower", cJSON_CreateNumber(pObjIndex->specpower));
            }
        }
    }

    rooms = cJSON_CreateArray();

    cJSON_AddItemToObject(areaData, "rooms", rooms);

    log_string("Creating rooms");

    for (iHash = 0; iHash < MAX_KEY_HASH; iHash++)
    {
        for (pRoomIndex = room_index_hash[iHash]; pRoomIndex != NULL; pRoomIndex = pRoomIndex->next)
        {
            if (!str_cmp(pRoomIndex->area->name, area->name))
            {
                log_string("Found a room");
                room = cJSON_CreateObject();

                cJSON_AddItemToArray(rooms, room);

                cJSON_AddItemToObject(room, "vnum", cJSON_CreateNumber(pRoomIndex->vnum));
                cJSON_AddItemToObject(room, "name", cJSON_CreateString(pRoomIndex->name));
                cJSON_AddItemToObject(room, "description", cJSON_CreateString(pRoomIndex->description));
                cJSON_AddItemToObject(room, "area_number", cJSON_CreateNumber(pRoomIndex->area_number));

                room_flags = cJSON_CreateArray();

                cJSON_AddItemToObject(room, "room_flags", room_flags);
                log_string("Checking room flags");
                for (int i = 0; i < 32; i++)
                {
                    long value = pow(2, i);
                    if (IS_SET(pRoomIndex->room_flags, value))
                    {
                        cJSON_AddItemToArray(room_flags, cJSON_CreateNumber(value));
                    }
                }

                cJSON_AddItemToObject(room, "sector_type", cJSON_CreateNumber(pRoomIndex->sector_type));

                exits = cJSON_CreateArray();

                cJSON_AddItemToObject(room, "exits", exits);

                log_string("Checking exits");

                for (int door = 0; door <= 5; door++)
                {
                    log_string("Checking if exit exists");
                    if (pexit = pRoomIndex->exit[door] != NULL)
                    {
                        log_string("Checking if to_room exists");
                        if (pexit->to_room != NULL)
                        {
                            log_string("Creating exit");
                            fexit = cJSON_CreateObject();

                            cJSON_AddItemToArray(exits, fexit);

                            

                            cJSON_AddItemToObject(fexit, "door", cJSON_CreateNumber(door));
                            cJSON_AddItemToObject(fexit, "vnum", cJSON_CreateNumber(pexit->vnum));
                            cJSON_AddItemToObject(fexit, "description", cJSON_CreateString(pexit->description));
                            cJSON_AddItemToObject(fexit, "keyword", cJSON_CreateString(pexit->keyword));
                            cJSON_AddItemToObject(fexit, "key", cJSON_CreateNumber(pexit->key));
                            cJSON_AddItemToObject(fexit, "exit_info", cJSON_CreateNumber(pexit->exit_info));
                        }
                        pexit = NULL;
                    }
                }

                extra_descr_data = cJSON_CreateArray();

                cJSON_AddItemToObject(room, "extra_descr_data", extra_descr_data);

                log_string("Extra descriptor data");

                for (ed = pRoomIndex->extra_descr; ed != NULL; ed = ed->next)
                {
                    extra_descrSingle = cJSON_CreateObject();
                    cJSON_AddItemToArray(extra_descr_data, extra_descrSingle);
                    cJSON_AddItemToObject(extra_descrSingle, "keyword", cJSON_CreateString(ed->keyword));
                    cJSON_AddItemToObject(extra_descrSingle, "description", cJSON_CreateString(ed->description));
                }

                roomtext_data = cJSON_CreateArray();

                cJSON_AddItemToObject(room, "roomtext_data", roomtext_data);

                log_string("Roomtext data");

                for (roomText = pRoomIndex->roomtext; roomText != NULL; roomText = roomText->next)
                {
                    roomtextSingle = cJSON_CreateObject();
                    cJSON_AddItemToArray(roomtext_data, roomtextSingle);

                    cJSON_AddItemToObject(roomtextSingle, "input", cJSON_CreateString(roomText->input));
                    cJSON_AddItemToObject(roomtextSingle, "output", cJSON_CreateString(roomText->output));
                    cJSON_AddItemToObject(roomtextSingle, "choutput", cJSON_CreateString(roomText->choutput));
                    cJSON_AddItemToObject(roomtextSingle, "name", cJSON_CreateString(roomText->name));
                    cJSON_AddItemToObject(roomtextSingle, "type", cJSON_CreateNumber(roomText->type));
                    cJSON_AddItemToObject(roomtextSingle, "power", cJSON_CreateNumber(roomText->power));
                    cJSON_AddItemToObject(roomtextSingle, "mob", cJSON_CreateNumber(roomText->mob));
                }
            }
        }
    }

    resets = cJSON_CreateArray();

    cJSON_AddItemToObject(areaData, "resets", resets);

    log_string("Creating resets");

    for (pReset = area->reset_first; pReset != NULL; pReset = pReset->next)
    {
        reset = cJSON_CreateObject();

        cJSON_AddItemToArray(resets, reset);

        cJSON_AddItemToObject(reset, "command", cJSON_CreateString(pReset->command));
        cJSON_AddItemToObject(reset, "arg1", cJSON_CreateNumber(pReset->arg1));
        cJSON_AddItemToObject(reset, "arg2", cJSON_CreateNumber(pReset->arg2));
        cJSON_AddItemToObject(reset, "arg3", cJSON_CreateNumber(pReset->arg3));
        cJSON_AddItemToObject(reset, "description", cJSON_CreateString(pReset->description));
    }

    shops = cJSON_CreateArray();
    cJSON_AddItemToObject(areaData, "shops", shops);

    for (pShop = shop_first; pShop != NULL; pShop = pShop->next)
    {
        if (!str_cmp(pShop->area->name, area->name))
        {
            shop = cJSON_CreateObject();
            cJSON_AddItemToArray(shops, shop);

            cJSON_AddItemToObject(shop, "keeper", cJSON_CreateNumber(pShop->keeper));
            cJSON_AddItemToObject(shop, "profit_buy", cJSON_CreateNumber(pShop->profit_buy));
            cJSON_AddItemToObject(shop, "profit_sell", cJSON_CreateNumber(pShop->profit_sell));
            cJSON_AddItemToObject(shop, "open_hour", cJSON_CreateNumber(pShop->open_hour));
            cJSON_AddItemToObject(shop, "close_hour", cJSON_CreateNumber(pShop->close_hour));

            numbers = cJSON_CreateArray();

            cJSON_AddItemToObject(shop, "buy_type", numbers);

            for (int i = 0; i < MAX_TRADE; i++)
            {
                cJSON_AddItemToArray(numbers, cJSON_CreateNumber(pShop->buy_type[i]));
            }
        }
    }

    specials = cJSON_CreateArray();
    cJSON_AddItemToObject(areaData, "specials", specials);

    log_string("Creating specials");

    for (pSpec = spec_first; pSpec != NULL; pSpec = pSpec->next)
    {
        if (!str_cmp(pSpec->area->name, area->name))
        {
            special = cJSON_CreateObject();
            cJSON_AddItemToArray(specials, special);

            cJSON_AddItemToObject(special, "vnum", cJSON_CreateNumber(pSpec->vnum));
            cJSON_AddItemToObject(special, "command", cJSON_CreateString(pSpec->command));
            cJSON_AddItemToObject(special, "spec", cJSON_CreateString(pSpec->spec));
        }
    }

    helps = cJSON_CreateArray();
    cJSON_AddItemToObject(areaData, "helps", helps);

    log_string("Creating helps");

    for (pHelp = help_first; pHelp != NULL; pHelp = pHelp->next)
    {
        if (!str_cmp(pHelp->area->name, area->name))
        {
            help = cJSON_CreateObject();

            cJSON_AddItemToArray(helps, help);

            cJSON_AddItemToObject(help, "level", cJSON_CreateNumber(pHelp->level));
            cJSON_AddItemToObject(help, "keyword", cJSON_CreateString(pHelp->keyword));
            cJSON_AddItemToObject(help, "text", cJSON_CreateString(pHelp->text));
        }
    }

    snprintf(buf, MAX_INPUT_LENGTH, "%s", area->file);

    log_string("Trying to save json file");

    buf[strlen(buf) - 3] = '\0';

    snprintf(buf, MAX_INPUT_LENGTH, "%s.json", buf);

    areaFile = fopen(buf, "ab+");
    fprintf(areaFile, "%s", areaData->valuestring);

    cJSON_Delete(areaData);
}

void load_area_file_json(char *areaFile)
{
    extern bool fBootDb;
    AREA_DATA *pArea;
    MOB_INDEX_DATA *pMobIndex;
    OBJ_INDEX_DATA *pObjIndex;
    ROOM_INDEX_DATA *pRoomIndex;
    RESET_DATA *pReset;
    const cJSON *mobiles = NULL;
    const cJSON *mobile = NULL;
    const cJSON *rooms = NULL;
    const cJSON *room = NULL;
    const cJSON *objects = NULL;
    const cJSON *object = NULL;
    const cJSON *resets = NULL;
    const cJSON *reset = NULL;
    const cJSON *numbers = NULL;
    const cJSON *number = NULL;

    log_string("Loading file");

    if ((fpArea = fopen(areaFile, "r")) == NULL)
    {
        perror(areaFile);
        exit(1);
    }

    fseek(fpArea, 0, SEEK_END);
    long fsize = ftell(fpArea);
    fseek(fpArea, 0, SEEK_SET);

    char *data = malloc(fsize + 1);
    fread(data, fsize, 1, fpArea);

    if (fpArea != stdin)
    {
        fclose(fpArea);
    }

    strncpy(strArea, areaFile, MAX_INPUT_LENGTH);
    fpArea = NULL;
    fBootDb = FALSE;

    data[fsize] = 0;

    cJSON *j_area = cJSON_Parse(data);

    if (j_area == NULL)
    {
        const char *error_ptr = cJSON_GetErrorPtr();
        if (error_ptr != NULL)
        {
            fprintf(stderr, "Error before: %s\n", error_ptr);
        }
        log_string("Error in j_area");
        cJSON_Delete(j_area);
        exit(1);
    }

    log_string("Loading area");
    // Load area
    pArea = alloc_perm(sizeof(*pArea));
    pArea->reset_first = NULL;
    pArea->reset_last = NULL;
    pArea->name = str_dup(cJSON_GetObjectItemCaseSensitive(j_area, "name")->valuestring);
    pArea->creator = str_dup(cJSON_GetObjectItemCaseSensitive(j_area, "creator")->valuestring);
    pArea->wasModified = FALSE;
    pArea->file = str_dup(areaFile);
    pArea->age = 15;
    pArea->nplayer = 0;
    pArea->helps = 0;
    pArea->mobiles = 0;
    pArea->rooms = 0;
    pArea->resets = 0;
    pArea->shops = 0;
    pArea->objects = 0;
    pArea->specials = 0;

    if (area_first == NULL)
        area_first = pArea;

    if (area_last != NULL)
    {
        area_last->next = pArea;
    }

    area_last = pArea;
    pArea->next = NULL;

    top_area++;

    // Check mobiles
    mobiles = cJSON_GetObjectItemCaseSensitive(j_area, "mobiles");

    log_string("Loading mobiles");
    cJSON_ArrayForEach(mobile, mobiles)
    {
        log_string("Loading mobile");
        int iHash;
        sh_int vnum;
        pMobIndex = alloc_perm(sizeof(*pMobIndex));
        vnum = cJSON_GetObjectItemCaseSensitive(mobile, "vnum")->valuedouble;
        pMobIndex->vnum = vnum;
        pMobIndex->player_name = str_dup(cJSON_GetObjectItemCaseSensitive(mobile, "name")->valuestring);
        pMobIndex->short_descr = str_dup(cJSON_GetObjectItemCaseSensitive(mobile, "short_description")->valuestring);
        pMobIndex->long_descr = str_dup(cJSON_GetObjectItemCaseSensitive(mobile, "long_description")->valuestring);
        pMobIndex->description = str_dup(cJSON_GetObjectItemCaseSensitive(mobile, "description")->valuestring);
        pMobIndex->act = cJSON_GetObjectItemCaseSensitive(mobile, "act")->valuedouble;
        pMobIndex->affected_by = cJSON_GetObjectItemCaseSensitive(mobile, "affected_by")->valuedouble;
        pMobIndex->alignment = cJSON_GetObjectItemCaseSensitive(mobile, "alignment")->valuedouble;
        pMobIndex->level = cJSON_GetObjectItemCaseSensitive(mobile, "level")->valuedouble;
        pMobIndex->hitroll = cJSON_GetObjectItemCaseSensitive(mobile, "hitroll")->valuedouble;
        pMobIndex->damplus = cJSON_GetObjectItemCaseSensitive(mobile, "damroll")->valuedouble;
        pMobIndex->ac = cJSON_GetObjectItemCaseSensitive(mobile, "ac")->valuedouble;
        pMobIndex->hitplus = cJSON_GetObjectItemCaseSensitive(mobile, "hp")->valuedouble;
        pMobIndex->gold = cJSON_GetObjectItemCaseSensitive(mobile, "gold")->valuedouble;
        pMobIndex->sex = cJSON_GetObjectItemCaseSensitive(mobile, "sex")->valuedouble;
        pMobIndex->area = pArea;

        iHash = vnum % MAX_KEY_HASH;
        pMobIndex->next = mob_index_hash[iHash];
        mob_index_hash[iHash] = pMobIndex;
        pArea->mobiles++;
        top_mob_index++;
    }

    objects = cJSON_GetObjectItemCaseSensitive(j_area, "objects");

    log_string("Loading objects");
    cJSON_ArrayForEach(object, objects)
    {
        log_string("Loading object");
        int iHash;
        sh_int vnum;
        long extra_flags = 0;
        long wear_flags = 0;
        pObjIndex = alloc_perm(sizeof(*pObjIndex));
        vnum = cJSON_GetObjectItemCaseSensitive(object, "vnum")->valuedouble;
        pObjIndex->vnum = vnum;
        pObjIndex->name = str_dup(cJSON_GetObjectItemCaseSensitive(object, "name")->valuestring);
        pObjIndex->short_descr = str_dup(cJSON_GetObjectItemCaseSensitive(object, "short_description")->valuestring);
        pObjIndex->description = str_dup(cJSON_GetObjectItemCaseSensitive(object, "long_description")->valuestring);
        pObjIndex->item_type = cJSON_GetObjectItemCaseSensitive(object, "item_type")->valuedouble;
        pObjIndex->area = pArea;

        numbers = cJSON_GetObjectItemCaseSensitive(object, "extra_flags");
        cJSON_ArrayForEach(number, numbers)
        {
            extra_flags += number->valuedouble;
        }
        number = NULL;
        numbers = NULL;
        pObjIndex->extra_flags = extra_flags;

        numbers = cJSON_GetObjectItemCaseSensitive(object, "wear_flags");
        cJSON_ArrayForEach(number, numbers)
        {
            wear_flags += number->valuedouble;
        }
        pObjIndex->wear_flags = wear_flags;
        pObjIndex->value[0] = cJSON_GetObjectItemCaseSensitive(object, "value0")->valuedouble;
        pObjIndex->value[1] = cJSON_GetObjectItemCaseSensitive(object, "value1")->valuedouble;
        pObjIndex->value[2] = cJSON_GetObjectItemCaseSensitive(object, "value2")->valuedouble;
        pObjIndex->value[3] = cJSON_GetObjectItemCaseSensitive(object, "value3")->valuedouble;
        pObjIndex->weight = cJSON_GetObjectItemCaseSensitive(object, "weight")->valuedouble;
        pObjIndex->cost = cJSON_GetObjectItemCaseSensitive(object, "cost")->valuedouble;
        //pObjIndex->affected = NULL; // Not currently setting these
        //pObjIndex->extra_descr = NULL // Not currently setting these

        switch (pObjIndex->item_type)
        {
        case ITEM_PILL:
        case ITEM_POTION:
        case ITEM_SCROLL:
            pObjIndex->value[1] = slot_lookup(pObjIndex->value[1]);
            pObjIndex->value[2] = slot_lookup(pObjIndex->value[2]);
            pObjIndex->value[3] = slot_lookup(pObjIndex->value[3]);
            break;

        case ITEM_BOMB:
            pObjIndex->value[3] = pObjIndex->value[3];
            break;

        case ITEM_STAFF:
        case ITEM_WAND:
            pObjIndex->value[3] = slot_lookup(pObjIndex->value[3]);
            break;
        }

        iHash = vnum % MAX_KEY_HASH;
        pObjIndex->next = obj_index_hash[iHash];
        obj_index_hash[iHash] = pObjIndex;

        pArea->objects++;
        top_obj_index++;
    }

    rooms = cJSON_GetObjectItemCaseSensitive(j_area, "rooms");

    log_string("Loading rooms");
    cJSON_ArrayForEach(room, rooms)
    {
        log_string("Loading room");
        int iHash;
        long room_flags = 0;
        sh_int vnum;
        pRoomIndex = alloc_perm(sizeof(*pRoomIndex));

        pRoomIndex->people = NULL;
        pRoomIndex->contents = NULL;
        pRoomIndex->extra_descr = NULL;
        pRoomIndex->area = pArea;
        vnum = cJSON_GetObjectItemCaseSensitive(room, "vnum")->valuedouble;
        pRoomIndex->vnum = vnum;
        pRoomIndex->name = str_dup(cJSON_GetObjectItemCaseSensitive(room, "name")->valuestring);
        pRoomIndex->description = str_dup(cJSON_GetObjectItemCaseSensitive(room, "description")->valuestring);
        pRoomIndex->area_number = cJSON_GetObjectItemCaseSensitive(room, "area_number")->valuedouble;

        number = NULL;
        numbers = NULL;

        log_string("Loading room_flags");
        numbers = cJSON_GetObjectItemCaseSensitive(room, "room_flags");
        cJSON_ArrayForEach(number, numbers)
        {
            room_flags += number->valuedouble;
        }

        pRoomIndex->room_flags = room_flags;
        pRoomIndex->sector_type = cJSON_GetObjectItemCaseSensitive(room, "sector_type")->valuedouble;

        iHash = vnum % MAX_KEY_HASH;
        pRoomIndex->next = room_index_hash[iHash];
        room_index_hash[iHash] = pRoomIndex;

        pArea->rooms++;
        top_room++;
    }

    resets = cJSON_GetObjectItemCaseSensitive(j_area, "resets");

    log_string("Loading Resets");

    cJSON_ArrayForEach(reset, resets)
    {
        log_string("Loading reset");
        pReset = alloc_perm(sizeof(*pReset));
        pReset->command = str_dup(cJSON_GetObjectItemCaseSensitive(reset, "command")->valuestring)[0];
        pReset->description = str_dup(cJSON_GetObjectItemCaseSensitive(reset, "description")->valuestring);
        pReset->arg1 = cJSON_GetObjectItemCaseSensitive(reset, "arg1")->valuedouble;
        pReset->arg2 = cJSON_GetObjectItemCaseSensitive(reset, "arg2")->valuedouble;
        pReset->arg3 = cJSON_GetObjectItemCaseSensitive(reset, "arg3")->valuedouble;

        if (pArea->reset_first == NULL)
        {
            pArea->reset_first = pReset;
        }

        if (pArea->reset_last != NULL)
        {
            pArea->reset_last->next = pReset;
        }

        pArea->reset_last = pReset;
        pArea->resets++;
        pReset->next = NULL;
        top_reset++;
    }

    free_mem(data, sizeof(char *));
    cJSON_Delete(j_area);
    return;
}