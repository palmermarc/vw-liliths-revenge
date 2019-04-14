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

extern int top_area;
extern int top_mob_index;
extern int top_obj_index;
extern int top_room;
extern AREA_DATA *area_first;

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
    if (!str_cmp(ch->clan, "Assamite"))
    {
        obj_to_room(obj, donate = get_room_index(ROOM_VNUM_ASSAMITE_DONROOM));
        save_donroom(ROOM_VNUM_ASSAMITE_DONROOM);
    }
    else if (!str_cmp(ch->clan, "Lasombra"))
    {
        obj_to_room(obj, donate = get_room_index(ROOM_VNUM_LASOMBRA_DONROOM));
        save_donroom(ROOM_VNUM_LASOMBRA_DONROOM);
    }
    else if (!str_cmp(ch->clan, "Ventrue"))
    {
        obj_to_room(obj, donate = get_room_index(ROOM_VNUM_VENTRUE_DONROOM));
        save_donroom(ROOM_VNUM_VENTRUE_DONROOM);
    }
    else if (!str_cmp(ch->clan, "Tzimisce"))
    {
        obj_to_room(obj, donate = get_room_index(ROOM_VNUM_TZIMISCE_DONROOM));
        save_donroom(ROOM_VNUM_TZIMISCE_DONROOM);
    }
    else if (!str_cmp(ch->clan, "Tremere"))
    {
        obj_to_room(obj, donate = get_room_index(ROOM_VNUM_TREMERE_DONROOM));
        save_donroom(ROOM_VNUM_TREMERE_DONROOM);
    }
    else if (!str_cmp(ch->clan, "Toreador"))
    {
        obj_to_room(obj, donate = get_room_index(ROOM_VNUM_TOREADOR_DONROOM));
        save_donroom(ROOM_VNUM_TOREADOR_DONROOM);
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
    log_string("Saving donrooms");
    save_donroom(ROOM_VNUM_ASSAMITE_DONROOM);
    save_donroom(ROOM_VNUM_TZIMISCE_DONROOM);
    save_donroom(ROOM_VNUM_VENTRUE_DONROOM);
    save_donroom(ROOM_VNUM_TREMERE_DONROOM);
    save_donroom(ROOM_VNUM_TOREADOR_DONROOM);
    save_donroom(ROOM_VNUM_LASOMBRA_DONROOM);
    save_donroom(ROOM_VNUM_DONATION_ROOM);
    log_string("Donrooms saved");
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
        case ROOM_VNUM_ASSAMITE_DONROOM:
            snprintf(Filename, MAX_INPUT_LENGTH, "../donrooms/assamite.txt");
            break;
        case ROOM_VNUM_LASOMBRA_DONROOM:
            snprintf(Filename, MAX_INPUT_LENGTH, "../donrooms/lasombra.txt");
            break;
        case ROOM_VNUM_VENTRUE_DONROOM:
            snprintf(Filename, MAX_INPUT_LENGTH, "../donrooms/ventrue.txt");
            break;
        case ROOM_VNUM_TZIMISCE_DONROOM:
            snprintf(Filename, MAX_INPUT_LENGTH, "../donrooms/tzimisce.txt");
            break;
        case ROOM_VNUM_TREMERE_DONROOM:
            snprintf(Filename, MAX_INPUT_LENGTH, "../donrooms/tremere.txt");
            break;
        case ROOM_VNUM_TOREADOR_DONROOM:
            snprintf(Filename, MAX_INPUT_LENGTH, "../donrooms/toreador.txt");
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
    load_donroom(ROOM_VNUM_ASSAMITE_DONROOM);
    load_donroom(ROOM_VNUM_TZIMISCE_DONROOM);
    load_donroom(ROOM_VNUM_VENTRUE_DONROOM);
    load_donroom(ROOM_VNUM_TREMERE_DONROOM);
    load_donroom(ROOM_VNUM_TOREADOR_DONROOM);
    load_donroom(ROOM_VNUM_LASOMBRA_DONROOM);
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
    case ROOM_VNUM_ASSAMITE_DONROOM:
        snprintf(Filename, MAX_INPUT_LENGTH, "../donrooms/assamite.txt");
        break;
    case ROOM_VNUM_TZIMISCE_DONROOM:
        snprintf(Filename, MAX_INPUT_LENGTH, "../donrooms/tzimisce.txt");
        break;
    case ROOM_VNUM_VENTRUE_DONROOM:
        snprintf(Filename, MAX_INPUT_LENGTH, "../donrooms/ventrue.txt");
        break;
    case ROOM_VNUM_LASOMBRA_DONROOM:
        snprintf(Filename, MAX_INPUT_LENGTH, "../donrooms/lasombra.txt");
        break;
    case ROOM_VNUM_TREMERE_DONROOM:
        snprintf(Filename, MAX_INPUT_LENGTH, "../donrooms/tremere.txt");
        break;
    case ROOM_VNUM_TOREADOR_DONROOM:
        snprintf(Filename, MAX_INPUT_LENGTH, "../donrooms/toreador.txt");
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

    if (arg[0] == '\0')
    {
        send_to_char("Please specify an area file without the .are extension.\n\r", ch);
        return;
    }

    snprintf(buf, MAX_STRING_LENGTH, "%s%s.are", AREA_DIR, arg);

    load_area_file(buf);

    return;
}

