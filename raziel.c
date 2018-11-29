#if defined(macintosh)
#include <types.h>
#else
#include <sys/types.h>
#endif
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "merc.h"

void save_donroom args((int roomVnum));
void save_objects_to_file args((OBJ_DATA * objects, char *filename, int roomVnum));
void load_donroom args((int roomVnum));
void fwrite_obj args((CHAR_DATA * ch, OBJ_DATA *obj,
                      FILE *fp, int iNest));

void fread_room_obj args((FILE * fp, int roomVnum));
void load_donrooms args(());

#define MAX_NEST 100
static OBJ_DATA *rgObjNest[MAX_NEST];

#define KEYS(literal, field, value) \
    if (!str_cmp(word, literal))    \
    {                               \
        field = value;              \
        break;                      \
    }

void do_donate(CHAR_DATA *ch, char *argument)
{
    OBJ_DATA *obj;
    DESCRIPTOR_DATA *d;
    char arg[MAX_INPUT_LENGTH];
    ROOM_INDEX_DATA *donate;

    argument = one_argument(argument, arg, MAX_INPUT_LENGTH);

    if (arg[0] == '\0')
    {
        send_to_char("Donate what?\n\r", ch);
        return;
    }

    if (is_number(arg))
    {
        send_to_char("You cannot donate gold.\n\r", ch);
        return;
    }

    if (!(obj = get_obj_carry(ch, arg)))
    {
        send_to_char("You do not have that item.\n\r", ch);
        return;
    }

    if (!can_drop_obj(ch, obj))
    {
        send_to_char("You cannot let go of it!\n\r", ch);
        return;
    }

    if (obj->item_type == ITEM_BOMB)
    {
        send_to_char("You cannot donate BOMBS!! \n\r", ch);
        return;
    }

    obj_from_char(obj);

    // This stuff should realistically be a switch case or some type of dynamic array
    // The code repeat is way too fucking real....
    if (!str_cmp(ch->clan, "Brujah"))
    {
        obj_to_room(obj, donate = get_room_index(ROOM_VNUM_BRUJAH_DONROOM));
        save_donroom(ROOM_VNUM_BRUJAH_DONROOM);
    }
    else if (!str_cmp(ch->clan, "Gangrel"))
    {
        obj_to_room(obj, donate = get_room_index(ROOM_VNUM_GANGREL_DONROOM));
        save_donroom(ROOM_VNUM_GANGREL_DONROOM);
    }
    else if (!str_cmp(ch->clan, "Ventrue"))
    {
        obj_to_room(obj, donate = get_room_index(ROOM_VNUM_VENTRUE_DONROOM));
        save_donroom(ROOM_VNUM_VENTRUE_DONROOM);
    }
    else if (!str_cmp(ch->clan, "Malkavian"))
    {
        obj_to_room(obj, donate = get_room_index(ROOM_VNUM_MALKAVIAN_DONROOM));
        save_donroom(ROOM_VNUM_MALKAVIAN_DONROOM);
    }
    else if (!str_cmp(ch->clan, "Tremere"))
    {
        obj_to_room(obj, donate = get_room_index(ROOM_VNUM_TREMERE_DONROOM));
        save_donroom(ROOM_VNUM_TREMERE_DONROOM);
    }
    else if (!str_cmp(ch->clan, "Nosferatu"))
    {
        obj_to_room(obj, donate = get_room_index(ROOM_VNUM_NOSFERATU_DONROOM));
        save_donroom(ROOM_VNUM_NOSFERATU_DONROOM);
    }
    else if (!str_cmp(ch->clan, "Toreador"))
    {
        obj_to_room(obj, donate = get_room_index(ROOM_VNUM_TOREADOR_DONROOM));
        save_donroom(ROOM_VNUM_TOREADOR_DONROOM);
    }
    else if (!str_cmp(ch->clan, "Cappadocian"))
    {
        obj_to_room(obj, donate = get_room_index(ROOM_VNUM_CAPPADOCIAN_DONROOM));
        save_donroom(ROOM_VNUM_CAPPADOCIAN_DONROOM);
    }
    else
    {
        obj_to_room(obj, donate = get_room_index(ROOM_VNUM_DONATION_ROOM));
        save_donroom(ROOM_VNUM_DONATION_ROOM);
    }
    act("You donate $p.", ch, obj, NULL, TO_CHAR);
    act("$n donates $p.", ch, obj, NULL, TO_ROOM);

    for (d = descriptor_list; d; d = d->next)
    {
        if (d->connected == CON_PLAYING && d->character->in_room == donate && d->character->position > POS_SLEEPING)
        {
            snprintf(arg, MAX_INPUT_LENGTH, "%s fades into view on the floor.\n\r", obj->short_descr);
            send_to_char(arg, d->character);
        }
    }
    return;
}

