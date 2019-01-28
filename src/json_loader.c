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
extern int top_affect;
extern int top_ed;
extern int top_exit;
extern int top_rt;
extern int top_shop;
extern int top_special;
extern int top_help;

extern ROOM_INDEX_DATA *room_index_hash[MAX_KEY_HASH];
extern MOB_INDEX_DATA *mob_index_hash[MAX_KEY_HASH];
extern OBJ_INDEX_DATA *obj_index_hash[MAX_KEY_HASH];

extern AREA_DATA *area_last;
extern AREA_DATA *area_first;
extern SHOP_DATA *shop_first;
extern SHOP_DATA *shop_last;
extern SPEC_DATA *spec_first;
extern SPEC_DATA *spec_last;
extern HELP_DATA *help_first;
extern HELP_DATA *help_last;

extern char strArea[MAX_INPUT_LENGTH];
extern char *help_greeting;
extern FILE *fpArea;

bool devLogging = FALSE;

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
    char tempbuf[MAX_INPUT_LENGTH];
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
    cJSON_AddItemToObject(areaData, "min_vnum", cJSON_CreateNumber(area->min_vnum));
    cJSON_AddItemToObject(areaData, "max_vnum", cJSON_CreateNumber(area->max_vnum));

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
                    cJSON_AddItemToObject(affect_dataSingle, "min_modifier", cJSON_CreateNumber(paf->min_modifier));
                    cJSON_AddItemToObject(affect_dataSingle, "max_modifier", cJSON_CreateNumber(paf->max_modifier));
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
                if (devLogging)
                    log_string("Found a room");
                room = cJSON_CreateObject();

                cJSON_AddItemToArray(rooms, room);

                cJSON_AddItemToObject(room, "vnum", cJSON_CreateNumber(pRoomIndex->vnum));
                cJSON_AddItemToObject(room, "name", cJSON_CreateString(pRoomIndex->name));
                cJSON_AddItemToObject(room, "description", cJSON_CreateString(pRoomIndex->description));
                cJSON_AddItemToObject(room, "area_number", cJSON_CreateNumber(pRoomIndex->area_number));

                room_flags = cJSON_CreateArray();

                cJSON_AddItemToObject(room, "room_flags", room_flags);
                if (devLogging)
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

                if (devLogging)
                    log_string("Checking exits");

                for (int door = 0; door <= 5; door++)
                {
                    if (devLogging)
                        log_string("Checking if exit exists");
                    if ((pexit = pRoomIndex->exit[door]) != NULL)
                    {
                        if (devLogging)
                            log_string("Checking if to_room exists");
                        if (pexit->to_room != NULL)
                        {
                            if (devLogging)
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

                if (devLogging)
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

                if (devLogging)
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
        if (devLogging)
            log_string("Creating reset");
        reset = cJSON_CreateObject();

        cJSON_AddItemToArray(resets, reset);

        if (devLogging)
            log_string("Setting command");

        cJSON_AddItemToObject(reset, "command", cJSON_CreateString(&pReset->command));

        if (devLogging)
            log_string("Setting arguments");
        cJSON_AddItemToObject(reset, "arg1", cJSON_CreateNumber(pReset->arg1));
        cJSON_AddItemToObject(reset, "arg2", cJSON_CreateNumber(pReset->arg2));
        cJSON_AddItemToObject(reset, "arg3", cJSON_CreateNumber(pReset->arg3));
        cJSON_AddItemToObject(reset, "comment", cJSON_CreateString(pReset->comment));
    }

    shops = cJSON_CreateArray();
    cJSON_AddItemToObject(areaData, "shops", shops);

    log_string("Creating shops");
    for (pShop = shop_first; pShop != NULL; pShop = pShop->next)
    {
        if (!str_cmp(pShop->area->name, area->name))
        {
            if (devLogging)
                log_string("Creating shop");
            shop = cJSON_CreateObject();
            cJSON_AddItemToArray(shops, shop);

            cJSON_AddItemToObject(shop, "keeper", cJSON_CreateNumber(pShop->keeper));
            cJSON_AddItemToObject(shop, "profit_buy", cJSON_CreateNumber(pShop->profit_buy));
            cJSON_AddItemToObject(shop, "profit_sell", cJSON_CreateNumber(pShop->profit_sell));
            cJSON_AddItemToObject(shop, "open_hour", cJSON_CreateNumber(pShop->open_hour));
            cJSON_AddItemToObject(shop, "close_hour", cJSON_CreateNumber(pShop->close_hour));
            cJSON_AddItemToObject(shop, "comment", cJSON_CreateString(pShop->comment));

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
            if (devLogging)
                log_string("Creating Special");
            special = cJSON_CreateObject();
            cJSON_AddItemToArray(specials, special);

            cJSON_AddItemToObject(special, "vnum", cJSON_CreateNumber(pSpec->vnum));
            cJSON_AddItemToObject(special, "command", cJSON_CreateString(&pSpec->command));
            cJSON_AddItemToObject(special, "spec", cJSON_CreateString(pSpec->spec));
            cJSON_AddItemToObject(special, "comment", cJSON_CreateString(pSpec->comment));
        }
    }

    helps = cJSON_CreateArray();
    cJSON_AddItemToObject(areaData, "helps", helps);

    log_string("Creating helps");

    for (pHelp = help_first; pHelp != NULL; pHelp = pHelp->next)
    {
        if (!str_cmp(pHelp->area->name, area->name))
        {
            if (devLogging)
                log_string("Creating help");
            help = cJSON_CreateObject();

            cJSON_AddItemToArray(helps, help);

            cJSON_AddItemToObject(help, "level", cJSON_CreateNumber(pHelp->level));
            cJSON_AddItemToObject(help, "keyword", cJSON_CreateString(pHelp->keyword));
            cJSON_AddItemToObject(help, "text", cJSON_CreateString(pHelp->text));
        }
    }

    snprintf(buf, MAX_INPUT_LENGTH, "%s", area->file);

    buf[strlen(buf) - 3] = '\0';

    strcat(buf, "json");

    snprintf(tempbuf, MAX_INPUT_LENGTH, "Opening %s to save", buf);

    log_string(tempbuf);

    areaFile = fopen(buf, "ab+");

    fprintf(areaFile, "%s", cJSON_Print(areaData));

    snprintf(tempbuf, MAX_INPUT_LENGTH, "%s saved", buf);
    log_string(tempbuf);
    fclose(areaFile);

    cJSON_Delete(areaData);
}

void load_area_file_json(char *areaFile)
{
    extern bool fBootDb;
    AREA_DATA *pArea;
    char buf[MAX_STRING_LENGTH];

    fBootDb = TRUE;

    snprintf(buf, MAX_STRING_LENGTH, "Loading %s", areaFile);

    log_string(buf);

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
    pArea->name = jread_string(cJSON_GetObjectItemCaseSensitive(j_area, "name")->valuestring);
    pArea->creator = jread_string(cJSON_GetObjectItemCaseSensitive(j_area, "creator")->valuestring);

    pArea->min_vnum = 0;
    pArea->max_vnum = 0;
    cJSON *min, *max;
    if(min = cJSON_GetObjectItemCaseSensitive(j_area, "min_vnum") != NULL)
    {
        pArea->min_vnum = min->valuedouble;
    }
    
    if(max = cJSON_GetObjectItemCaseSensitive(j_area, "max_vnum") != NULL)
    {
        pArea->max_vnum = max->valuedouble;
    }

    pArea->wasModified = FALSE;
    pArea->wasModified = FALSE;
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

    // These are all separate calls to make everything less cluttered
    // This also helps with isolation and separate calls to load specific areas of a file
    load_mobiles_json(cJSON_GetObjectItemCaseSensitive(j_area, "mobiles"), pArea);
    load_objects_json(cJSON_GetObjectItemCaseSensitive(j_area, "objects"), pArea);
    load_rooms_json(cJSON_GetObjectItemCaseSensitive(j_area, "rooms"), pArea);
    load_resets_json(cJSON_GetObjectItemCaseSensitive(j_area, "resets"), pArea);
    load_shops_json(cJSON_GetObjectItemCaseSensitive(j_area, "shops"), pArea);
    load_specials_json(cJSON_GetObjectItemCaseSensitive(j_area, "specials"), pArea);
    load_helps_json(cJSON_GetObjectItemCaseSensitive(j_area, "helps"), pArea);

    fpArea = NULL;
    fBootDb = FALSE;

    free_mem(data, sizeof(char *));
    cJSON_Delete(j_area);
    return;
}

void load_mobiles_json(cJSON *mobiles, AREA_DATA *pArea)
{
    const cJSON *mobile = NULL;
    MOB_INDEX_DATA *pMobIndex;

    log_string("Loading mobiles");
    cJSON_ArrayForEach(mobile, mobiles)
    {
        if (devLogging)
            log_string("Loading mobile");
        int iHash;
        sh_int vnum;
        pMobIndex = NULL;
        pMobIndex = alloc_perm(sizeof(*pMobIndex));
        vnum = cJSON_GetObjectItemCaseSensitive(mobile, "vnum")->valuedouble;
        pMobIndex->vnum = vnum;
        pMobIndex->player_name = jread_string(cJSON_GetObjectItemCaseSensitive(mobile, "name")->valuestring);
        pMobIndex->short_descr = jread_string(cJSON_GetObjectItemCaseSensitive(mobile, "short_description")->valuestring);
        pMobIndex->long_descr = jread_string(cJSON_GetObjectItemCaseSensitive(mobile, "long_description")->valuestring);
        pMobIndex->description = jread_string(cJSON_GetObjectItemCaseSensitive(mobile, "description")->valuestring);
        pMobIndex->act = cJSON_GetObjectItemCaseSensitive(mobile, "act")->valuedouble;
        pMobIndex->affected_by = cJSON_GetObjectItemCaseSensitive(mobile, "affected_by")->valuedouble;
        pMobIndex->alignment = cJSON_GetObjectItemCaseSensitive(mobile, "alignment")->valuedouble;
        pMobIndex->level = cJSON_GetObjectItemCaseSensitive(mobile, "level")->valuedouble;
        pMobIndex->exp_level = cJSON_GetObjectItemCaseSensitive(mobile, "exp_level")->valuedouble;
        pMobIndex->hitroll = cJSON_GetObjectItemCaseSensitive(mobile, "hitroll")->valuedouble;
        pMobIndex->damplus = cJSON_GetObjectItemCaseSensitive(mobile, "damroll")->valuedouble;
        pMobIndex->ac = cJSON_GetObjectItemCaseSensitive(mobile, "ac")->valuedouble;
        pMobIndex->hitplus = cJSON_GetObjectItemCaseSensitive(mobile, "hp")->valuedouble;
        pMobIndex->gold = cJSON_GetObjectItemCaseSensitive(mobile, "gold")->valuedouble;
        pMobIndex->sex = cJSON_GetObjectItemCaseSensitive(mobile, "sex")->valuedouble;
        pMobIndex->area = pArea;
        pMobIndex->itemaffect = 0;
        pMobIndex->pShop = NULL;

        iHash = vnum % MAX_KEY_HASH;
        pMobIndex->next = mob_index_hash[iHash];
        mob_index_hash[iHash] = pMobIndex;
        pArea->mobiles++;
        top_mob_index++;
    }
}

void load_objects_json(cJSON *objects, AREA_DATA *pArea)
{
    const cJSON *object = NULL;
    const cJSON *numbers = NULL;
    const cJSON *number = NULL;
    const cJSON *affect_datas = NULL;
    const cJSON *affect_data = NULL;
    const cJSON *extra_descr_data = NULL;
    const cJSON *extra_descr_datas = NULL;

    OBJ_INDEX_DATA *pObjIndex;
    AFFECT_DATA *paf;
    EXTRA_DESCR_DATA *ed;

    log_string("Loading objects");
    cJSON_ArrayForEach(object, objects)
    {
        if (devLogging)
            log_string("Loading object");
        int iHash;
        sh_int vnum;
        long extra_flags = 0;
        long wear_flags = 0;
        pObjIndex = NULL;
        pObjIndex = alloc_perm(sizeof(*pObjIndex));
        vnum = cJSON_GetObjectItemCaseSensitive(object, "vnum")->valuedouble;
        pObjIndex->vnum = vnum;
        pObjIndex->name = jread_string(cJSON_GetObjectItemCaseSensitive(object, "name")->valuestring);
        pObjIndex->short_descr = jread_string(cJSON_GetObjectItemCaseSensitive(object, "short_description")->valuestring);
        pObjIndex->description = jread_string(cJSON_GetObjectItemCaseSensitive(object, "description")->valuestring);
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

        affect_datas = cJSON_GetObjectItemCaseSensitive(object, "affect_data");

        cJSON_ArrayForEach(affect_data, affect_datas)
        {
            paf = alloc_perm(sizeof(*paf));
            paf->type = -1;
            paf->duration = -1;
            paf->location = cJSON_GetObjectItemCaseSensitive(affect_data, "location")->valuedouble;
            paf->modifier = cJSON_GetObjectItemCaseSensitive(affect_data, "modifier")->valuedouble;
            paf->min_modifier = cJSON_GetObjectItemCaseSensitive(affect_data, "min_modifier")->valuedouble;
            paf->max_modifier = cJSON_GetObjectItemCaseSensitive(affect_data, "max_modifier")->valuedouble;
            paf->bitvector = 0;
            paf->next = pObjIndex->affected;
            pObjIndex->affected = paf;
            top_affect++;
        }

        extra_descr_datas = cJSON_GetObjectItemCaseSensitive(object, "extra_descr_data");

        cJSON_ArrayForEach(extra_descr_data, extra_descr_datas)
        {
            ed = alloc_perm(sizeof(*ed));
            ed->keyword = jread_string(cJSON_GetObjectItemCaseSensitive(extra_descr_data, "keyword")->valuestring);
            ed->description = jread_string(cJSON_GetObjectItemCaseSensitive(extra_descr_data, "description")->valuestring);
            ed->next = pObjIndex->extra_descr;
            pObjIndex->extra_descr = ed;
            top_ed++;
        }

        pObjIndex->chpoweron = jread_string(cJSON_GetObjectItemCaseSensitive(object, "chpoweron")->valuestring);
        pObjIndex->chpoweroff = jread_string(cJSON_GetObjectItemCaseSensitive(object, "chpoweroff")->valuestring);
        pObjIndex->chpoweruse = jread_string(cJSON_GetObjectItemCaseSensitive(object, "chpoweruse")->valuestring);
        pObjIndex->victpoweron = jread_string(cJSON_GetObjectItemCaseSensitive(object, "victpoweron")->valuestring);
        pObjIndex->victpoweroff = jread_string(cJSON_GetObjectItemCaseSensitive(object, "victpoweroff")->valuestring);
        pObjIndex->victpoweruse = jread_string(cJSON_GetObjectItemCaseSensitive(object, "victpoweruse")->valuestring);
        pObjIndex->spectype = cJSON_GetObjectItemCaseSensitive(object, "spectype")->valuedouble;
        pObjIndex->specpower = cJSON_GetObjectItemCaseSensitive(object, "specpower")->valuedouble;

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
}

void load_rooms_json(cJSON *rooms, AREA_DATA *pArea)
{
    const cJSON *extra_descr_data = NULL;
    const cJSON *extra_descr_datas = NULL;
    const cJSON *roomtext_data = NULL;
    const cJSON *roomtext_datas = NULL;
    const cJSON *room = NULL;
    const cJSON *exitSingle = NULL;
    const cJSON *exits = NULL;
    const cJSON *numbers = NULL;
    const cJSON *number = NULL;

    ROOMTEXT_DATA *rt;
    EXIT_DATA *pExit;
    ROOM_INDEX_DATA *pRoomIndex;
    EXTRA_DESCR_DATA *ed;

    log_string("Loading rooms");
    cJSON_ArrayForEach(room, rooms)
    {
        if (devLogging)
            log_string("Loading room");
        int iHash;
        long room_flags = 0;
        sh_int vnum;
        pRoomIndex = NULL;
        pRoomIndex = alloc_perm(sizeof(*pRoomIndex));

        pRoomIndex->people = NULL;
        pRoomIndex->contents = NULL;
        pRoomIndex->extra_descr = NULL;
        pRoomIndex->area = pArea;
        vnum = cJSON_GetObjectItemCaseSensitive(room, "vnum")->valuedouble;
        pRoomIndex->vnum = vnum;
        pRoomIndex->name = jread_string(cJSON_GetObjectItemCaseSensitive(room, "name")->valuestring);
        pRoomIndex->description = jread_string(cJSON_GetObjectItemCaseSensitive(room, "description")->valuestring);
        pRoomIndex->area_number = cJSON_GetObjectItemCaseSensitive(room, "area_number")->valuedouble;

        number = NULL;
        numbers = NULL;

        if (devLogging)
            log_string("Loading room_flags");
        numbers = cJSON_GetObjectItemCaseSensitive(room, "room_flags");
        cJSON_ArrayForEach(number, numbers)
        {
            room_flags += number->valuedouble;
        }

        pRoomIndex->room_flags = room_flags;
        pRoomIndex->sector_type = cJSON_GetObjectItemCaseSensitive(room, "sector_type")->valuedouble;
        pRoomIndex->light = 0;
        pRoomIndex->blood = 0;
        pRoomIndex->bomb = 0;

        exits = cJSON_GetObjectItemCaseSensitive(room, "exits");
        cJSON_ArrayForEach(exitSingle, exits)
        {
            int door = cJSON_GetObjectItemCaseSensitive(exitSingle, "door")->valuedouble;
            pExit = alloc_perm(sizeof(*pExit));
            pExit->vnum = cJSON_GetObjectItemCaseSensitive(exitSingle, "vnum")->valuedouble;
            pExit->description = jread_string(cJSON_GetObjectItemCaseSensitive(exitSingle, "description")->valuestring);
            pExit->keyword = jread_string(cJSON_GetObjectItemCaseSensitive(exitSingle, "keyword")->valuestring);
            pExit->key = cJSON_GetObjectItemCaseSensitive(exitSingle, "key")->valuedouble;
            pExit->exit_info = cJSON_GetObjectItemCaseSensitive(exitSingle, "exit_info")->valuedouble;
            pRoomIndex->exit[door] = pExit;
            top_exit++;
        }

        extra_descr_datas = cJSON_GetObjectItemCaseSensitive(room, "extra_descr_data");
        cJSON_ArrayForEach(extra_descr_data, extra_descr_datas)
        {
            ed = alloc_perm(sizeof(*ed));
            ed->keyword = jread_string(cJSON_GetObjectItemCaseSensitive(extra_descr_data, "keyword")->valuestring);
            ed->description = jread_string(cJSON_GetObjectItemCaseSensitive(extra_descr_data, "description")->valuestring);
            ed->next = pRoomIndex->extra_descr;
            pRoomIndex->extra_descr = ed;
            top_ed++;
        }

        roomtext_datas = cJSON_GetObjectItemCaseSensitive(room, "roomtext_data");
        cJSON_ArrayForEach(roomtext_data, roomtext_datas)
        {
            rt = alloc_perm(sizeof(*rt));
            rt->input = jread_string(cJSON_GetObjectItemCaseSensitive(roomtext_data, "input")->valuestring);
            rt->output = jread_string(cJSON_GetObjectItemCaseSensitive(roomtext_data, "output")->valuestring);
            rt->choutput = jread_string(cJSON_GetObjectItemCaseSensitive(roomtext_data, "choutput")->valuestring);
            rt->name = jread_string(cJSON_GetObjectItemCaseSensitive(roomtext_data, "name")->valuestring);
            rt->type = cJSON_GetObjectItemCaseSensitive(roomtext_data, "type")->valuedouble;
            rt->power = cJSON_GetObjectItemCaseSensitive(roomtext_data, "power")->valuedouble;
            rt->mob = cJSON_GetObjectItemCaseSensitive(roomtext_data, "mob")->valuedouble;

            rt->next = pRoomIndex->roomtext;
            pRoomIndex->roomtext = rt;
            top_rt++;
        }

        iHash = vnum % MAX_KEY_HASH;
        pRoomIndex->next = room_index_hash[iHash];
        room_index_hash[iHash] = pRoomIndex;

        pArea->rooms++;
        top_room++;
    }
}

void load_resets_json(cJSON *resets, AREA_DATA *pArea)
{
    const cJSON *reset = NULL;
    char buf[MAX_STRING_LENGTH];

    RESET_DATA *pReset;
    EXIT_DATA *pexit;
    ROOM_INDEX_DATA *pRoomIndex;

    log_string("Loading Resets");

    cJSON_ArrayForEach(reset, resets)
    {
        if (devLogging)
            log_string("Loading reset");
        pReset = NULL;
        pexit = NULL;
        pReset = alloc_perm(sizeof(*pReset));
        pReset->command = str_dup(cJSON_GetObjectItemCaseSensitive(reset, "command")->valuestring)[0];
        pReset->arg1 = cJSON_GetObjectItemCaseSensitive(reset, "arg1")->valuedouble;
        pReset->arg2 = cJSON_GetObjectItemCaseSensitive(reset, "arg2")->valuedouble;
        pReset->arg3 = cJSON_GetObjectItemCaseSensitive(reset, "arg3")->valuedouble;
        pReset->comment = str_dup(cJSON_GetObjectItemCaseSensitive(reset, "comment")->valuestring);

        switch (pReset->command)
        {
        default:
            bug("Load_resets: bad command '%c'.", pReset->command);
            exit(1);
            break;

        case 'M':
            get_mob_index(pReset->arg1);
            get_room_index(pReset->arg3);
            break;

        case 'O':
            get_obj_index(pReset->arg1);
            get_room_index(pReset->arg3);
            break;

        case 'P':
            get_obj_index(pReset->arg1);
            get_obj_index(pReset->arg3);
            break;

        case 'G':
        case 'E':
            get_obj_index(pReset->arg1);
            break;

        case 'D':
            pRoomIndex = get_room_index(pReset->arg1);

            if (pReset->arg2 < 0 || pReset->arg2 > 5 || (pexit = pRoomIndex->exit[pReset->arg2]) == NULL || !IS_SET(pexit->exit_info, EX_ISDOOR))
            {
                snprintf(buf, MAX_STRING_LENGTH, "Load_reset: %s 'D': room %ld, exit %ld, lock %ld not door.", pReset->comment, pReset->arg1, pReset->arg2, pReset->arg3);
                bug(buf, 0);
                exit(1);
            }

            if (pReset->arg3 < 0 || pReset->arg3 > 2)
            {
                bug("Load_resets: 'D': bad 'locks': %d.", pReset->arg3);
                exit(1);
            }

            break;

        case 'R':
            pRoomIndex = get_room_index(pReset->arg1);

            if (pReset->arg2 < 0 || pReset->arg2 > 6)
            {
                bug("Load_resets: 'R': bad exit %d.", pReset->arg2);
                exit(1);
            }

            break;
        }

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
}

void load_shops_json(cJSON *shops, AREA_DATA *pArea)
{
    SHOP_DATA *pShop;
    MOB_INDEX_DATA *pMobIndex;
    const cJSON *shop = NULL;
    const cJSON *number = NULL;
    const cJSON *numbers = NULL;

    log_string("Loading Shops");

    cJSON_ArrayForEach(shop, shops)
    {
        if (devLogging)
            log_string("Loading shop");
        pShop = NULL;
        pShop = alloc_perm(sizeof(*pShop));

        pShop->area = pArea;

        int iTrade = 0;

        pShop->keeper = cJSON_GetObjectItemCaseSensitive(shop, "keeper")->valuedouble;
        pShop->profit_buy = cJSON_GetObjectItemCaseSensitive(shop, "profit_buy")->valuedouble;
        pShop->profit_sell = cJSON_GetObjectItemCaseSensitive(shop, "profit_sell")->valuedouble;
        pShop->open_hour = cJSON_GetObjectItemCaseSensitive(shop, "open_hour")->valuedouble;
        pShop->close_hour = cJSON_GetObjectItemCaseSensitive(shop, "close_hour")->valuedouble;

        numbers = cJSON_GetObjectItemCaseSensitive(shop, "buy_type");
        cJSON_ArrayForEach(number, numbers)
        {
            pShop->buy_type[iTrade] = number->valuedouble;
            iTrade++;
        }

        pShop->comment = str_dup(cJSON_GetObjectItemCaseSensitive(shop, "comment")->valuestring);

        pMobIndex = get_mob_index(pShop->keeper);
        pMobIndex->pShop = pShop;

        if (shop_first == NULL)
            shop_first = pShop;
        if (shop_last != NULL)
            shop_last->next = pShop;

        pArea->shops++;
        shop_last = pShop;
        pShop->next = NULL;
        top_shop++;
    }
}

void load_specials_json(cJSON *specials, AREA_DATA *pArea)
{
    const cJSON *special = NULL;
    SPEC_DATA *pSpec;
    MOB_INDEX_DATA *pMobIndex;

    log_string("Loading Specials");

    cJSON_ArrayForEach(special, specials)
    {
        if (devLogging)
            log_string("Loading special");

        pSpec = NULL;
        pMobIndex = NULL;
        pSpec = alloc_perm(sizeof(*pSpec));
        pSpec->area = pArea;

        pSpec->vnum = cJSON_GetObjectItemCaseSensitive(special, "vnum")->valuedouble;
        pSpec->command = str_dup(cJSON_GetObjectItemCaseSensitive(special, "command")->valuestring)[0];
        pSpec->spec = str_dup(cJSON_GetObjectItemCaseSensitive(special, "spec")->valuestring);
        pSpec->comment = str_dup(cJSON_GetObjectItemCaseSensitive(special, "comment")->valuestring);

        pMobIndex = get_mob_index(pSpec->vnum);
        pMobIndex->spec_fun = spec_lookup(pSpec->spec);
        pMobIndex->spec = pSpec;

        if (pMobIndex->spec_fun == 0)
        {
            bug("Load_specials: spec_fun: vnum %d.", pMobIndex->vnum);
            exit(1);
        }

        pArea->specials++;

        if (spec_first == NULL)
            spec_first = pSpec;
        if (spec_last != NULL)
            spec_last->next = pSpec;

        spec_last = pSpec;
        pSpec->next = NULL;
        top_special++;
    }
}

void load_helps_json(cJSON *helps, AREA_DATA *pArea)
{
    const cJSON *help = NULL;
    HELP_DATA *pHelp;

    log_string("Loading Helps");

    cJSON_ArrayForEach(help, helps)
    {
        if (devLogging)
            log_string("Loading help");

        pHelp = NULL;

        pHelp = alloc_perm(sizeof(*pHelp));
        pHelp->area = pArea;
        pHelp->level = cJSON_GetObjectItemCaseSensitive(help, "level")->valuedouble;
        pHelp->keyword = jread_string(cJSON_GetObjectItemCaseSensitive(help, "keyword")->valuestring);
        pHelp->text = jread_string(cJSON_GetObjectItemCaseSensitive(help, "text")->valuestring);

        if (!str_cmp(pHelp->keyword, "greeting"))
        {
            log_string("Found greeting");
            help_greeting = pHelp->text;
        }

        pArea->helps++;

        if (help_first == NULL)
        {
            help_first = pHelp;
        }

        if (help_last != NULL)
        {
            help_last->next = pHelp;
        }
        help_last = pHelp;
        pHelp->next = NULL;
        top_help++;
    }
}