void do_astat(CHAR_DATA *ch, char *argument)
{
    char arg[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];
    AREA_DATA *pArea;
    AREA_DATA *foundArea = NULL;

    one_argument(argument, arg, MAX_INPUT_LENGTH);

    if (!str_cmp(arg, "list"))
    {
        for (pArea = area_first; pArea != NULL; pArea = pArea->next)
        {
            snprintf(buf, MAX_STRING_LENGTH, "%-30s %s\n\r", pArea->file, pArea->name);
            send_to_char(buf, ch);
        }
        return;
    }

    if (arg[0] == '\0')
    {
        foundArea = ch->in_room->area;
    }
    else
    {
        for (pArea = area_first; pArea != NULL; pArea = pArea->next)
        {
            if (!str_cmp(arg, pArea->name))
            {
                foundArea = pArea;
                break;
            }
        }
    }

    if (foundArea == NULL)
    {
        send_to_char("No such area.\n\r", ch);
        return;
    }

    snprintf(buf, MAX_STRING_LENGTH, "Area: %s  Age: %d\n\r", foundArea->name, foundArea->resets);
    send_to_char(buf, ch);

    snprintf(buf, MAX_STRING_LENGTH, "Creator: %s  File: %s\n\r", foundArea->creator, foundArea->file);
    send_to_char(buf, ch);

    snprintf(buf, MAX_STRING_LENGTH, "Reset_first: %c %ld %ld %ld\n\r",
             foundArea->reset_first->command, foundArea->reset_first->arg1,
             foundArea->reset_first->arg2, foundArea->reset_first->arg3);
    send_to_char(buf, ch);

    snprintf(buf, MAX_STRING_LENGTH, "Reset_last:  %c %ld %ld %ld\n\r",
             foundArea->reset_last->command, foundArea->reset_last->arg1,
             foundArea->reset_last->arg2, foundArea->reset_last->arg3);
    send_to_char(buf, ch);

    snprintf(buf, MAX_STRING_LENGTH, "Players: %d  Mobiles: %d  Objects: %d\n\r", foundArea->nplayer, foundArea->mobiles, foundArea->objects);
    send_to_char(buf, ch);

    snprintf(buf, MAX_STRING_LENGTH, "Rooms: %d  Resets: %d  Shops: %d\n\r", foundArea->rooms, foundArea->resets, foundArea->shops);
    send_to_char(buf, ch);

    snprintf(buf, MAX_STRING_LENGTH, "Helps: %d  Specials: %d\n\r", foundArea->helps, foundArea->specials);
    send_to_char(buf, ch);

    return;
}