void save_donrooms(void)
{
    save_donroom(ROOM_VNUM_BRUJAH_DONROOM);
    save_donroom(ROOM_VNUM_GANGREL_DONROOM);
    save_donroom(ROOM_VNUM_VENTRUE_DONROOM);
    save_donroom(ROOM_VNUM_MALKAVIAN_DONROOM);
    save_donroom(ROOM_VNUM_TREMERE_DONROOM);
    save_donroom(ROOM_VNUM_NOSFERATU_DONROOM);
    save_donroom(ROOM_VNUM_TOREADOR_DONROOM);
    save_donroom(ROOM_VNUM_CAPPADOCIAN_DONROOM);
    save_donroom(ROOM_VNUM_DONATION_ROOM);
}

void save_donroom(int roomVnum)
{
    int MaxItems = 100;
    ROOM_INDEX_DATA *location;
    OBJ_DATA *obj;
    OBJ_DATA *objectsToSave = NULL;
    OBJ_DATA *prevObject;
    int Items[MaxItems];
    int CurrentItem = 0;
    char Filename[MAX_STRING_LENGTH];

    location = get_room_index(roomVnum);

    if (location == NULL)
    {
        // We should basically never be able to get here, but check anyhow
        bug("Could not find room %d", roomVnum);
        return;
    }

    // Iterate through the entire room and get all objects
    for (obj = location->contents; obj; obj = obj->next_content)
    {
        if (CurrentItem == MaxItems + 1)
        {
            // We have hit the maximum in a donroom
            break;
        }

        bool duplicateItem = FALSE;
        // Iterate through all of the current vnums and see if we have a duplicate object vnum
        // This could be built smarter to look at VERY duplicate objects (Objects that have the same VNUM but different stats,
        // like quested items or anything of that sort
        for (int i = 0; i < (sizeof(Items) / sizeof(Items[0])); i++)
        {
            if (obj->pIndexData->vnum == Items[i])
            {
                // We don't want your dirty item
                duplicateItem = TRUE;
                continue;
            }
        }

        if (duplicateItem)
        {
            // Skip the item
            continue;
        }
        else
        {
            if (CurrentItem == 0)
            {
                objectsToSave = obj;
                prevObject = obj;
            }
            else
            {
                prevObject->next_content = obj;
                prevObject = obj;
            }
            Items[CurrentItem] = obj->pIndexData->vnum;
            CurrentItem++;
        }
    }

    // Only bother with this code path if we have items to save
    if (CurrentItem != 0 && objectsToSave != NULL)
    {
        switch (roomVnum)
        {
        case ROOM_VNUM_BRUJAH_DONROOM:
            snprintf(Filename, MAX_INPUT_LENGTH, "../donrooms/brujah.txt");
            break;
        case ROOM_VNUM_GANGREL_DONROOM:
            snprintf(Filename, MAX_INPUT_LENGTH, "../donrooms/gangrel.txt");
            break;
        case ROOM_VNUM_VENTRUE_DONROOM:
            snprintf(Filename, MAX_INPUT_LENGTH, "../donrooms/ventrue.txt");
            break;
        case ROOM_VNUM_MALKAVIAN_DONROOM:
            snprintf(Filename, MAX_INPUT_LENGTH, "../donrooms/malkavian.txt");
            break;
        case ROOM_VNUM_TREMERE_DONROOM:
            snprintf(Filename, MAX_INPUT_LENGTH, "../donrooms/tremere.txt");
            break;
        case ROOM_VNUM_NOSFERATU_DONROOM:
            snprintf(Filename, MAX_INPUT_LENGTH, "../donrooms/nosferatu.txt");
            break;
        case ROOM_VNUM_TOREADOR_DONROOM:
            snprintf(Filename, MAX_INPUT_LENGTH, "../donrooms/toreador.txt");
            break;
        case ROOM_VNUM_CAPPADOCIAN_DONROOM:
            snprintf(Filename, MAX_INPUT_LENGTH, "../donrooms/cappadocian.txt");
            break;
        case ROOM_VNUM_DONATION_ROOM:
            snprintf(Filename, MAX_INPUT_LENGTH, "../donrooms/default.txt");
            break;
        default:
            // I guess we are saving the vnum then
            snprintf(Filename, MAX_INPUT_LENGTH, "%d", roomVnum);
            break;
        }

        // This name is terrible and realistically should go into some OLC stuff, but this works for now
        save_objects_to_file(objectsToSave, Filename, roomVnum);
    }

    return;
}

