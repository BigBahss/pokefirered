#if DEBUG

#include "global.h"
#include "event_data.h"
#include "field_fadetransition.h"
#include "item.h"
#include "list_menu.h"
#include "main.h"
#include "map_name_popup.h"
#include "menu.h"
#include "new_menu_helpers.h"
#include "overworld.h"
#include "party_menu.h"
#include "pokemon.h"
#include "script.h"
#include "script_pokemon_util.h"
#include "sound.h"
#include "strings.h"
#include "task.h"
#include "text_window.h"
#include "constants/items.h"
#include "constants/maps.h"
#include "constants/moves.h"
#include "constants/pokemon.h"
#include "constants/species.h"
#include "constants/songs.h"

#define DEBUG_MAIN_MENU_HEIGHT 7
#define DEBUG_MAIN_MENU_WIDTH 11

void Debug_ShowMainMenu(void);
static void Debug_DestroyMainMenu(u8);
static void DebugAction_WildEncounters(u8);
static void DebugAction_RareCandy(u8);
static void DebugAction_MasterBall(u8);
static void DebugAction_Mew(u8);
static void DebugAction_Warp(u8);
static void DebugAction_Cancel(u8);
static void DebugTask_HandleMainMenuInput(u8);

static const u8 gDebugText_WildEncounters[] = _("Encounters");
static const u8 gDebugText_RareCandy[] = _("Rare Candies");
static const u8 gDebugText_MasterBall[] = _("Master Balls");
static const u8 gDebugText_Mew[] = _("Mew + HMs");
static const u8 gDebugText_Warp[] = _("Warp");
static const u8 gDebugText_Cancel[] = _("Cancel");

enum {
    DEBUG_MENU_ITEM_WILD_ENCOUNTERS,
    DEBUG_MENU_ITEM_RARE_CANDY,
    DEBUG_MENU_ITEM_MASTER_BALL,
    DEBUG_MENU_ITEM_MEW,
    DEBUG_MENU_ITEM_WARP,
    DEBUG_MENU_ITEM_CANCEL,
};

static const struct ListMenuItem sDebugMenuItems[] =
{
    [DEBUG_MENU_ITEM_WILD_ENCOUNTERS] = {gDebugText_WildEncounters, DEBUG_MENU_ITEM_WILD_ENCOUNTERS},
    [DEBUG_MENU_ITEM_RARE_CANDY] = {gDebugText_RareCandy, DEBUG_MENU_ITEM_RARE_CANDY},
    [DEBUG_MENU_ITEM_MASTER_BALL] = {gDebugText_MasterBall, DEBUG_MENU_ITEM_MASTER_BALL},
    [DEBUG_MENU_ITEM_MEW] = {gDebugText_Mew, DEBUG_MENU_ITEM_MEW},
    [DEBUG_MENU_ITEM_WARP] = {gDebugText_Warp, DEBUG_MENU_ITEM_WARP},
    [DEBUG_MENU_ITEM_CANCEL] = {gDebugText_Cancel, DEBUG_MENU_ITEM_CANCEL},
};

static void (*const sDebugMenuActions[])(u8) =
{
    [DEBUG_MENU_ITEM_WILD_ENCOUNTERS] = DebugAction_WildEncounters,
    [DEBUG_MENU_ITEM_RARE_CANDY] = DebugAction_RareCandy,
    [DEBUG_MENU_ITEM_MASTER_BALL] = DebugAction_MasterBall,
    [DEBUG_MENU_ITEM_MEW] = DebugAction_Mew,
    [DEBUG_MENU_ITEM_WARP] = DebugAction_Warp,
    [DEBUG_MENU_ITEM_CANCEL] = DebugAction_Cancel,
};

static const struct WindowTemplate sDebugMenuWindowTemplate =
{
    .bg = 0,
    .tilemapLeft = 1,
    .tilemapTop = 1,
    .width = DEBUG_MAIN_MENU_WIDTH,
    .height = 2 * DEBUG_MAIN_MENU_HEIGHT,
    .paletteNum = 15,
    .baseBlock = 1,
};

static const struct ListMenuTemplate sDebugMenuListTemplate =
{
    .items = sDebugMenuItems,
    .moveCursorFunc = ListMenuDefaultCursorMoveFunc,
    .totalItems = ARRAY_COUNT(sDebugMenuItems),
    .maxShowed = DEBUG_MAIN_MENU_HEIGHT,
    .windowId = 0,
    .header_X = 0,
    .item_X = 8,
    .cursor_X = 0,
    .upText_Y = 1,
    .cursorPal = 2,
    .fillValue = 1,
    .cursorShadowPal = 3,
    .lettersSpacing = 1,
    .itemVerticalPadding = 0,
    .scrollMultiple = LIST_NO_MULTIPLE_SCROLL,
    .fontId = 1,
    .cursorKind = 0
};