void do_hstat(CHAR_DATA *ch, char *argument)
{
    char argall[MAX_INPUT_LENGTH];
    char argone[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];
    char *line;
    int lineCount = 0;

    HELP_DATA *pHelp = NULL;

    if (argument[0] == '\0')
    {
        send_to_char("hstat which help?\n\r", ch);
        return;
    }

    argall[0] = '\0';
    while (argument[0] != '\0')
    {
        argument = one_argument(argument, argone, MAX_INPUT_LENGTH);
        if (argall[0] != '\0')
            strncat(argall, " ", MAX_INPUT_LENGTH - strlen(argall));
        strncat(argall, argone, MAX_INPUT_LENGTH - strlen(argall));
    }

    for (pHelp = help_first; pHelp != NULL; pHelp = pHelp->next)
    {

        if (is_name(argall, pHelp->keyword))
        {
            break;
            /*
		  * Strip leading '.' to allow initial blanks.
		  */
            if (pHelp->text[0] == '.')
                send_to_char_formatted(pHelp->text + 1, ch);
            else
                send_to_char_formatted(pHelp->text, ch);
            return;
        }
    }

    if (pHelp == NULL)
    {
        send_to_char("No help for that.\n\r", ch);
        return;
    }

    snprintf(buf, MAX_STRING_LENGTH, "Area: %s\n\rArea File: %s\n\rModified: %s\n\rLevel: %d\n\r", 
    pHelp->area->name, pHelp->area->file, (pHelp->area->wasModified == TRUE ? "True" : "False"), pHelp->level);
    send_to_char_formatted(buf, ch);

    snprintf(buf, MAX_STRING_LENGTH, "Keywords: %s\n\r", pHelp->keyword);
    send_to_char_formatted(buf, ch);

    line = pHelp->text;
    line[strlen(line)-1] = 0;
    while (line)
    {
        lineCount++;
        char *nextLine = strchr(line, '\r');

        if (nextLine)
        {
            *nextLine = '\0';
        }

        snprintf(buf, MAX_STRING_LENGTH, "%2d: %s", lineCount, line);
        send_to_char(buf, ch);

        if (nextLine)
        {
            *nextLine = '\r';
        }

        line = nextLine ? (nextLine + 1) : NULL;
    }

    send_to_char("\n\r", ch);

    /*
    snprintf(buf, MAX_STRING_LENGTH, "%s\n\r", (pHelp->text[0] == '.' ? pHelp->text + 1 : pHelp->text));
    send_to_char_formatted(buf, ch);
    */

    return;
}

void do_customcommand(CHAR_DATA *ch, char *argument)
{
    char buf[MAX_STRING_LENGTH];
    MOB_INDEX_DATA *pMobIndex;
    ROOM_INDEX_DATA *pRoomIndex;
    OBJ_INDEX_DATA *pObjIndex;
    int vnum;

    // mobiles
    for (vnum = 0; vnum < 50000; vnum++)
	{
		if ((pMobIndex = get_mob_index(vnum)) != NULL)
		{
            snprintf(buf, MAX_STRING_LENGTH, "[%5d] %-28s %s\n\r",
						 vnum, pMobIndex->area->file, capitalize(pMobIndex->short_descr));
		}
        else
        {
            snprintf(buf, MAX_STRING_LENGTH, "[%5d] %-28s %s\n\r",
						 vnum, "No Area", "None");
        }

        append_file(ch, "mobs.txt", buf);
	}

    // objects
    for (vnum = 0; vnum < 50000; vnum++)
    {
        if ((pObjIndex = get_obj_index(vnum)) != NULL)
		{
            snprintf(buf, MAX_STRING_LENGTH, "[%5d] %-28s %s\n\r",
						 vnum, pObjIndex->area->file, capitalize(pObjIndex->short_descr));
		}
        else
        {
            snprintf(buf, MAX_STRING_LENGTH, "[%5d] %-28s %s\n\r",
						 vnum, "No Area", "None");
        }
        
        //send_to_char_formatted(buf, ch);
        append_file(ch, "objects.txt", buf);
    }

    // rooms
    for (vnum = 0; vnum < 50000; vnum++)
    {
        if ((pRoomIndex = get_room_index(vnum)) != NULL)
		{
            snprintf(buf, MAX_STRING_LENGTH, "[%5d] %-28s %s\n\r",
						 vnum, pRoomIndex->area->file, capitalize(pRoomIndex->name));
		}
        else
        {
            snprintf(buf, MAX_STRING_LENGTH, "[%5d] %-28s %s\n\r",
						 vnum, "No Area", "None");
        }
        
        append_file(ch, "rooms.txt", buf);
    }
}