void save_objects_to_file(OBJ_DATA *objects, char *filename, int roomVnum)
{
    char temp[MAX_INPUT_LENGTH];
    char strsave[MAX_INPUT_LENGTH];
    FILE *fp;

    // Always need to close this fucking reserve and re-open it, it's insane
    fclose(fpReserve);

    // Create a temporary copy, saves to the area folder
    snprintf(temp, MAX_INPUT_LENGTH, "%s", filename);
    snprintf(strsave, MAX_INPUT_LENGTH, "tmp.file");

    if ((fp = fopen(strsave, "w")) == NULL)
    {
        bug("save_objects_to_file: fopen", 0);
        perror(strsave);
    }
    else
    {
        fprintf(fp, "%d\n", roomVnum);
        // Write the objects, this runs recursively --  yay for base code writing
        fwrite_obj(NULL, objects, fp, 0);
        fprintf(fp, "#END\n");
    }

    fclose(fp);

    // Replace the temporary
    fpReserve = fopen(NULL_FILE, "r");
    rename(strsave, temp);

    return;
}

void load_donrooms(void)
{
    load_donroom(ROOM_VNUM_BRUJAH_DONROOM);
    load_donroom(ROOM_VNUM_GANGREL_DONROOM);
    load_donroom(ROOM_VNUM_VENTRUE_DONROOM);
    load_donroom(ROOM_VNUM_MALKAVIAN_DONROOM);
    load_donroom(ROOM_VNUM_TREMERE_DONROOM);
    load_donroom(ROOM_VNUM_NOSFERATU_DONROOM);
    load_donroom(ROOM_VNUM_TOREADOR_DONROOM);
    load_donroom(ROOM_VNUM_CAPPADOCIAN_DONROOM);
    load_donroom(ROOM_VNUM_DONATION_ROOM);
}