void Debug_ShowMainMenu(void)
{
    struct ListMenuTemplate menuTemplate;
    u8 windowId;
    u8 menuTaskId;
    u8 inputTaskId;

    // create window
    DismissMapNamePopup();
    LoadMessageBoxAndBorderGfx();
    windowId = AddWindow(&sDebugMenuWindowTemplate);
    DrawStdWindowFrame(windowId, FALSE);

    // create list menu
    menuTemplate = sDebugMenuListTemplate;
    menuTemplate.windowId = windowId;
    menuTaskId = ListMenuInit(&menuTemplate, 0, 0);

    // draw everything
    CopyWindowToVram(windowId, 3);

    // create input handler task
    inputTaskId = CreateTask(DebugTask_HandleMainMenuInput, 3);
    gTasks[inputTaskId].data[0] = menuTaskId;
    gTasks[inputTaskId].data[1] = windowId;
}

static void Debug_DestroyMainMenu(u8 taskId)
{
    DestroyListMenuTask(gTasks[taskId].data[0], NULL, NULL);
    ClearStdWindowAndFrameToTransparent(gTasks[taskId].data[1], TRUE);
    RemoveWindow(gTasks[taskId].data[1]);
    DestroyTask(taskId);
    EnableBothScriptContexts();
}

static void DebugTask_HandleMainMenuInput(u8 taskId)
{
    void (*func)(u8);
    u32 input = ListMenu_ProcessInput(gTasks[taskId].data[0]);

    if (gMain.newKeys & A_BUTTON)
    {
        PlaySE(SE_SELECT);
        if ((func = sDebugMenuActions[input]) != NULL)
            func(taskId);
    }
    else if (gMain.newKeys & B_BUTTON)
    {
        PlaySE(SE_SELECT);
        Debug_DestroyMainMenu(taskId);
    }
}

static void DebugAction_WildEncounters(u8 taskId)
{
    if (FlagGet(FLAG_DEBUG_DISABLE_WILD_ENCOUNTERS) == 0)
        FlagSet(FLAG_DEBUG_DISABLE_WILD_ENCOUNTERS);
    else
        FlagClear(FLAG_DEBUG_DISABLE_WILD_ENCOUNTERS);
}

static void DebugAction_RareCandy(u8 taskId)
{
    AddBagItem(ITEM_RARE_CANDY, 99);
}

static void DebugAction_MasterBall(u8 taskId)
{
    AddBagItem(ITEM_MASTER_BALL, 99);
}

static void DebugAction_Mew(u8 taskId)
{
    ScriptGiveMon(SPECIES_MEW, 100, 0, 0, 0, 0);
    DeleteFirstMoveAndGiveMoveToMon(&gPlayerParty[gPlayerPartyCount - 1], MOVE_SURF);
    DeleteFirstMoveAndGiveMoveToMon(&gPlayerParty[gPlayerPartyCount - 1], MOVE_FLY);
    DeleteFirstMoveAndGiveMoveToMon(&gPlayerParty[gPlayerPartyCount - 1], MOVE_WATERFALL);
    DeleteFirstMoveAndGiveMoveToMon(&gPlayerParty[gPlayerPartyCount - 1], MOVE_CUT);
    AddBagItem(ITEM_HM01, 1);
    AddBagItem(ITEM_HM02, 1);
    AddBagItem(ITEM_HM03, 1);
    AddBagItem(ITEM_HM04, 1);
    AddBagItem(ITEM_HM05, 1);
    AddBagItem(ITEM_HM06, 1);
    AddBagItem(ITEM_HM07, 1);
    AddBagItem(ITEM_HM08, 1);
    FlagSet(FLAG_BADGE01_GET);
    FlagSet(FLAG_BADGE02_GET);
    FlagSet(FLAG_BADGE03_GET);
    FlagSet(FLAG_BADGE04_GET);
    FlagSet(FLAG_BADGE05_GET);
    FlagSet(FLAG_BADGE06_GET);
    FlagSet(FLAG_BADGE07_GET);
    FlagSet(FLAG_BADGE08_GET);
}

static void DebugAction_Warp(u8 taskId)
{
    SetWarpDestinationToMapWarp(MAP_GROUP(MT_SILVER_EXTERIOR), MAP_NUM(MT_SILVER_EXTERIOR), 0);
    DoWarp();
    Debug_DestroyMainMenu(taskId);
}

static void DebugAction_Cancel(u8 taskId)
{
    Debug_DestroyMainMenu(taskId);
}

#endif