// This shit is breaky atm

void do_hedit(CHAR_DATA *ch, char *argument)
{
    char arg[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    char arg3[MAX_INPUT_LENGTH];
    char *leftover;
    char buf[MAX_STRING_LENGTH];
    char *text = alloc_mem(sizeof(buf));
    char *line;
    int lineCount = 0;

    HELP_DATA *pHelp = NULL;

    if (argument[0] == '\0')
    {
        send_to_char("hstat which help?\n\r", ch);
        return;
    }

    leftover = one_argument(argument, arg, MAX_INPUT_LENGTH);

    for (pHelp = help_first; pHelp != NULL; pHelp = pHelp->next)
    {

        if (is_name(arg, pHelp->keyword))
        {
            break;
        }
    }

    if (pHelp == NULL)
    {
        send_to_char("No help with that name found.\n\r", ch);
        return;
    }

    leftover = one_argument(leftover, arg2, MAX_INPUT_LENGTH);

    if (!str_cmp(arg2, "level"))
    {
        if (is_number(leftover))
        {
            pHelp->level = atoi(leftover);
        }
        else
        {
            send_to_char("Not a valid number.\n\r", ch);
            return;
        }
    }
    else if (!str_cmp(arg2, "keyword"))
    {
        pHelp->keyword = str_dup(leftover);
    }
    else if (!str_cmp(arg2, "line"))
    {
        leftover = one_argument(leftover, arg3, MAX_INPUT_LENGTH);
        if (is_number(arg3))
        {
            line = strtok(pHelp->text, "\r");

            while (line != NULL)
            {
                lineCount++;
                if (atoi(arg3) == lineCount)
                {
                    snprintf(buf, MAX_STRING_LENGTH, "%s\n\r", leftover);
                    strcat(text, buf);
                }
                else
                {
                    snprintf(buf, MAX_STRING_LENGTH, "%s\r", line);
                    strcat(text, buf);
                }

                line = strtok(NULL, "\r");
            }

            text[strlen(text)-1] = 0;

            log_string(text);

            pHelp->text = str_dup(text);
        }
        else
        {
            send_to_char("Not a valid line number.\n\r", ch);
            return;
        }
    }
    else
    {
        send_to_char("Valid arguments: keyword <text>, level <##> and line\n\r", ch);
        send_to_char("hedit <help name/keyword> <line> <##> <text>\n\r", ch);
        return;
    }

    pHelp->area->wasModified = TRUE;

    snprintf(buf, MAX_STRING_LENGTH, "Area: %s\n\rArea File: %s\n\rModified: %s\n\rLevel: %d\n\r", 
    pHelp->area->name, pHelp->area->file, (pHelp->area->wasModified == TRUE ? "True" : "False"), pHelp->level);
    send_to_char_formatted(buf, ch);

    snprintf(buf, MAX_STRING_LENGTH, "Keywords: %s\n\r", pHelp->keyword);
    send_to_char_formatted(buf, ch);

    lineCount = 0;

    line = pHelp->text;
    while (line)
    {
        lineCount++;
        char *nextLine = strchr(line, '\r');

        if (nextLine)
        {
            *nextLine = '\0';
        }

        snprintf(buf, MAX_STRING_LENGTH, "%2d: %s", lineCount, line);
        send_to_char(buf, ch);

        if (nextLine)
        {
            *nextLine = '\r';
        }

        line = nextLine ? (nextLine + 1) : NULL;
    }

    send_to_char("\n\r", ch);

    return;
}

void do_asave(CHAR_DATA *ch, char *argument)
{
    char arg[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];
    char *leftover;
    AREA_DATA *pArea;
    int modifiedAreas = 0;

    leftover = one_argument(argument, arg, MAX_INPUT_LENGTH);
    one_argument(leftover, arg2, MAX_INPUT_LENGTH);

    if (!str_cmp(arg, "list"))
    {
        send_to_char("Modified Areas:\n\r", ch);
        for (pArea = area_first; pArea != NULL; pArea = pArea->next)
        {
            if(!pArea->wasModified) continue;

            modifiedAreas++;
            snprintf(buf, MAX_STRING_LENGTH, "%s\n\r", pArea->name);
            send_to_char(buf, ch);
        }

        if(modifiedAreas == 0)
        {
            send_to_char("No modified areas.\n\r", ch);
        }
        return;
    }

    if(!str_cmp(arg, "new"))
    {
        for(pArea = area_first; pArea != NULL; pArea = pArea->next)
        {
            if(!str_cmp(pArea->name, arg2))
            {
                save_area_file_json(pArea);
                send_to_char("Area saved\n\r", ch);
                return;
            }
        }
    }

    if (!str_cmp(arg, "save"))
    {
        for (pArea = area_first; pArea != NULL; pArea = pArea->next)
        {
            if(!str_cmp(pArea->name, arg2))
            {
                if(pArea->wasModified)
                {
                    save_area_file_json(pArea);
                    send_to_char("Area Saved\n\r", ch);
                    return;
                }
                else
                {
                    send_to_char("Area has not been modified\n\r", ch);
                    return;
                }
            }
        }
    }

    send_to_char("Arguments: list, save\n\r", ch);
    return;
}

void do_cstat(CHAR_DATA *ch, char *argument)
{
	char buf[MAX_STRING_LENGTH];
	char arg[MAX_INPUT_LENGTH];
	CLANDISC_DATA *disc;
	CHAR_DATA *victim;

	one_argument(argument, arg, MAX_INPUT_LENGTH);

	if (arg[0] == '\0')
	{
		send_to_char("Cstat whom?\n\r", ch);
		return;
	}

	if ((victim = get_char_world(ch, arg)) == NULL)
	{
		send_to_char("They aren't here.\n\r", ch);
		return;
	}

	if (IS_SET(victim->act, PLR_GODLESS) && ch->level != LEVEL_CODER)
	{
		send_to_char("You failed.\n\r", ch);
		return;
	}

	if (ch->level < LEVEL_ORACLE && !IS_NPC(victim))
	{
		send_to_char("You can only cstat mobs!\n\r", ch);
		return;
	}

    for(disc = ch->clandisc; disc != NULL; disc = disc->next)
    {
        snprintf(buf, MAX_STRING_LENGTH, "Ability: %s  Disc: %s  Tier: %d  Cooldown: %d  Bloodcost: %d.\n\r", 
            disc->name, disc->clandisc, disc->tier, disc->cooldown, disc->bloodcost);

	    send_to_char(buf, ch);
    }

    return;
}

void do_cset(CHAR_DATA *ch, char *argument)
{
    char arg1[MAX_INPUT_LENGTH];
	char arg2[MAX_INPUT_LENGTH];
	char buf[MAX_INPUT_LENGTH];
	CHAR_DATA *victim;
    CLANDISC_DATA *disc;

	snprintf(buf, MAX_INPUT_LENGTH, "%s: Cset %s", ch->name, argument);
	if (ch->level < NO_WATCH)
		do_watching(ch, buf);

	smash_tilde(argument);
	argument = one_argument(argument, arg1, MAX_INPUT_LENGTH);
	argument = one_argument(argument, arg2, MAX_INPUT_LENGTH);

	if (arg1[0] == '\0' || arg2[0] == '\0')
	{
		send_to_char("Syntax: cset <victim> <ability>\n\r", ch);
		return;
	}

	if ((victim = get_char_world(ch, arg1)) == NULL)
	{
		send_to_char("They aren't here.\n\r", ch);
		return;
	}

    if( (disc = get_disc_by_name(arg2)) == NULL)
    {
        send_to_char("That isn't a valid clandisc ability\n\r", ch);
        return;
    }

	if (!IS_NPC(ch) && !IS_NPC(victim) && (ch->level <= victim->level) && ch != victim)
	{
		send_to_char("No way!! \n\r", ch);
		return;
	}

	if (IS_SET(victim->act, PLR_GODLESS) && ch->level != LEVEL_CODER)
	{
		send_to_char("You failed.\n\r", ch);
		return;
	}

    SetPlayerDisc(victim, disc);
    
    snprintf(buf, MAX_INPUT_LENGTH, "%s ability set.\n\r", disc->name);
    send_to_char(buf, ch);

    return;
}

void GiveNewbieGear(CHAR_DATA *ch, int option)
{
    OBJ_INDEX_DATA *pObjIndex;
    OBJ_DATA *obj;
    int i;

    long equippedNewbieItems[17] = { STARTING_NEWBIE_ARMGUARDS, STARTING_NEWBIE_BOOTS, STARTING_NEWBIE_BRACER_1,
    STARTING_NEWBIE_BRACER_2, STARTING_NEWBIE_CHESTPLATE, STARTING_NEWBIE_CLOAK, STARTING_NEWBIE_GIRTH, STARTING_NEWBIE_GLOVES,
    STARTING_NEWBIE_HELMET, STARTING_NEWBIE_LEGGINGS, STARTING_NEWBIE_MASK, STARTING_NEWBIE_NECK_1,
    STARTING_NEWBIE_NECK_2, STARTING_NEWBIE_RING_1, STARTING_NEWBIE_RING_2, STARTING_NEWBIE_BAG, STARTING_NEWBIE_LIGHT };

    for(i = 0; i < 17; i++)
    {
        pObjIndex = NULL;
        obj = NULL;

        if ((pObjIndex = get_obj_index(equippedNewbieItems[i])) != NULL)
        {
            obj = create_object(pObjIndex, 25);
            obj->questowner = str_dup(ch->name);
            obj_to_char(obj, ch);

            if(equippedNewbieItems[i] != STARTING_NEWBIE_BAG && equippedNewbieItems[i] != STARTING_NEWBIE_LIGHT)
            {
                wear_obj(ch, obj, TRUE);
            }
        }
    }

    // These could be done with Switch/Case, just didn't feel like it

    if(option == STARTING_OPTION_SWORD_BOARD)
    {
        if ((pObjIndex = get_obj_index(STARTING_NEWBIE_SWORD_1H)) != NULL)
        {
            obj = create_object(pObjIndex, 25);
            obj->questowner = str_dup(ch->name);
            obj_to_char(obj, ch);
            wear_obj(ch, obj, TRUE);
        }

        if ((pObjIndex = get_obj_index(STARTING_NEWBIE_SHIELD)) != NULL)
        {
            obj = create_object(pObjIndex, 25);
            obj->questowner = str_dup(ch->name);
            obj_to_char(obj, ch);
            wear_obj(ch, obj, TRUE);
        }
    }

    if(option == STARTING_OPTION_DUAL_WIELD)
    {
        if ((pObjIndex = get_obj_index(STARTING_NEWBIE_SWORD_1H)) != NULL)
        {
            obj = create_object(pObjIndex, 25);
            obj->questowner = str_dup(ch->name);
            obj_to_char(obj, ch);
            wear_obj(ch, obj, TRUE);
        }

        if ((pObjIndex = get_obj_index(STARTING_NEWBIE_SWORD_1H)) != NULL)
        {
            obj = create_object(pObjIndex, 25);
            obj->questowner = str_dup(ch->name);
            obj_to_char(obj, ch);
            wear_obj(ch, obj, TRUE);
        }
    }

    if(option == STARTING_OPTION_2HANDER)
    {
        if ((pObjIndex = get_obj_index(STARTING_NEWBIE_SWORD_2H)) != NULL)
        {
            obj = create_object(pObjIndex, 25);
            obj->questowner = str_dup(ch->name);
            obj_to_char(obj, ch);
            wear_obj(ch, obj, TRUE);
        }
    }

    return;
}

void do_changes(CHAR_DATA *ch, char *argument)
{

}