void load_donroom(int roomVnum)
{
    OBJ_DATA *objects;
    char Filename[MAX_STRING_LENGTH];
    FILE *fp;

    // Always need to close this fucking reserve and re-open it, it's insane
    fclose(fpReserve);

    switch (roomVnum)
    {
    case ROOM_VNUM_BRUJAH_DONROOM:
        snprintf(Filename, MAX_INPUT_LENGTH, "../donrooms/brujah.txt");
        break;
    case ROOM_VNUM_GANGREL_DONROOM:
        snprintf(Filename, MAX_INPUT_LENGTH, "../donrooms/gangrel.txt");
        break;
    case ROOM_VNUM_VENTRUE_DONROOM:
        snprintf(Filename, MAX_INPUT_LENGTH, "../donrooms/ventrue.txt");
        break;
    case ROOM_VNUM_MALKAVIAN_DONROOM:
        snprintf(Filename, MAX_INPUT_LENGTH, "../donrooms/malkavian.txt");
        break;
    case ROOM_VNUM_TREMERE_DONROOM:
        snprintf(Filename, MAX_INPUT_LENGTH, "../donrooms/tremere.txt");
        break;
    case ROOM_VNUM_NOSFERATU_DONROOM:
        snprintf(Filename, MAX_INPUT_LENGTH, "../donrooms/nosferatu.txt");
        break;
    case ROOM_VNUM_TOREADOR_DONROOM:
        snprintf(Filename, MAX_INPUT_LENGTH, "../donrooms/toreador.txt");
        break;
    case ROOM_VNUM_CAPPADOCIAN_DONROOM:
        snprintf(Filename, MAX_INPUT_LENGTH, "../donrooms/cappadocian.txt");
        break;
    case ROOM_VNUM_DONATION_ROOM:
        snprintf(Filename, MAX_INPUT_LENGTH, "../donrooms/default.txt");
        break;
    default:
        // I guess we are saving the vnum then
        snprintf(Filename, MAX_INPUT_LENGTH, "%d", roomVnum);
        break;
    }

    if ((fp = fopen(Filename, "r")) != NULL)
    {
        objects = alloc_perm(sizeof(*objects));

        roomVnum = fread_number(fp, -999);

        for (;;)
        {
            char letter;
            char *word;

            letter = fread_letter(fp);
            if (letter != '#')
            {
                bug("Load_donroom: # not found.", 0);
                break;
            }
            word = fread_word(fp);
            if (!str_cmp(word, "OBJECT"))
                fread_room_obj(fp, roomVnum);
            else if (!str_cmp(word, "END"))
                break;
            else
            {
                bug("Load_donroom: bad section.", 0);
                break;
            }
        }

        fclose(fp);
    }
    fpReserve = fopen(NULL_FILE, "r");
    return;
}

