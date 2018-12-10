#include <stdio.h>
#include <stdlib.h>
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

void load_area_file_json(char *areaFile)
{
    FILE *area;
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

    if ((area = fopen(areaFile, "r")) == NULL)
	{
		perror(areaFile);
		exit(1);
	}

    fseek(area, 0, SEEK_END);
    long fsize = ftell(area);
    fseek(area, 0, SEEK_SET);

    char *data = malloc(fsize + 1);
    fread(data, fsize, 1, area);
    fclose(area);

    data[fsize] = 0;

    cJSON *j_area = cJSON_Parse(data);

    if(j_area == NULL)
    {
        const char *error_ptr = cJSON_GetErrorPtr();
        if(error_ptr != NULL)
        {
            fprintf(stderr, "Error before: %s\n", error_ptr);
        }
        cJSON_Delete(j_area);
        exit(1);
    }

    // Load area
    pArea = alloc_perm(sizeof(*pArea));
    pArea->reset_first = NULL;
	pArea->reset_last = NULL;
    pArea->name = cJSON_GetObjectItemCaseSensitive(j_area, "name")->valuestring;
    pArea->creator = cJSON_GetObjectItemCaseSensitive(j_area, "creator")->valuestring;
    pArea->wasModified = FALSE;
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

    cJSON_ArrayForEach(mobile, mobiles)
    {
        int iHash;
        sh_int vnum;
        pMobIndex = alloc_perm(sizeof(*pMobIndex));
        vnum = cJSON_GetObjectItemCaseSensitive(mobile, "vnum")->valuedouble;
        pMobIndex->vnum = vnum;
        pMobIndex->name = cJSON_GetObjectItemCaseSensitive(mobile, "name")->valuestring;
        pMobIndex->short_descr = cJSON_GetObjectItemCaseSensitive(mobile, "short_description")->valuestring;
        pMobIndex->long_descr = cJSON_GetObjectItemCaseSensitive(mobile, "long_description")->valuestring;
        pMobIndex->desc = cJSON_GetObjectItemCaseSensitive(mobile, "description")->valuestring;
        pMobIndex->act = cJSON_GetObjectItemCaseSensitive(mobile, "act")->valuedouble;
        pMobIndex->affected_by = cJSON_GetObjectItemCaseSensitive(mobile, "affected_by")->valuedouble;
        pMobIndex->alignment = cJSON_GetObjectItemCaseSensitive(mobile, "alignment")->valuedouble;
        pMobIndex->level = cJSON_GetObjectItemCaseSensitive(mobile, "level")->valuedouble;
        pMobIndex->hitroll = cJSON_GetObjectItemCaseSensitive(mobile, "hitroll")->valuedouble;
        pMobIndex->damroll = cJSON_GetObjectItemCaseSensitive(mobile, "damroll")->valuedouble;
        pMobIndex->ac = cJSON_GetObjectItemCaseSensitive(mobile, "ac")->valuedouble;
        pMobIndex->hitplus = cJSON_GetObjectItemCaseSensitive(mobile, "hp")->valuedouble;
        pMobIndex->gold = cJSON_GetObjectItemCaseSensitive(mobile, "gold")->valuedouble;
        pMobIndex->sex = cJSON_GetObjectItemCaseSensitive(mobile, "sex")->valuedouble;
        pMobIndex->area = pArea;

        iHash = vnum % MAX_KEY_HASH;
		pMobIndex->next = mob_index_hash[iHash];
		mob_index_hash[iHash] = pMobIndex;
		area->mobiles++;
        top_mob_index++;
    }

    objects = cJSON_GetObjectItemCaseSensitive(j_area, "objects");

    cJSON_ArrayForEach(object, objects)
    {
        int iHash;
        sh_int vnum;
        long extraflags = 0;
        long wear_flags = 0;
        OBJ_INDEX_DATA *pObjIndex = alloc_perm(sizeof(*pObjIndex));
        vnum = cJSON_GetObjectItemCaseSensitive(object, "vnum")->valuedouble;
        pObjIndex->vnum = vnum;
        pObjIndex->name = cJSON_GetObjectItemCaseSensitive(object, "name")->valuestring;
        pObjIndex->short_descr = cJSON_GetObjectItemCaseSensitive(object, "short_description")->valuestring;
        pObjIndex->long_descr = cJSON_GetObjectItemCaseSensitive(object, "long_description")->valuestring;
        pObjIndex->item_type = cJSON_GetObjectItemCaseSensitive(object, "item_type")->valuedouble;
        pObjIndex->area = pArea;

        numbers = cJSON_GetObjectItemCaseSensitive(object, "extra_flags");
        cJSON_ArrayForEach(number, numbers)
        {
            extraflags += number->valuedouble;
        }
        number = NULL;
        numbers = NULL;
        pObjIndex->extra_flags = extraflags;

        numbers = cJSON_GetObjectItemCaseSensitive(object, "wear_flags");
        cJSON_ArrayForEach(number, numbers)
        {
            wear_flags += number->valuedouble;
        }
        pObjIndex->wear_flags = wearflags;
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

    cJSON_ArrayForEach(room, rooms)
    {
        int iHash;
        long roomflags = 0;
        sh_int vnum;
        pRoomIndex = alloc_perm(sizeof(*pRoomIndex));

        pRoomIndex->people = NULL;
		pRoomIndex->contents = NULL;
		pRoomIndex->extra_descr = NULL;
		pRoomIndex->area = pArea;
        vnum = cJSON_GetObjectItemCaseSensitive(room, "vnum")->valuedouble;
		pRoomIndex->vnum = vnum;
        pRoomIndex->name = cJSON_GetObjectItemCaseSensitive(room, "name")->valuestring;
        pRoomIndex->short_descr = cJSON_GetObjectItemCaseSensitive(object, "description")->valuestring;
        pRoomIndex->area_number = cJSON_GetObjectItemCaseSensitive(object, "description")->valuedouble;

        number = NULL;
        numbers = NULL;

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

    cJSON_ArrayForEach(reset, resets)
    {
        pReset = alloc_perm(sizeof(*pReset));
        pReset->command = cJSON_GetObjectItemCaseSensitive(reset, "command")->valuestring;
        pReset->description = cJSON_GetObjectItemCaseSensitive(reset, "description")->valuestring;
        pReset->arg1 = cJSON_GetObjectItemCaseSensitive(reset, "arg1")->valuedouble;
        pReset->arg2 = cJSON_GetObjectItemCaseSensitive(reset, "arg2")->valuedouble;
        pReset->arg3 = cJSON_GetObjectItemCaseSensitive(reset, "arg3")->valuedouble;

        if(pArea->reset_first == NULL)
		{
			pArea->reset_first = pReset;
		}

		if(pArea->reset_last != NULL)
		{
			pArea->reset_last->next = pReset;
		}

		pArea->reset_last = pReset;
		pArea->resets++;
		pReset->next = NULL;
		top_reset++;
    }    

    free_mem(data, sizeof(char*));
    return;
}