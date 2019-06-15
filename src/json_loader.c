#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
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
int        top_change;

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

CHANGE_DATA *change_first;
CHANGE_DATA *change_last;

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

                cJSON_AddItemToObject(object, "Value0", cJSON_CreateNumber(pObjIndex->value[0]));
                cJSON_AddItemToObject(object, "Value1", cJSON_CreateNumber(pObjIndex->value[1]));
                cJSON_AddItemToObject(object, "Value2", cJSON_CreateNumber(pObjIndex->value[2]));
                cJSON_AddItemToObject(object, "Value3", cJSON_CreateNumber(pObjIndex->value[3]));

                cJSON_AddItemToObject(object, "Weight", cJSON_CreateNumber(pObjIndex->weight));
                cJSON_AddItemToObject(object, "Cost", cJSON_CreateNumber(pObjIndex->cost));

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
                    cJSON_AddItemToObject(affect_dataSingle, "spawn_chance", cJSON_CreateNumber(paf->spawn_chance));
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
    int result = fread(data, fsize, 1, fpArea);

    if (fpArea != stdin)
    {
        fclose(fpArea);
    }

    //strncpy(strArea, areaFile, MAX_INPUT_LENGTH);

    data[fsize] = 0;

    cJSON *j_area = cJSON_Parse(data);

    if (j_area == NULL)
    {
        const char *error_ptr = cJSON_GetErrorPtr();
        if (error_ptr != NULL)
        {
            fprintf(stderr, "Error before: %s and result %d\n", error_ptr, result);
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
    pArea->name = jread_string(cJSON_GetObjectItemCaseSensitive(j_area, "Name")->valuestring);
    pArea->creator = jread_string(cJSON_GetObjectItemCaseSensitive(j_area, "Creator")->valuestring);
    pArea->min_vnum = 0;
    pArea->max_vnum = 0;
    cJSON *min, *max;
    if ((min = cJSON_GetObjectItemCaseSensitive(j_area, "Min_vnum")) != NULL)
    {
        pArea->min_vnum = min->valuedouble;
    }

    if ((max = cJSON_GetObjectItemCaseSensitive(j_area, "Max_vnum")) != NULL)
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
    load_mobiles_json(cJSON_GetObjectItemCaseSensitive(j_area, "Mobiles"), pArea);
    load_objects_json(cJSON_GetObjectItemCaseSensitive(j_area, "Objects"), pArea);
    load_rooms_json(cJSON_GetObjectItemCaseSensitive(j_area, "Rooms"), pArea);
    load_resets_json(cJSON_GetObjectItemCaseSensitive(j_area, "Resets"), pArea);
    load_shops_json(cJSON_GetObjectItemCaseSensitive(j_area, "Shops"), pArea);
    load_specials_json(cJSON_GetObjectItemCaseSensitive(j_area, "Specials"), pArea);
    load_helps_json(cJSON_GetObjectItemCaseSensitive(j_area, "Helps"), pArea);

    fpArea = NULL;
    fBootDb = FALSE;

    free_mem(data, sizeof(char *));
    cJSON_Delete(j_area);
    return;
}

void load_mobiles_json(cJSON *mobiles, AREA_DATA *pArea)
{
    const cJSON *mobile = NULL;
    const cJSON *numbers = NULL;
    const cJSON *number = NULL;
    MOB_INDEX_DATA *pMobIndex;

    log_string("Loading mobiles");
    cJSON_ArrayForEach(mobile, mobiles)
    {
        if (devLogging)
            log_string("Loading mobile");
        int iHash;
        long act_flags = 0, affected_flags = 0;
        sh_int vnum;
        pMobIndex = NULL;
        pMobIndex = alloc_perm(sizeof(*pMobIndex));
        vnum = cJSON_GetObjectItemCaseSensitive(mobile, "Vnum")->valuedouble;
        pMobIndex->vnum = vnum;
        pMobIndex->player_name = jread_string(cJSON_GetObjectItemCaseSensitive(mobile, "Name")->valuestring);
        pMobIndex->short_descr = jread_string(cJSON_GetObjectItemCaseSensitive(mobile, "Short_Description")->valuestring);
        pMobIndex->long_descr = jread_string(cJSON_GetObjectItemCaseSensitive(mobile, "Long_Description")->valuestring);
        pMobIndex->description = jread_string(cJSON_GetObjectItemCaseSensitive(mobile, "Description")->valuestring);

        numbers = cJSON_GetObjectItemCaseSensitive(mobile, "Act_Flags");
        cJSON_ArrayForEach(number, numbers)
        {
            act_flags += number->valuedouble;
        }
        number = NULL;
        numbers = NULL;
        pMobIndex->act = act_flags;

        numbers = cJSON_GetObjectItemCaseSensitive(mobile, "Affected_Flags");
        cJSON_ArrayForEach(number, numbers)
        {
            affected_flags += number->valuedouble;
        }
        number = NULL;
        numbers = NULL;
        pMobIndex->affected_by = affected_flags;
        pMobIndex->alignment = cJSON_GetObjectItemCaseSensitive(mobile, "Alignment")->valuedouble;
        pMobIndex->level = cJSON_GetObjectItemCaseSensitive(mobile, "Level")->valuedouble;
        pMobIndex->exp_level = cJSON_GetObjectItemCaseSensitive(mobile, "Exp_Level")->valuedouble;
        pMobIndex->hitroll = cJSON_GetObjectItemCaseSensitive(mobile, "Hitroll")->valuedouble;
        pMobIndex->damplus = cJSON_GetObjectItemCaseSensitive(mobile, "Damroll")->valuedouble;
        pMobIndex->ac = cJSON_GetObjectItemCaseSensitive(mobile, "AC")->valuedouble;
        pMobIndex->hitplus = cJSON_GetObjectItemCaseSensitive(mobile, "HP")->valuedouble;
        pMobIndex->gold = cJSON_GetObjectItemCaseSensitive(mobile, "Gold")->valuedouble;
        pMobIndex->sex = cJSON_GetObjectItemCaseSensitive(mobile, "Sex")->valuedouble;
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
        if (devLogging) log_string("Loading object, pulling in base info");
        int iHash;
        sh_int vnum;
        long extra_flags = 0;
        long wear_flags = 0;
        pObjIndex = NULL;
        pObjIndex = alloc_perm(sizeof(*pObjIndex));
        vnum = cJSON_GetObjectItemCaseSensitive(object, "Vnum")->valuedouble;
        pObjIndex->vnum = vnum;
        char * tempName = jread_string(cJSON_GetObjectItemCaseSensitive(object, "Name")->valuestring);

        pObjIndex->name = tempName;
        pObjIndex->short_descr = jread_string(cJSON_GetObjectItemCaseSensitive(object, "Short_Description")->valuestring);
        pObjIndex->description = jread_string(cJSON_GetObjectItemCaseSensitive(object, "Description")->valuestring);
        pObjIndex->item_type = cJSON_GetObjectItemCaseSensitive(object, "Item_Type")->valuedouble;
        pObjIndex->area = pArea;

        if (devLogging) log_string("Pulling in Extra_Flags");

        numbers = cJSON_GetObjectItemCaseSensitive(object, "Extra_Flags");
        cJSON_ArrayForEach(number, numbers)
        {
            extra_flags += number->valuedouble;
        }
        number = NULL;
        numbers = NULL;
        pObjIndex->extra_flags = extra_flags;

        if (devLogging) log_string("Pulling in Wear_Flags");

        numbers = cJSON_GetObjectItemCaseSensitive(object, "Wear_Flags");
        cJSON_ArrayForEach(number, numbers)
        {
            wear_flags += number->valuedouble;
        }
        pObjIndex->wear_flags = wear_flags;
        pObjIndex->value[0] = cJSON_GetObjectItemCaseSensitive(object, "Value0")->valuedouble;
        pObjIndex->value[1] = cJSON_GetObjectItemCaseSensitive(object, "Value1")->valuedouble;
        pObjIndex->value[2] = cJSON_GetObjectItemCaseSensitive(object, "Value2")->valuedouble;
        pObjIndex->value[3] = cJSON_GetObjectItemCaseSensitive(object, "Value3")->valuedouble;
        pObjIndex->weight = cJSON_GetObjectItemCaseSensitive(object, "Weight")->valuedouble;
        pObjIndex->cost = cJSON_GetObjectItemCaseSensitive(object, "Cost")->valuedouble;

        if (devLogging) log_string("Pulling in Affect_Data");

        affect_datas = cJSON_GetObjectItemCaseSensitive(object, "Affect_Data");

        

        cJSON_ArrayForEach(affect_data, affect_datas)
        {
            paf = alloc_perm(sizeof(*paf));
            cJSON *min_mod = NULL, *max_mod = NULL, *mod = NULL, *spawn_chance = NULL;
            paf->type = -1;
            paf->duration = -1;
            paf->location = cJSON_GetObjectItem(affect_data, "Location")->valuedouble;
            paf->modifier = 0;
            paf->min_modifier = 0;
            paf->max_modifier = 0;
            paf->spawn_chance = 0;
            if ((mod = cJSON_GetObjectItem(affect_data, "Modifier")) != NULL)
                paf->modifier = mod->valuedouble;
            if ((min_mod = cJSON_GetObjectItem(affect_data, "Min_modifier")) != NULL)
                paf->min_modifier = min_mod->valuedouble;
            if ((max_mod = cJSON_GetObjectItem(affect_data, "Max_modifier")) != NULL)
                paf->max_modifier = max_mod->valuedouble;
            if ((spawn_chance = cJSON_GetObjectItem(affect_data, "Spawn_chance")) != NULL)
                paf->spawn_chance = spawn_chance->valuedouble;
            paf->bitvector = 0;
            paf->next = pObjIndex->affected;
            pObjIndex->affected = paf;
            top_affect++;
        }

        if (devLogging) log_string("Pulling in Extra_Descr_Data");

        extra_descr_datas = cJSON_GetObjectItemCaseSensitive(object, "Extra_Descr_Data");

        cJSON_ArrayForEach(extra_descr_data, extra_descr_datas)
        {
            ed = alloc_perm(sizeof(*ed));
            ed->keyword = jread_string(cJSON_GetObjectItemCaseSensitive(extra_descr_data, "Keyword")->valuestring);
            ed->description = jread_string(cJSON_GetObjectItemCaseSensitive(extra_descr_data, "Description")->valuestring);
            ed->next = pObjIndex->extra_descr;
            pObjIndex->extra_descr = ed;
            top_ed++;
        }

        if (devLogging) log_string("Setting PowerOn/Off/Spec Info");

        pObjIndex->chpoweron = jread_string(cJSON_GetObjectItemCaseSensitive(object, "ChPowerOn")->valuestring);
        pObjIndex->chpoweroff = jread_string(cJSON_GetObjectItemCaseSensitive(object, "ChPowerOff")->valuestring);
        pObjIndex->chpoweruse = jread_string(cJSON_GetObjectItemCaseSensitive(object, "ChPowerUse")->valuestring);
        pObjIndex->victpoweron = jread_string(cJSON_GetObjectItemCaseSensitive(object, "VictPowerOn")->valuestring);
        pObjIndex->victpoweroff = jread_string(cJSON_GetObjectItemCaseSensitive(object, "VictPowerOff")->valuestring);
        pObjIndex->victpoweruse = jread_string(cJSON_GetObjectItemCaseSensitive(object, "VictPowerUse")->valuestring);
        pObjIndex->spectype = cJSON_GetObjectItemCaseSensitive(object, "SpecType")->valuedouble;
        pObjIndex->specpower = cJSON_GetObjectItemCaseSensitive(object, "SpecPower")->valuedouble;

        if (devLogging) log_string("Looking up Item types");
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

        if (devLogging) log_string("Getting vnum hash and adding to index");

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
        sh_int exit_flags = 0;
        sh_int vnum;
        pRoomIndex = NULL;
        pRoomIndex = alloc_perm(sizeof(*pRoomIndex));

        pRoomIndex->people = NULL;
        pRoomIndex->contents = NULL;
        pRoomIndex->extra_descr = NULL;
        pRoomIndex->area = pArea;
        vnum = cJSON_GetObjectItemCaseSensitive(room, "Vnum")->valuedouble;
        pRoomIndex->vnum = vnum;
        pRoomIndex->name = jread_string(cJSON_GetObjectItemCaseSensitive(room, "Name")->valuestring);
        pRoomIndex->description = jread_string(cJSON_GetObjectItemCaseSensitive(room, "Description")->valuestring);
        pRoomIndex->area_number = cJSON_GetObjectItemCaseSensitive(room, "Area_Number")->valuedouble;

        number = NULL;
        numbers = NULL;

        if (devLogging)
            log_string("Loading room_flags");
        numbers = cJSON_GetObjectItemCaseSensitive(room, "Room_Flags");
        cJSON_ArrayForEach(number, numbers)
        {
            room_flags += number->valuedouble;
        }

        number = NULL;
        numbers = NULL;

        pRoomIndex->room_flags = room_flags;
        pRoomIndex->sector_type = cJSON_GetObjectItemCaseSensitive(room, "Sector_Type")->valuedouble;
        pRoomIndex->light = 0;
        pRoomIndex->blood = 0;
        pRoomIndex->bomb = 0;

        exits = cJSON_GetObjectItemCaseSensitive(room, "Exits");
        cJSON_ArrayForEach(exitSingle, exits)
        {
            exit_flags = 0;
            int door = cJSON_GetObjectItemCaseSensitive(exitSingle, "Door")->valuedouble;
            pExit = alloc_perm(sizeof(*pExit));
            pExit->vnum = cJSON_GetObjectItemCaseSensitive(exitSingle, "Vnum")->valuedouble;
            pExit->description = jread_string(cJSON_GetObjectItemCaseSensitive(exitSingle, "Description")->valuestring);
            pExit->keyword = jread_string(cJSON_GetObjectItemCaseSensitive(exitSingle, "Keyword")->valuestring);
            pExit->key = cJSON_GetObjectItemCaseSensitive(exitSingle, "Key")->valuedouble;

            numbers = cJSON_GetObjectItemCaseSensitive(exitSingle, "ExitInfo");
            cJSON_ArrayForEach(number, numbers)
            {
                exit_flags += number->valuedouble;
            }
            number = NULL;
            numbers = NULL;
            pExit->exit_info = exit_flags;

            pRoomIndex->exit[door] = pExit;
            top_exit++;
        }

        extra_descr_datas = cJSON_GetObjectItemCaseSensitive(room, "Extra_Descr_Data");
        cJSON_ArrayForEach(extra_descr_data, extra_descr_datas)
        {
            ed = alloc_perm(sizeof(*ed));
            ed->keyword = jread_string(cJSON_GetObjectItemCaseSensitive(extra_descr_data, "Keyword")->valuestring);
            ed->description = jread_string(cJSON_GetObjectItemCaseSensitive(extra_descr_data, "Description")->valuestring);
            ed->next = pRoomIndex->extra_descr;
            pRoomIndex->extra_descr = ed;
            top_ed++;
        }

        roomtext_datas = cJSON_GetObjectItemCaseSensitive(room, "RoomText_Data");
        cJSON_ArrayForEach(roomtext_data, roomtext_datas)
        {
            rt = alloc_perm(sizeof(*rt));
            rt->input = jread_string(cJSON_GetObjectItemCaseSensitive(roomtext_data, "Input")->valuestring);
            rt->output = jread_string(cJSON_GetObjectItemCaseSensitive(roomtext_data, "Output")->valuestring);
            rt->choutput = jread_string(cJSON_GetObjectItemCaseSensitive(roomtext_data, "ChOutput")->valuestring);
            rt->name = jread_string(cJSON_GetObjectItemCaseSensitive(roomtext_data, "Name")->valuestring);
            rt->type = cJSON_GetObjectItemCaseSensitive(roomtext_data, "Type")->valuedouble;
            rt->power = cJSON_GetObjectItemCaseSensitive(roomtext_data, "Power")->valuedouble;
            rt->mob = cJSON_GetObjectItemCaseSensitive(roomtext_data, "Mob")->valuedouble;

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
        pReset->command = str_dup(cJSON_GetObjectItemCaseSensitive(reset, "Command")->valuestring)[0];
        pReset->arg1 = cJSON_GetObjectItemCaseSensitive(reset, "Arg1")->valuedouble;
        pReset->arg2 = cJSON_GetObjectItemCaseSensitive(reset, "Arg2")->valuedouble;
        pReset->arg3 = cJSON_GetObjectItemCaseSensitive(reset, "Arg3")->valuedouble;
        pReset->comment = str_dup(cJSON_GetObjectItemCaseSensitive(reset, "Comment")->valuestring);

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
                if(pReset->arg2 < 0 || pReset->arg2 > 5)
                {
                    bug("arg2 needs to be 0-5",0);
                }
                if((pexit = pRoomIndex->exit[pReset->arg2]) == NULL)
                {
                    bug("Exit on arg2 is null", 0);
                }

                if(!IS_SET(pexit->exit_info, EX_ISDOOR))
                {
                    snprintf(buf, MAX_STRING_LENGTH, "Exit %s, %d is not a door", pexit->keyword, pexit->exit_info);
                    log_string(buf);
                }

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

        pShop->keeper = cJSON_GetObjectItemCaseSensitive(shop, "Keeper")->valuedouble;
        pShop->profit_buy = cJSON_GetObjectItemCaseSensitive(shop, "Profit_Buy")->valuedouble;
        pShop->profit_sell = cJSON_GetObjectItemCaseSensitive(shop, "Profit_Sell")->valuedouble;
        pShop->open_hour = cJSON_GetObjectItemCaseSensitive(shop, "Open_Hour")->valuedouble;
        pShop->close_hour = cJSON_GetObjectItemCaseSensitive(shop, "Close_Hour")->valuedouble;

        numbers = cJSON_GetObjectItemCaseSensitive(shop, "Buy_Type");
        cJSON_ArrayForEach(number, numbers)
        {
            pShop->buy_type[iTrade] = number->valuedouble;
            iTrade++;
        }

        pShop->comment = str_dup(cJSON_GetObjectItemCaseSensitive(shop, "Comment")->valuestring);

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

        pSpec->vnum = cJSON_GetObjectItemCaseSensitive(special, "Vnum")->valuedouble;
        pSpec->command = str_dup(cJSON_GetObjectItemCaseSensitive(special, "Command")->valuestring)[0];
        pSpec->spec = str_dup(cJSON_GetObjectItemCaseSensitive(special, "Spec")->valuestring);
        pSpec->comment = str_dup(cJSON_GetObjectItemCaseSensitive(special, "Comment")->valuestring);

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
        pHelp->level = cJSON_GetObjectItemCaseSensitive(help, "Level")->valuedouble;
        pHelp->keyword = jread_string(cJSON_GetObjectItemCaseSensitive(help, "Keyword")->valuestring);
        pHelp->text = jread_string(cJSON_GetObjectItemCaseSensitive(help, "Text")->valuestring);

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

void load_changes_json(char * file)
{
    CHANGE_DATA *pChange;
    cJSON *change = NULL;
    cJSON *changes = NULL;
    FILE *changeFile;

    char buf[MAX_STRING_LENGTH];

    snprintf(buf, MAX_STRING_LENGTH, "Loading %s", file);

    log_string(buf);

    if ((changeFile = fopen(file, "r")) == NULL)
    {
        perror(file);
        exit(1);
    }

    fseek(changeFile, 0, SEEK_END);
    long fsize = ftell(changeFile);
    fseek(changeFile, 0, SEEK_SET);

    char *data = malloc(fsize + 1);
    int result = fread(data, fsize, 1, changeFile);

    if (changeFile != stdin)
    {
        fclose(changeFile);
    }

    data[fsize] = 0;

    changes = cJSON_Parse(data);

    if (changes == NULL)
    {
        const char *error_ptr = cJSON_GetErrorPtr();
        if (error_ptr != NULL)
        {
            fprintf(stderr, "Error before: %s and result %d\n", error_ptr, result);
        }
        log_string("Error in loading changes");
        cJSON_Delete(changes);
        exit(1);
    }

    cJSON_ArrayForEach(change, changes)
    {
        pChange = NULL;

        pChange = alloc_perm(sizeof(*pChange));

        pChange->changeId = cJSON_GetObjectItemCaseSensitive(change, "ChangeId")->valuedouble;
        pChange->category = jread_string(cJSON_GetObjectItemCaseSensitive(change, "Category")->valuestring);
        strptime(jread_string(cJSON_GetObjectItemCaseSensitive(change, "Date")->valuestring), "%D", &pChange->date);
        pChange->message = jread_string(cJSON_GetObjectItemCaseSensitive(change, "Message")->valuestring);

        if (change_first == NULL)
        {
            change_first = pChange;
        }

        if (change_last != NULL)
        {
            change_last->next = pChange;
        }
        change_last = pChange;
        pChange->next = NULL;
        top_change++;
    }

    free_mem(data, sizeof(char *));
    cJSON_Delete(changes);
    return;
}

void save_player_file_json(CHAR_DATA *ch)
{
	FILE *charFile;

	char buf[MAX_INPUT_LENGTH];
	char tempbuf[MAX_INPUT_LENGTH];
	int iHash;

	cJSON *pk = NULL;
	cJSON *weapons = NULL;
	cJSON *spells = NULL;
	cJSON *blue = NULL;
	cJSON *red = NULL;
	cJSON *yellow = NULL;
	cJSON *green = NULL;
	cJSON *purple = NULL;

	cJSON *stances = NULL;
	cJSON *items = NULL;
	cJSON *clandiscs = NULL;


	log_string("Creating Player");
	cJSON *charData = cJSON_CreateObject();

	cJSON_AddItemToObject(charData, "name", cJSON_CreateString(ch->name));
    cJSON_AddItemToObject(charData, "short_description", cJSON_CreateString(ch->short_descr));
    cJSON_AddItemToObject(charData, "long_description", cJSON_CreateString(ch->long_descr));
    cJSON_AddItemToObject(charData, "description", cJSON_CreateString(ch->description));

    if (ch->prompt != NULL || !str_cmp(ch->prompt, "<%hhp %mm %vmv> "))
    {
    	cJSON_AddItemToObject(charData, "prompt", cJSON_CreateString(ch->prompt));
    	//fprintf(fp, " %s~\n", ch->);
    }

    cJSON_AddItemToObject(charData, "lord", cJSON_CreateString(ch->lord));
    cJSON_AddItemToObject(charData, "clan", cJSON_CreateString(ch->clan));
    cJSON_AddItemToObject(charData, "morph", cJSON_CreateString(ch->morph));
    cJSON_AddItemToObject(charData, "create_time", cJSON_CreateString(ch->createtime));
    cJSON_AddItemToObject(charData, "last_time", cJSON_CreateString(ch->lasttime));
    cJSON_AddItemToObject(charData, "last_host", cJSON_CreateString(ch->lasthost));
    cJSON_AddItemToObject(charData, "power_action", cJSON_CreateString(ch->poweraction));
    cJSON_AddItemToObject(charData, "power_type", cJSON_CreateString(ch->powertype));
    cJSON_AddItemToObject(charData, "sex", cJSON_CreateNumber(ch->sex));
    cJSON_AddItemToObject(charData, "class", cJSON_CreateNumber(ch->class));
    cJSON_AddItemToObject(charData, "status", cJSON_CreateNumber(ch->status));
    cJSON_AddItemToObject(charData, "pk_enabled", cJSON_CreateNumber(ch->pk_enabled));
    cJSON_AddItemToObject(charData, "remort_level", cJSON_CreateNumber(ch->remortlevel));
    cJSON_AddItemToObject(charData, "immune", cJSON_CreateNumber(ch->immune));
    cJSON_AddItemToObject(charData, "polyaff", cJSON_CreateNumber(ch->polyaff));
    cJSON_AddItemToObject(charData, "itemaffect", cJSON_CreateNumber(ch->itemaffect));
    cJSON_AddItemToObject(charData, "vampaff", cJSON_CreateNumber(ch->vampaff));
    cJSON_AddItemToObject(charData, "vamppass", cJSON_CreateNumber(ch->vamppass));
    cJSON_AddItemToObject(charData, "form", cJSON_CreateNumber(ch->form));
    cJSON_AddItemToObject(charData, "beast", cJSON_CreateNumber(ch->beast));
    cJSON_AddItemToObject(charData, "vampgen", cJSON_CreateNumber(ch->vampgen));
    cJSON_AddItemToObject(charData, "spectype", cJSON_CreateNumber(ch->spectype));
    cJSON_AddItemToObject(charData, "specpower", cJSON_CreateNumber(ch->specpower));
    cJSON_AddItemToObject(charData, "home", cJSON_CreateNumber(ch->home));
    cJSON_AddItemToObject(charData, "level", cJSON_CreateNumber(ch->level));
    cJSON_AddItemToObject(charData, "trust", cJSON_CreateNumber(ch->trust));
    cJSON_AddItemToObject(charData, "played", cJSON_CreateNumber( ch->played + (int)(current_time - ch->logon) ));
    cJSON_AddItemToObject(charData, "room", cJSON_CreateNumber( (ch->in_room == get_room_index(ROOM_VNUM_LIMBO) && ch->was_in_room != NULL) ? ch->was_in_room->vnum : ch->in_room->vnum ) );

    pk = cJSON_CreateObject();

	cJSON_AddItemToObject(charData, "pk", pk);
    cJSON_AddItemToObject(pk, "enabled", cJSON_CreateNumber(ch->pk_enabled));
    cJSON_AddItemToObject(pk, "kills", cJSON_CreateNumber(ch->pkill));
    cJSON_AddItemToObject(pk, "death", cJSON_CreateNumber(ch->pdeath));

    cJSON_AddItemToObject(charData, "mob_deaths", cJSON_CreateNumber(ch->mkill));
    cJSON_AddItemToObject(charData, "mob_kills", cJSON_CreateNumber(ch->mdeath));

    weapons = cJSON_CreateObject();

    cJSON_AddItemToObject(charData, "weapons", weapons);
    cJSON_AddItemToObject(weapons, "hit", cJSON_CreateNumber(ch->wpn[WEAPON_HIT]));
    cJSON_AddItemToObject(weapons, "slice", cJSON_CreateNumber(ch->wpn[WEAPON_SLICE]));
    cJSON_AddItemToObject(weapons, "stab", cJSON_CreateNumber(ch->wpn[WEAPON_STAB]));
    cJSON_AddItemToObject(weapons, "slash", cJSON_CreateNumber(ch->wpn[WEAPON_SLASH]));
    cJSON_AddItemToObject(weapons, "whip", cJSON_CreateNumber(ch->wpn[WEAPON_WHIP]));
    cJSON_AddItemToObject(weapons, "claw", cJSON_CreateNumber(ch->wpn[WEAPON_CLAW]));
    cJSON_AddItemToObject(weapons, "blast", cJSON_CreateNumber(ch->wpn[WEAPON_BLAST]));
    cJSON_AddItemToObject(weapons, "pound", cJSON_CreateNumber(ch->wpn[WEAPON_POUND]));
    cJSON_AddItemToObject(weapons, "crush", cJSON_CreateNumber(ch->wpn[WEAPON_CRUSH]));
    cJSON_AddItemToObject(weapons, "bite", cJSON_CreateNumber(ch->wpn[WEAPON_BITE]));
    cJSON_AddItemToObject(weapons, "grep", cJSON_CreateNumber(ch->wpn[WEAPON_GREP]));
    cJSON_AddItemToObject(weapons, "pierce", cJSON_CreateNumber(ch->wpn[WEAPON_PIERCE]));
    cJSON_AddItemToObject(weapons, "suck", cJSON_CreateNumber(ch->wpn[WEAPON_SUCK]));

    spells = cJSON_CreateArray();

	cJSON_AddItemToObject(charData, "spells", spells);

	blue = cJSON_CreateObject();
	cJSON_AddItemToArray(spells, blue);
	cJSON_AddItemToObject(blue, "color", cJSON_CreateString("blue"));
	cJSON_AddItemToObject(blue, "level", cJSON_CreateNumber(ch->spl[SPELL_BLUE]));
	cJSON_AddItemToObject(blue, "tier", cJSON_CreateNumber(ch->tier_spl[SPELL_BLUE]));

	green = cJSON_CreateObject();
	cJSON_AddItemToArray(spells, green);
	cJSON_AddItemToObject(green, "color", cJSON_CreateString("green"));
	cJSON_AddItemToObject(green, "level", cJSON_CreateNumber(ch->spl[SPELL_GREEN]));
	cJSON_AddItemToObject(green, "tier", cJSON_CreateNumber(ch->tier_spl[SPELL_GREEN]));

	purple = cJSON_CreateObject();
	cJSON_AddItemToArray(spells, purple);
	cJSON_AddItemToObject(purple, "color", cJSON_CreateString("purple"));
	cJSON_AddItemToObject(purple, "level", cJSON_CreateNumber(ch->spl[SPELL_PURPLE]));
	cJSON_AddItemToObject(purple, "tier", cJSON_CreateNumber(ch->tier_spl[SPELL_PURPLE]));

	red = cJSON_CreateObject();
	cJSON_AddItemToArray(spells, red);
	cJSON_AddItemToObject(red, "color", cJSON_CreateString("red"));
	cJSON_AddItemToObject(red, "level", cJSON_CreateNumber(ch->spl[SPELL_RED]));
	cJSON_AddItemToObject(red, "tier", cJSON_CreateNumber(ch->tier_spl[SPELL_RED]));

	yellow = cJSON_CreateObject();
	cJSON_AddItemToArray(spells, yellow);
	cJSON_AddItemToObject(yellow, "color", cJSON_CreateString("yellow"));
	cJSON_AddItemToObject(yellow, "level", cJSON_CreateNumber(ch->spl[SPELL_YELLOW]));
	cJSON_AddItemToObject(yellow, "tier", cJSON_CreateNumber(ch->tier_spl[SPELL_YELLOW]));

	stances = cJSON_CreateObject();
	cJSON_AddItemToObject(areaData, "stances", stances);

	for( iHash = 0; iHash < MAX_STANCE; iHash++ )
	{
		if( iHash == 0)
			cJSON_AddItemToObject(stances, "autodrop", cJSON_CreateNumber(ch->stance[CURRENT_STANCE]));

		if( iHash == MAX_STANCE)
			cJSON_AddItemToObject(stances, "autodrop", cJSON_CreateNumber(ch->stance[AUTODROP]));
	}

	/*
    fprintf(fp, "Combat       %d %d %d %d %d %d %d %d\n",
    		ch->cmbt[0], ch->cmbt[1], ch->cmbt[2], ch->cmbt[3],
    		ch->cmbt[4], ch->cmbt[5], ch->cmbt[6], ch->cmbt[7]);
    fprintf(fp, "Stance       %d %d %d %d %d %d %d %d %d %d %d %d\n",
    		ch->stance[CURRENT_STANCE], ch->stance[STANCE_VIPER], ch->stance[STANCE_CRANE], ch->stance[STANCE_FALCON],
    		ch->stance[STANCE_MONGOOSE], ch->stance[STANCE_BULL], ch->stance[STANCE_SWALLOW], ch->stance[STANCE_COBRA],
    		ch->stance[STANCE_LION], ch->stance[STANCE_GRIZZLIE], ch->stance[STANCE_PANTHER], ch->stance[AUTODROP]);
    fprintf(fp, "Locationhp   %d %d %d %d %d %d %d\n",
    		ch->loc_hp[0], ch->loc_hp[1], ch->loc_hp[2], ch->loc_hp[3],
    		ch->loc_hp[4], ch->loc_hp[5], ch->loc_hp[6]);

    fprintf(fp, "HpManaMove   %d %d %d %d %d %d\n", ch->hit, ch->max_hit, ch->mana, ch->max_mana, ch->move, ch->max_move);


	/*
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
	*/


	snprintf(buf, MAX_INPUT_LENGTH, "%s", ch->name);
	strcat(buf, ".json");

	snprintf(tempbuf, MAX_INPUT_LENGTH, "Opening %s to save", buf);

	log_string(tempbuf);

	//charFile = fopen(buf, "ab+");

	//fprintf(charFile, "%s", cJSON_Print(charData));
	log_string(cJSON_Print(charData));

	snprintf(tempbuf, MAX_INPUT_LENGTH, "%s saved", buf);
	log_string(tempbuf);
	//fclose(charFile);

	cJSON_Delete(charData);
}

/*
cJSON_AddItemToObject(charData, "Name", cJSON_CreateString(ch->name));
cJSON_AddItemToObject(charData, "Name", cJSON_CreateString(ch->name));
cJSON_AddItemToObject(charData, "Name", cJSON_CreateString(ch->name));
cJSON_AddItemToObject(charData, "Name", cJSON_CreateString(ch->name));

cJSON_AddItemToObject(charData, "sex", cJSON_CreateNumber(ch->sex));
cJSON_AddItemToObject(charData, "sex", cJSON_CreateNumber(ch->sex));
cJSON_AddItemToObject(charData, "sex", cJSON_CreateNumber(ch->sex));
cJSON_AddItemToObject(charData, "sex", cJSON_CreateNumber(ch->sex));
cJSON_AddItemToObject(charData, "sex", cJSON_CreateNumber(ch->sex));
cJSON_AddItemToObject(charData, "sex", cJSON_CreateNumber(ch->sex));
cJSON_AddItemToObject(charData, "sex", cJSON_CreateNumber(ch->sex));
cJSON_AddItemToObject(charData, "sex", cJSON_CreateNumber(ch->sex));







fprintf(fp, "TierClandiscs       %d %d %d %d %d %d %d %d %d %d %d %d\n",
		ch->tier_clandisc[0], ch->tier_clandisc[1], ch->tier_clandisc[2], ch->tier_clandisc[3],
		ch->tier_clandisc[4], ch->tier_clandisc[5], ch->tier_clandisc[6], ch->tier_clandisc[7],
		ch->tier_clandisc[8], ch->tier_clandisc[9], ch->tier_clandisc[10], ch->tier_clandisc[11]);

/ *
 * Prefacing with a zero, always, because that's the autostance
 * /
fprintf(fp, "TierStances       0 %d %d %d %d %d %d %d %d %d %d %d\n",
		ch->tier_stance[STANCE_VIPER], ch->tier_stance[STANCE_CRANE], ch->tier_stance[STANCE_FALCON],
		ch->tier_stance[STANCE_MONGOOSE], ch->tier_stance[STANCE_BULL], ch->tier_stance[STANCE_SWALLOW], ch->tier_stance[STANCE_COBRA],
		ch->tier_stance[STANCE_LION], ch->tier_stance[STANCE_GRIZZLIE], ch->tier_stance[STANCE_PANTHER], ch->tier_stance[AUTODROP]);

fprintf(fp, "TierSpells       %d %d %d %d %d\n",


fprintf(fp, "TierWeapons       %d %d %d %d %d %d %d %d %d %d %d %d %d\n",
		ch->tier_wpn[WEAPON_HIT], ch->tier_wpn[WEAPON_SLICE], ch->tier_wpn[WEAPON_STAB], ch->tier_wpn[WEAPON_SLASH], ch->tier_wpn[WEAPON_WHIP],
		ch->tier_wpn[WEAPON_CLAW], ch->tier_wpn[WEAPON_BLAST], ch->tier_wpn[WEAPON_POUND], ch->tier_wpn[WEAPON_CRUSH], ch->tier_wpn[WEAPON_BITE],
		ch->tier_wpn[WEAPON_GREP], ch->tier_wpn[WEAPON_PIERCE], ch->tier_wpn[WEAPON_SUCK]);

fprintf(fp, "Gold         %ld\n", ch->gold);
fprintf(fp, "Bank         %ld\n", ch->bank);
fprintf(fp, "Exp          %ld\n", ch->exp);
fprintf(fp, "TierPoints   %ld\n", ch->tierpoints);
fprintf(fp, "BloodPoints  %ld\n", ch->bloodpoints);
fprintf(fp, "Act          %ld\n", ch->act);
fprintf(fp, "Extra        %d\n",  ch->extra);
fprintf(fp, "AffectedBy   %ld\n", ch->affected_by);
fprintf(fp, "Position     %d\n",  ch->position == POS_FIGHTING ? POS_STANDING : ch->position);
fprintf(fp, "Primal       %d\n",  ch->primal);
fprintf(fp, "SavingThrow  %d\n",  ch->saving_throw);
fprintf(fp, "Alignment    %d\n",  ch->alignment);
fprintf(fp, "Hitroll      %d\n",  ch->hitroll);
fprintf(fp, "Damroll      %d\n",  ch->damroll);
fprintf(fp, "Dodge        %d\n",  ch->dodge);
fprintf(fp, "Parry        %d\n",  ch->parry);
fprintf(fp, "Block        %d\n",  ch->block);
fprintf(fp, "Armor        %d\n",  ch->armor);
fprintf(fp, "Wimpy        %d\n",  ch->wimpy);
fprintf(fp, "Deaf         %ld\n", ch->deaf);
fprintf(fp, "LagPenalty   %d\n",  ch->lagpenalty);

if (IS_NPC(ch))
{
	fprintf(fp, "Vnum         %ld\n", ch->pIndexData->vnum);
}
else
{
	fprintf(fp, "Password     %s~\n", ch->pcdata->pwd);
	fprintf(fp, "Email        %s~\n", ch->pcdata->email);
	fprintf(fp, "Bamfin       %s~\n", ch->pcdata->bamfin);
	fprintf(fp, "Bamfout      %s~\n", ch->pcdata->bamfout);
	fprintf(fp, "Title        %s~\n", ch->pcdata->title);
	fprintf(fp, "AttrPerm     %d %d %d %d %d\n",
			ch->pcdata->perm_str,
			ch->pcdata->perm_int,
			ch->pcdata->perm_wis,
			ch->pcdata->perm_dex,
			ch->pcdata->perm_con);

	fprintf(fp, "AttrMod      %d %d %d %d %d\n",
			ch->pcdata->mod_str,
			ch->pcdata->mod_int,
			ch->pcdata->mod_wis,
			ch->pcdata->mod_dex,
			ch->pcdata->mod_con);

	fprintf(fp, "Quest        %d\n", ch->pcdata->quest);

	if (ch->pcdata->obj_vnum != 0)
		fprintf(fp, "Objvnum      %d\n", ch->pcdata->obj_vnum);

	fprintf(fp, "Condition    %d %d %d\n",
			ch->pcdata->condition[0],
			ch->pcdata->condition[1],
			ch->pcdata->condition[2]);

	for (sn = 0; sn < MAX_SKILL; sn++)
	{
		if (skill_table[sn].name != NULL && ch->pcdata->learned[sn] > 0)
		{
			fprintf(fp, "Skill        %d '%s'\n",
					ch->pcdata->learned[sn], skill_table[sn].name);
		}
	}
}
*/