void fread_room_obj(FILE *fp, int roomVnum)
{
    static OBJ_DATA obj_zero;
    OBJ_DATA *obj;
    char *word;
    int iNest;
    bool fNest;
    bool fVnum;
    char errormess[MAX_STRING_LENGTH];

    if (obj_free == NULL)
    {
        obj = alloc_perm(sizeof(*obj));
    }
    else
    {
        obj = obj_free;
        obj_free = obj_free->next;
    }

    *obj = obj_zero;
    obj->name = str_dup("");
    obj->short_descr = str_dup("");
    obj->description = str_dup("");
    obj->chpoweron = str_dup("(null)");
    obj->chpoweroff = str_dup("(null)");
    obj->chpoweruse = str_dup("(null)");
    obj->victpoweron = str_dup("(null)");
    obj->victpoweroff = str_dup("(null)");
    obj->victpoweruse = str_dup("(null)");
    obj->questmaker = str_dup("");
    obj->questowner = str_dup("");
    obj->spectype = 0;
    obj->specpower = 0;
    obj->condition = 100;
    obj->toughness = 0;
    obj->resistance = 100;
    obj->quest = 0;
    obj->points = 0;

    fNest = FALSE;
    fVnum = TRUE;
    iNest = 0;

    for (;;)
    {
        word = feof(fp) ? "End" : fread_word(fp);

        switch (UPPER(word[0]))
        {
        case '*':
            fread_to_eol(fp);
            break;

        case 'A':
            if (!str_cmp(word, "Affect") || !str_cmp(word, "AffectData"))
            {
                AFFECT_DATA *paf;

                if (affect_free == NULL)
                {
                    paf = alloc_perm(sizeof(*paf));
                }
                else
                {
                    paf = affect_free;
                    affect_free = affect_free->next;
                }

                paf->duration = fread_number(fp, -999);
                paf->modifier = fread_number(fp, -999);
                paf->location = fread_number(fp, -999);
                paf->next = obj->affected;
                obj->affected = paf;
                break;
            }
            break;

        case 'C':
            if (!str_cmp(word, "Condition"))
            {
                obj->condition = fread_number(fp, -999);
                if (obj->condition == -999)
                {
                    snprintf(errormess, MAX_STRING_LENGTH, "Error in Condition \n\r");
                    obj->condition = 100;
                }
                break;
            }

            if (!str_cmp(word, "Cost"))
            {
                obj->cost = fread_number(fp, -999);
                if (obj->cost == -999)
                {

                    snprintf(errormess, MAX_STRING_LENGTH, "Error in Cost \n\r");
                    obj->cost = 0;
                }
                break;
            }
            break;

        case 'D':
            KEYS("Description", obj->description, fread_string(fp));
            break;

        case 'E':
            if (!str_cmp(word, "ExtraFlags"))
            {
                obj->extra_flags = fread_number(fp, -999);
                if (obj->extra_flags == -999)
                {

                    snprintf(errormess, MAX_STRING_LENGTH, "Error in ExtraFlags \n\r");
                    obj->extra_flags = 0;
                }
                break;
            }

            if (!str_cmp(word, "ExtraDescr"))
            {
                EXTRA_DESCR_DATA *ed;

                if (extra_descr_free == NULL)
                {
                    ed = alloc_perm(sizeof(*ed));
                }
                else
                {
                    ed = extra_descr_free;
                    extra_descr_free = extra_descr_free->next;
                }

                ed->keyword = fread_string(fp);
                ed->description = fread_string(fp);
                ed->next = obj->extra_descr;
                obj->extra_descr = ed;
            }

            if (!str_cmp(word, "End"))
            {
                if (!fNest || !fVnum)
                {
                    bug("Fread_obj: incomplete object.", 0);
                    free_string(obj->name);
                    free_string(obj->description);
                    free_string(obj->short_descr);
                    obj->next = obj_free;
                    obj_free = obj;
                    return;
                }
                else
                {
                    obj->next = object_list;
                    object_list = obj;
                    obj->pIndexData->count++;
                    if (iNest == 0 || rgObjNest[iNest] == NULL)
                    {
                        obj_to_room(obj, get_room_index(roomVnum));
                    }
                    else
                        obj_to_obj(obj, rgObjNest[iNest - 1]);
                    return;
                }
            }
            break;

        case 'I':
            if (!str_cmp(word, "ItemType"))
            {
                obj->item_type = fread_number(fp, -999);
                if (obj->item_type == -999)
                {

                    snprintf(errormess, MAX_STRING_LENGTH, "Error in ItemType \n\r");
                    obj->item_type = 0;
                }
                break;
            }
            break;

        case 'L':
            if (!str_cmp(word, "Level"))
            {
                obj->level = fread_number(fp, -999);
                if (obj->level == -999)
                {

                    snprintf(errormess, MAX_STRING_LENGTH, "Error in Level \n\r");
                    obj->level = 0;
                }
                break;
            }
            break;

        case 'N':
            KEYS("Name", obj->name, fread_string(fp));

            if (!str_cmp(word, "Nest"))
            {
                iNest = fread_number(fp, -999);
                if (iNest < 0 || iNest >= MAX_NEST)
                {
                    bug("Fread_obj: bad nest %d.", iNest);
                }
                else
                {
                    rgObjNest[iNest] = obj;
                    fNest = TRUE;
                }
            }
            break;

        case 'P':
            if (!str_cmp(word, "Points"))
            {
                obj->points = fread_number(fp, -999);
                if (obj->points == -999)
                {

                    snprintf(errormess, MAX_STRING_LENGTH, "Error in Points \n\r");
                    obj->points = 0;
                }
                break;
            }
            KEYS("Poweronch", obj->chpoweron, fread_string(fp));
            KEYS("Poweroffch", obj->chpoweroff, fread_string(fp));
            KEYS("Powerusech", obj->chpoweruse, fread_string(fp));
            KEYS("Poweronvict", obj->victpoweron, fread_string(fp));
            KEYS("Poweroffvict", obj->victpoweroff, fread_string(fp));
            KEYS("Powerusevict", obj->victpoweruse, fread_string(fp));
            break;

        case 'Q':
            if (!str_cmp(word, "Quest"))
            {
                obj->quest = fread_number(fp, -999);
                if (obj->quest == -999)
                {

                    snprintf(errormess, MAX_STRING_LENGTH, "Error in Quest \n\r");
                    obj->quest = 0;
                }
                break;
            }
            KEYS("Questmaker", obj->questmaker, fread_string(fp));
            KEYS("Questowner", obj->questowner, fread_string(fp));
            break;

        case 'R':
            if (!str_cmp(word, "Resistance"))
            {
                obj->resistance = fread_number(fp, -999);
                if (obj->resistance == -999)
                {

                    snprintf(errormess, MAX_STRING_LENGTH, "Error in Resistance \n\r");
                    obj->resistance = 0;
                }
                break;
            }
            break;

        case 'S':
            KEYS("ShortDescr", obj->short_descr, fread_string(fp));
            if (!str_cmp(word, "Spectype"))
            {
                obj->spectype = fread_number(fp, -999);
                if (obj->spectype == -999)
                {

                    snprintf(errormess, MAX_STRING_LENGTH, "Error in Spectype \n\r");
                    obj->spectype = 0;
                }
                break;
            }

            if (!str_cmp(word, "Specpower"))
            {
                obj->specpower = fread_number(fp, -999);
                if (obj->specpower == -999)
                {

                    snprintf(errormess, MAX_STRING_LENGTH, "Error in Specpower \n\r");
                    obj->specpower = 0;
                }
                break;
            }

            if (!str_cmp(word, "Spell"))
            {
                int iValue;
                int sn;

                iValue = fread_number(fp, -999);
                sn = skill_lookup(fread_word(fp));
                if (iValue < 0 || iValue > 3)
                {
                    bug("Fread_obj: bad iValue %d.", iValue);
                }
                else if (sn < 0)
                {
                    bug("Fread_obj: unknown skill.", 0);
                }
                else
                {
                    obj->value[iValue] = sn;
                }
                break;
            }
            break;

        case 'T':
            if (!str_cmp(word, "Timer"))
            {
                obj->timer = fread_number(fp, -999);
                if (obj->timer == -999)
                {

                    snprintf(errormess, MAX_STRING_LENGTH, "Error in Timer \n\r");
                    obj->timer = 0;
                }
                break;
            }

            if (!str_cmp(word, "Toughness"))
            {
                obj->toughness = fread_number(fp, -999);
                if (obj->toughness == -999)
                {

                    snprintf(errormess, MAX_STRING_LENGTH, "Error in Toughness \n\r");
                    obj->toughness = 0;
                }
                break;
            }
            break;

        case 'V':
            if (!str_cmp(word, "Values"))
            {
                obj->value[0] = 0;
                obj->value[1] = 0;
                obj->value[2] = 0;
                obj->value[3] = 0;
                obj->value[0] = fread_number(fp, -999);
                obj->value[1] = fread_number(fp, -999);
                obj->value[2] = fread_number(fp, -999);
                obj->value[3] = fread_number(fp, -999);
                break;
            }

            if (!str_cmp(word, "Vnum"))
            {
                int vnum;

                vnum = fread_number(fp, -999);
                if ((obj->pIndexData = get_obj_index(vnum)) == NULL)
                    bug("Fread_obj: bad vnum %d.", vnum);
                else
                    fVnum = TRUE;
                break;
            }
            break;

        case 'W':
            if (!str_cmp(word, "WearFlags"))
            {
                obj->wear_flags = fread_number(fp, -999);
                if (obj->wear_flags == -999)
                {

                    snprintf(errormess, MAX_STRING_LENGTH, "Error in WearFlags \n\r");
                    obj->wear_flags = 0;
                }
                break;
            }

            if (!str_cmp(word, "WearLoc"))
            {
                obj->wear_loc = fread_number(fp, -999);
                if (obj->wear_loc == -999)
                {

                    snprintf(errormess, MAX_STRING_LENGTH, "Error in WearLoc \n\r");
                    obj->wear_loc = 0;
                }
                break;
            }

            if (!str_cmp(word, "Weight"))
            {
                obj->weight = fread_number(fp, -999);
                if (obj->weight == -999)
                {

                    snprintf(errormess, MAX_STRING_LENGTH, "Error in Weight \n\r");
                    obj->weight = 0;
                }
                break;
            }
            break;
        }
    }
}

void do_aload(CHAR_DATA *ch, char *argument)
{
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];

    one_argument(argument, arg, MAX_INPUT_LENGTH);

    if(arg[0] == '\0')
    {
        send_to_char("Please specify an area file without the .are extension.\n\r", ch);
        return;
    }

    snprintf(buf, MAX_STRING_LENGTH, "%s%s.are", AREA_DIR, arg);

    load_area_file(buf);
    
    return;
}