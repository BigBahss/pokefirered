#include "global.h"
#include "gflib.h"
#include "util.h"
#include "event_data.h"
#include "cereader_tool.h"
#include "easy_chat.h"
#include "battle_setup.h"
#include "battle_transition.h"
#include "battle.h"
#include "battle_main.h"
#include "overworld.h"
#include "item.h"
#include "menu.h"
#include "new_menu_helpers.h"
#include "constants/vars.h"
#include "constants/items.h"
#include "constants/species.h"
#include "constants/songs.h"
#include "constants/layouts.h"
#include "constants/trainers.h"
#include "constants/facility_trainer_classes.h"
#include "constants/event_objects.h"
#include "constants/trainer_tower.h"

#define CURR_FLOOR sTrainerTowerState->unk_0004.floors[sTrainerTowerState->floorIdx]
#define TRAINER_TOWER gSaveBlock1Ptr->trainerTower[gSaveBlock1Ptr->towerChallengeId]

struct UnkStruct_203F458
{
    /* 0x0000 */ u8 floorIdx;
    /* 0x0004 */ struct EReaderTrainerTowerSet unk_0004;
};

struct TrainerTowerOpponent
{
    /* 0x00 */ u8 name[11];
    /* 0x0C */ u16 speechWin[6];
    /* 0x18 */ u16 speechLose[6];
    /* 0x24 */ u16 speechWin2[6];
    /* 0x30 */ u16 speechLose2[6];
    /* 0x3C */ u8 battleType;
    /* 0x3D */ u8 facilityClass;
    /* 0x3E */ u8 gender;
};

struct SinglesTrainerInfo
{
    u8 objGfx;
    u8 facilityClass;
    bool8 gender;
};

struct DoublesTrainerInfo
{
    u8 objGfx1;
    u8 objGfx2;
    u8 facilityClass;
    bool8 gender1;
    bool8 gender2;
};

struct TrainerEncounterMusicPairs
{
    u8 facilityClass;
    u8 musicId;
};

static EWRAM_DATA struct UnkStruct_203F458 * sTrainerTowerState = NULL;
static EWRAM_DATA struct TrainerTowerOpponent * sTrainerTowerOpponent = NULL;
static EWRAM_DATA u8 sUnused_203F460 = 0;

static void SetUpTrainerTowerDataStruct(void);
static void FreeTrainerTowerDataStruct(void);
static void InitTrainerTowerFloor(void);
static void SetTrainerTowerNPCGraphics(void);
static void TT_ConvertEasyChatMessageToString(u16 *ecWords, u8 *dest);
static void BufferTowerOpponentSpeech(void);
static void TrainerTowerGetOpponentTextColor(u8 battleType, u8 facilityClass);
static void DoTrainerTowerBattle(void);
static void TrainerTowerGetChallengeType(void);
static void TrainerTowerAddFloorCleared(void);
static void GetFloorAlreadyCleared(void);
static void StartTrainerTowerChallenge(void);
static void GetOwnerState(void);
static void GiveChallengePrize(void);
static void CheckFinalTime(void);
static void TrainerTowerResumeTimer(void);
static void TrainerTowerSetPlayerLost(void);
static void GetTrainerTowerChallengeStatus(void);
static void GetCurrentTime(void);
static void ShowResultsBoard(void);
static void CloseResultsBoard(void);
static void TrainerTowerGetDoublesEligiblity(void);
static void TrainerTowerGetNumFloors(void);
static void ShouldWarpToCounter(void);
static void PlayTrainerTowerEncounterMusic(void);
static void HasSpokenToOwner(void);
static void BuildEnemyParty(void);
static s32 GetPartyMaxLevel(void);
static void ValidateOrResetCurTrainerTowerRecord(void);
static u32 GetTrainerTowerRecordTime(u32 *);
static void SetTrainerTowerRecordTime(u32 *, u32);

extern const u8 gText_TimeBoard[];
extern const u8 gText_XMinYZSec[];
extern const u8 *const gTrainerTowerChallengeTypeTexts[];

static const struct SinglesTrainerInfo sSingleBattleTrainerInfo[] = {
    {OBJ_EVENT_GFX_WOMAN_2,        FACILITY_CLASS_AROMA_LADY,       FEMALE},
    {OBJ_EVENT_GFX_HIKER,          FACILITY_CLASS_RUIN_MANIAC,      MALE},
    {OBJ_EVENT_GFX_TUBER_F,        FACILITY_CLASS_TUBER,            FEMALE},
    {OBJ_EVENT_GFX_TUBER_M_LAND,   FACILITY_CLASS_TUBER_2,          MALE},
    {OBJ_EVENT_GFX_COOLTRAINER_M,  FACILITY_CLASS_COOLTRAINER,      MALE},
    {OBJ_EVENT_GFX_COOLTRAINER_F,  FACILITY_CLASS_COOLTRAINER_2,    FEMALE},
    {OBJ_EVENT_GFX_SUPER_NERD,     FACILITY_CLASS_HEX_MANIAC,       MALE},
    {OBJ_EVENT_GFX_WOMAN_2,        FACILITY_CLASS_LADY,             FEMALE},
    {OBJ_EVENT_GFX_BEAUTY,         FACILITY_CLASS_BEAUTY,           FEMALE},
    {OBJ_EVENT_GFX_BOY,            FACILITY_CLASS_RICH_BOY,         MALE},
    {OBJ_EVENT_GFX_SUPER_NERD,     FACILITY_CLASS_POKEMANIAC,       MALE},
    {OBJ_EVENT_GFX_SWIMMER_M_LAND, FACILITY_CLASS_SWIMMER_MALE,     MALE},
    {OBJ_EVENT_GFX_BLACKBELT,      FACILITY_CLASS_BLACK_BELT,       MALE},
    {OBJ_EVENT_GFX_ROCKER,         FACILITY_CLASS_GUITARIST,        MALE},
    {OBJ_EVENT_GFX_ROCKER,         FACILITY_CLASS_KINDLER,          MALE},
    {OBJ_EVENT_GFX_CAMPER,         FACILITY_CLASS_CAMPER,           MALE},
    {OBJ_EVENT_GFX_SUPER_NERD,     FACILITY_CLASS_BUG_MANIAC,       MALE},
    {OBJ_EVENT_GFX_BOY,            FACILITY_CLASS_PSYCHIC,          MALE},
    {OBJ_EVENT_GFX_WOMAN_1,        FACILITY_CLASS_PSYCHIC_2,        FEMALE},
    {OBJ_EVENT_GFX_GENTLEMAN,      FACILITY_CLASS_GENTLEMAN,        MALE},
    {OBJ_EVENT_GFX_BOY,            FACILITY_CLASS_SCHOOL_KID,       MALE},
    {OBJ_EVENT_GFX_WOMAN_1,        FACILITY_CLASS_SCHOOL_KID_2,     FEMALE},
    {OBJ_EVENT_GFX_BALDING_MAN,    FACILITY_CLASS_POKEFAN,          MALE},
    {OBJ_EVENT_GFX_WOMAN_3,        FACILITY_CLASS_POKEFAN_2,        FEMALE},
    {OBJ_EVENT_GFX_OLD_MAN_1,      FACILITY_CLASS_EXPERT,           MALE},
    {OBJ_EVENT_GFX_OLD_WOMAN,      FACILITY_CLASS_EXPERT_2,         FEMALE},
    {OBJ_EVENT_GFX_YOUNGSTER,      FACILITY_CLASS_YOUNGSTER,        MALE},
    {OBJ_EVENT_GFX_FISHER,         FACILITY_CLASS_FISHERMAN,        MALE},
    {OBJ_EVENT_GFX_COOLTRAINER_M,  FACILITY_CLASS_DRAGON_TAMER,     MALE},
    {OBJ_EVENT_GFX_ROCKER,         FACILITY_CLASS_BIRD_KEEPER,      MALE},
    {OBJ_EVENT_GFX_LITTLE_BOY,     FACILITY_CLASS_NINJA_BOY,        MALE},
    {OBJ_EVENT_GFX_BATTLE_GIRL,    FACILITY_CLASS_BATTLE_GIRL,      FEMALE},
    {OBJ_EVENT_GFX_BEAUTY,         FACILITY_CLASS_PARASOL_LADY,     FEMALE},
    {OBJ_EVENT_GFX_SWIMMER_F_LAND, FACILITY_CLASS_SWIMMER_FEMALE,   FEMALE},
    {OBJ_EVENT_GFX_PICNICKER,      FACILITY_CLASS_PICNICKER,        FEMALE},
    {OBJ_EVENT_GFX_SAILOR,         FACILITY_CLASS_SAILOR,           MALE},
    {OBJ_EVENT_GFX_FAT_MAN,        FACILITY_CLASS_COLLECTOR,        MALE},
    {OBJ_EVENT_GFX_MAN,            FACILITY_CLASS_PKMN_BREEDER,     MALE},
    {OBJ_EVENT_GFX_WOMAN_2,        FACILITY_CLASS_PKMN_BREEDER_2,   FEMALE},
    {OBJ_EVENT_GFX_CAMPER,         FACILITY_CLASS_PKMN_RANGER,      MALE},
    {OBJ_EVENT_GFX_PICNICKER,      FACILITY_CLASS_PKMN_RANGER_2,    FEMALE},
    {OBJ_EVENT_GFX_LASS,           FACILITY_CLASS_LASS,             FEMALE},
    {OBJ_EVENT_GFX_BUG_CATCHER,    FACILITY_CLASS_BUG_CATCHER,      MALE},
    {OBJ_EVENT_GFX_HIKER,          FACILITY_CLASS_HIKER,            MALE},
    {OBJ_EVENT_GFX_YOUNGSTER,      FACILITY_CLASS_YOUNGSTER_2,      MALE},
    {OBJ_EVENT_GFX_BUG_CATCHER,    FACILITY_CLASS_BUG_CATCHER_2,    MALE},
    {OBJ_EVENT_GFX_LASS,           FACILITY_CLASS_LASS_2,           FEMALE},
    {OBJ_EVENT_GFX_SAILOR,         FACILITY_CLASS_SAILOR_2,         MALE},
    {OBJ_EVENT_GFX_CAMPER,         FACILITY_CLASS_CAMPER_2,         MALE},
    {OBJ_EVENT_GFX_PICNICKER,      FACILITY_CLASS_PICNICKER_2,      FEMALE},
    {OBJ_EVENT_GFX_SUPER_NERD,     FACILITY_CLASS_POKEMANIAC_2,     MALE},
    {OBJ_EVENT_GFX_SUPER_NERD,     FACILITY_CLASS_SUPER_NERD,       MALE},
    {OBJ_EVENT_GFX_HIKER,          FACILITY_CLASS_HIKER_2,          MALE},
    {OBJ_EVENT_GFX_BIKER,          FACILITY_CLASS_BIKER,            MALE},
    {OBJ_EVENT_GFX_SUPER_NERD,     FACILITY_CLASS_BURGLAR,          MALE},
    {OBJ_EVENT_GFX_BALDING_MAN,    FACILITY_CLASS_ENGINEER,         MALE},
    {OBJ_EVENT_GFX_FISHER,         FACILITY_CLASS_FISHERMAN_2,      MALE},
    {OBJ_EVENT_GFX_SWIMMER_M_LAND, FACILITY_CLASS_SWIMMER_MALE_2,   MALE},
    {OBJ_EVENT_GFX_BIKER,          FACILITY_CLASS_CUE_BALL,         MALE},
    {OBJ_EVENT_GFX_OLD_MAN_1,      FACILITY_CLASS_GAMER,            MALE},
    {OBJ_EVENT_GFX_BEAUTY,         FACILITY_CLASS_BEAUTY_2,         FEMALE},
    {OBJ_EVENT_GFX_SWIMMER_F_LAND, FACILITY_CLASS_SWIMMER_FEMALE_2, FEMALE},
    {OBJ_EVENT_GFX_BOY,            FACILITY_CLASS_PSYCHIC_3,        MALE},
    {OBJ_EVENT_GFX_ROCKER,         FACILITY_CLASS_ROCKER,           MALE},
    {OBJ_EVENT_GFX_ROCKER,         FACILITY_CLASS_JUGGLER,          MALE},
    {OBJ_EVENT_GFX_MAN,            FACILITY_CLASS_TAMER,            MALE},
    {OBJ_EVENT_GFX_ROCKER,         FACILITY_CLASS_BIRD_KEEPER_2,    MALE},
    {OBJ_EVENT_GFX_BLACKBELT,      FACILITY_CLASS_BLACK_BELT_2,     MALE},
    {OBJ_EVENT_GFX_SCIENTIST,      FACILITY_CLASS_SCIENTIST,        MALE},
    {OBJ_EVENT_GFX_COOLTRAINER_M,  FACILITY_CLASS_COOLTRAINER_3,    MALE},
    {OBJ_EVENT_GFX_COOLTRAINER_F,  FACILITY_CLASS_COOLTRAINER_4,    FEMALE},
    {OBJ_EVENT_GFX_GENTLEMAN,      FACILITY_CLASS_GENTLEMAN_2,      MALE},
    {OBJ_EVENT_GFX_CHANNELER,      FACILITY_CLASS_CHANNELER,        FEMALE},
    {OBJ_EVENT_GFX_WOMAN_1,        FACILITY_CLASS_PSYCHIC_4,        FEMALE},
    {OBJ_EVENT_GFX_BATTLE_GIRL,    FACILITY_CLASS_CRUSH_GIRL,       FEMALE},
    {OBJ_EVENT_GFX_TUBER_F,        FACILITY_CLASS_TUBER_3,          FEMALE},
    {OBJ_EVENT_GFX_WOMAN_2,        FACILITY_CLASS_PKMN_BREEDER_3,   FEMALE},
    {OBJ_EVENT_GFX_CAMPER,         FACILITY_CLASS_PKMN_RANGER_3,    MALE},
    {OBJ_EVENT_GFX_PICNICKER,      FACILITY_CLASS_PKMN_RANGER_4,    FEMALE},
    {OBJ_EVENT_GFX_WOMAN_2,        FACILITY_CLASS_AROMA_LADY_2,     FEMALE},
    {OBJ_EVENT_GFX_HIKER,          FACILITY_CLASS_RUIN_MANIAC_2,    MALE},
    {OBJ_EVENT_GFX_WOMAN_2,        FACILITY_CLASS_LADY_2,           FEMALE},
    {OBJ_EVENT_GFX_LASS,           FACILITY_CLASS_PAINTER,          FEMALE}
};

static const struct DoublesTrainerInfo sDoubleBattleTrainerInfo[] = {
    {OBJ_EVENT_GFX_BEAUTY,         OBJ_EVENT_GFX_WOMAN_1,        FACILITY_CLASS_SR_AND_JR,      FEMALE, FEMALE},
    {OBJ_EVENT_GFX_LITTLE_GIRL,    OBJ_EVENT_GFX_LITTLE_GIRL,    FACILITY_CLASS_TWINS,          FEMALE, FEMALE},
    {OBJ_EVENT_GFX_BEAUTY,         OBJ_EVENT_GFX_MAN,            FACILITY_CLASS_YOUNG_COUPLE,   FEMALE, MALE},
    {OBJ_EVENT_GFX_OLD_MAN_1,      OBJ_EVENT_GFX_OLD_WOMAN,      FACILITY_CLASS_OLD_COUPLE,     MALE, FEMALE},
    {OBJ_EVENT_GFX_TUBER_M_LAND,   OBJ_EVENT_GFX_SWIMMER_F_LAND, FACILITY_CLASS_SIS_AND_BRO,    MALE, FEMALE},
    {OBJ_EVENT_GFX_LITTLE_GIRL,    OBJ_EVENT_GFX_LITTLE_GIRL,    FACILITY_CLASS_TWINS_2,        FEMALE, FEMALE},
    {OBJ_EVENT_GFX_COOLTRAINER_M,  OBJ_EVENT_GFX_COOLTRAINER_F,  FACILITY_CLASS_COOL_COUPLE,    MALE, FEMALE},
    {OBJ_EVENT_GFX_BEAUTY,         OBJ_EVENT_GFX_MAN,            FACILITY_CLASS_YOUNG_COUPLE_2, FEMALE, MALE},
    {OBJ_EVENT_GFX_BATTLE_GIRL,    OBJ_EVENT_GFX_BLACKBELT,      FACILITY_CLASS_CRUSH_KIN,      FEMALE, MALE},
    {OBJ_EVENT_GFX_SWIMMER_F_LAND, OBJ_EVENT_GFX_TUBER_M_LAND,   FACILITY_CLASS_SIS_AND_BRO_2,  FEMALE, MALE}
};

static const struct TrainerEncounterMusicPairs sTrainerEncounterMusicLUT[105] = {
    {FACILITY_CLASS_AROMA_LADY,       TRAINER_ENCOUNTER_MUSIC_AQUA},
    {FACILITY_CLASS_BOARDER_2,        TRAINER_ENCOUNTER_MUSIC_AQUA},
    {FACILITY_CLASS_TEAM_AQUA_2,      TRAINER_ENCOUNTER_MUSIC_AQUA},
    {FACILITY_CLASS_GAMER,            TRAINER_ENCOUNTER_MUSIC_FEMALE},
    {FACILITY_CLASS_RUIN_MANIAC,      TRAINER_ENCOUNTER_MUSIC_FEMALE},
    {FACILITY_CLASS_YOUNGSTER,        TRAINER_ENCOUNTER_MUSIC_INTENSE},
    {FACILITY_CLASS_YOUNG_COUPLE,     TRAINER_ENCOUNTER_MUSIC_FEMALE},
    {FACILITY_CLASS_FISHERMAN,        TRAINER_ENCOUNTER_MUSIC_FEMALE},
    {FACILITY_CLASS_TRIATHLETE_4,     TRAINER_ENCOUNTER_MUSIC_MALE},
    {FACILITY_CLASS_PKMN_RANGER,      TRAINER_ENCOUNTER_MUSIC_SUSPICIOUS},
    {FACILITY_CLASS_LEADER_2,         TRAINER_ENCOUNTER_MUSIC_TWINS},
    {FACILITY_CLASS_SIS_AND_BRO,      TRAINER_ENCOUNTER_MUSIC_SUSPICIOUS},
    {FACILITY_CLASS_MAGMA_LEADER,     TRAINER_ENCOUNTER_MUSIC_SUSPICIOUS},
    {FACILITY_CLASS_POKEFAN_2,        TRAINER_ENCOUNTER_MUSIC_INTENSE},
    {FACILITY_CLASS_PKMN_RANGER_2,    TRAINER_ENCOUNTER_MUSIC_SUSPICIOUS},
    {FACILITY_CLASS_PSYCHIC_3,        TRAINER_ENCOUNTER_MUSIC_FEMALE},
    {FACILITY_CLASS_PICNICKER_2,      TRAINER_ENCOUNTER_MUSIC_COOL},
    {FACILITY_CLASS_ELITE_FOUR_3,     TRAINER_ENCOUNTER_MUSIC_COOL},
    {FACILITY_CLASS_COOLTRAINER_2,    TRAINER_ENCOUNTER_MUSIC_COOL},
    {FACILITY_CLASS_GUITARIST,        TRAINER_ENCOUNTER_MUSIC_INTENSE},
    {FACILITY_CLASS_BUG_CATCHER,      TRAINER_ENCOUNTER_MUSIC_SUSPICIOUS},
    {FACILITY_CLASS_PKMN_TRAINER_4,   TRAINER_ENCOUNTER_MUSIC_MALE},
    {FACILITY_CLASS_TRIATHLETE_3,     TRAINER_ENCOUNTER_MUSIC_MALE},
    {FACILITY_CLASS_CAMPER_2,         TRAINER_ENCOUNTER_MUSIC_TWINS},
    {FACILITY_CLASS_TRIATHLETE_2,     TRAINER_ENCOUNTER_MUSIC_TWINS},
    {FACILITY_CLASS_TUBER,            TRAINER_ENCOUNTER_MUSIC_INTERVIEWER},
    {FACILITY_CLASS_BEAUTY_2,         TRAINER_ENCOUNTER_MUSIC_HIKER},
    {FACILITY_CLASS_INTERVIEWER,      TRAINER_ENCOUNTER_MUSIC_HIKER},
    {FACILITY_CLASS_YOUNGSTER_2,      TRAINER_ENCOUNTER_MUSIC_RICH},
    {FACILITY_CLASS_PSYCHIC_2,        TRAINER_ENCOUNTER_MUSIC_RICH},
    {FACILITY_CLASS_TEAM_MAGMA_2,     TRAINER_ENCOUNTER_MUSIC_SWIMMER},
    {FACILITY_CLASS_SWIMMER_MALE,     TRAINER_ENCOUNTER_MUSIC_FEMALE},
    {FACILITY_CLASS_PKMN_TRAINER_7,   TRAINER_ENCOUNTER_MUSIC_SUSPICIOUS},
    {FACILITY_CLASS_POKEMANIAC,       TRAINER_ENCOUNTER_MUSIC_SUSPICIOUS},
    {FACILITY_CLASS_SUPER_NERD,       TRAINER_ENCOUNTER_MUSIC_INTENSE},
    {FACILITY_CLASS_ENGINEER,         TRAINER_ENCOUNTER_MUSIC_INTENSE},
    {FACILITY_CLASS_MAGMA_ADMIN_2,    TRAINER_ENCOUNTER_MUSIC_INTENSE},
    {FACILITY_CLASS_BLACK_BELT,       TRAINER_ENCOUNTER_MUSIC_INTENSE},
    {FACILITY_CLASS_LEADER_5,         TRAINER_ENCOUNTER_MUSIC_SUSPICIOUS},
    {FACILITY_CLASS_SAILOR,           TRAINER_ENCOUNTER_MUSIC_INTENSE},
    {FACILITY_CLASS_SAILOR_2,         TRAINER_ENCOUNTER_MUSIC_SUSPICIOUS},
    {FACILITY_CLASS_BUG_MANIAC,       TRAINER_ENCOUNTER_MUSIC_SUSPICIOUS},
    {FACILITY_CLASS_PKMN_TRAINER_5,   TRAINER_ENCOUNTER_MUSIC_MALE},
    {FACILITY_CLASS_CAMPER,           TRAINER_ENCOUNTER_MUSIC_MALE},
    {FACILITY_CLASS_KINDLER,          TRAINER_ENCOUNTER_MUSIC_HIKER},
    {FACILITY_CLASS_BATTLE_GIRL,      TRAINER_ENCOUNTER_MUSIC_MAGMA},
    {FACILITY_CLASS_COLLECTOR,        TRAINER_ENCOUNTER_MUSIC_MAGMA},
    {FACILITY_CLASS_NINJA_BOY,        TRAINER_ENCOUNTER_MUSIC_MAGMA},
    {FACILITY_CLASS_PKMN_TRAINER_3,   TRAINER_ENCOUNTER_MUSIC_FEMALE},
    {FACILITY_CLASS_PARASOL_LADY,     TRAINER_ENCOUNTER_MUSIC_FEMALE},
    {FACILITY_CLASS_AQUA_ADMIN_2,     TRAINER_ENCOUNTER_MUSIC_HIKER},
    {FACILITY_CLASS_PKMN_TRAINER_2,   TRAINER_ENCOUNTER_MUSIC_MALE},
    {FACILITY_CLASS_SWIMMER_FEMALE,   TRAINER_ENCOUNTER_MUSIC_MALE},
    {FACILITY_CLASS_EXPERT_2,         TRAINER_ENCOUNTER_MUSIC_SUSPICIOUS},
    {FACILITY_CLASS_RICH_BOY,         TRAINER_ENCOUNTER_MUSIC_RICH},
    {FACILITY_CLASS_HEX_MANIAC,       TRAINER_ENCOUNTER_MUSIC_SUSPICIOUS},
    {FACILITY_CLASS_HIKER,            TRAINER_ENCOUNTER_MUSIC_FEMALE},
    {FACILITY_CLASS_BEAUTY,           TRAINER_ENCOUNTER_MUSIC_FEMALE},
    {FACILITY_CLASS_SWIMMER_FEMALE_2, TRAINER_ENCOUNTER_MUSIC_FEMALE},
    {FACILITY_CLASS_LADY,             TRAINER_ENCOUNTER_MUSIC_FEMALE},
    {FACILITY_CLASS_CHAMPION,         TRAINER_ENCOUNTER_MUSIC_FEMALE},
    {FACILITY_CLASS_PKMN_TRAINER_6,   TRAINER_ENCOUNTER_MUSIC_GIRL},
    {FACILITY_CLASS_TRIATHLETE,       TRAINER_ENCOUNTER_MUSIC_GIRL},
    {FACILITY_CLASS_SWIMMER_MALE_2,   TRAINER_ENCOUNTER_MUSIC_FEMALE},
    {FACILITY_CLASS_DRAGON_TAMER,     TRAINER_ENCOUNTER_MUSIC_FEMALE},
    {FACILITY_CLASS_BIKER,            TRAINER_ENCOUNTER_MUSIC_FEMALE},
    {FACILITY_CLASS_TRIATHLETE_5,     TRAINER_ENCOUNTER_MUSIC_SUSPICIOUS},
    {FACILITY_CLASS_CUE_BALL,         TRAINER_ENCOUNTER_MUSIC_COOL},
    {FACILITY_CLASS_BIRD_KEEPER,      TRAINER_ENCOUNTER_MUSIC_COOL},
    {FACILITY_CLASS_TRIATHLETE_6,     TRAINER_ENCOUNTER_MUSIC_MALE},
    {FACILITY_CLASS_POKEMANIAC_2,     TRAINER_ENCOUNTER_MUSIC_GIRL},
    {FACILITY_CLASS_TWINS,            TRAINER_ENCOUNTER_MUSIC_GIRL},
    {FACILITY_CLASS_LEADER_4,         TRAINER_ENCOUNTER_MUSIC_MALE},
    {FACILITY_CLASS_BUG_CATCHER_2,    TRAINER_ENCOUNTER_MUSIC_MALE},
    {FACILITY_CLASS_PKMN_BREEDER,     TRAINER_ENCOUNTER_MUSIC_SUSPICIOUS},
    {FACILITY_CLASS_LEADER_8,         TRAINER_ENCOUNTER_MUSIC_AQUA},
    {FACILITY_CLASS_LEADER_6,         TRAINER_ENCOUNTER_MUSIC_AQUA},
    {FACILITY_CLASS_OLD_COUPLE,       TRAINER_ENCOUNTER_MUSIC_INTENSE},
    {FACILITY_CLASS_PSYCHIC,          TRAINER_ENCOUNTER_MUSIC_INTENSE},
    {FACILITY_CLASS_LEADER,           TRAINER_ENCOUNTER_MUSIC_TWINS},
    {FACILITY_CLASS_ELITE_FOUR_4,     TRAINER_ENCOUNTER_MUSIC_ELITE_FOUR},
    {FACILITY_CLASS_GENTLEMAN,        TRAINER_ENCOUNTER_MUSIC_FEMALE},
    {FACILITY_CLASS_LASS,             TRAINER_ENCOUNTER_MUSIC_SUSPICIOUS},
    {FACILITY_CLASS_BURGLAR,          TRAINER_ENCOUNTER_MUSIC_FEMALE},
    {FACILITY_CLASS_PKMN_TRAINER,     TRAINER_ENCOUNTER_MUSIC_MALE},
    {FACILITY_CLASS_SCHOOL_KID,       TRAINER_ENCOUNTER_MUSIC_MALE},
    {FACILITY_CLASS_LEADER_3,         TRAINER_ENCOUNTER_MUSIC_INTENSE},
    {FACILITY_CLASS_POKEFAN,          TRAINER_ENCOUNTER_MUSIC_MALE},
    {FACILITY_CLASS_MAGMA_ADMIN,      TRAINER_ENCOUNTER_MUSIC_COOL},
    {FACILITY_CLASS_EXPERT,           TRAINER_ENCOUNTER_MUSIC_COOL},
    {FACILITY_CLASS_TEAM_MAGMA,       TRAINER_ENCOUNTER_MUSIC_HIKER},
    {FACILITY_CLASS_SR_AND_JR,        TRAINER_ENCOUNTER_MUSIC_HIKER},
    {FACILITY_CLASS_LASS_2,           TRAINER_ENCOUNTER_MUSIC_MALE},
    {FACILITY_CLASS_SCHOOL_KID_2,     TRAINER_ENCOUNTER_MUSIC_MALE},
    {FACILITY_CLASS_COOLTRAINER,      TRAINER_ENCOUNTER_MUSIC_MALE},
    {FACILITY_CLASS_FISHERMAN_2,      TRAINER_ENCOUNTER_MUSIC_GIRL},
    {FACILITY_CLASS_TUBER_2,          TRAINER_ENCOUNTER_MUSIC_GIRL},
    {FACILITY_CLASS_HIKER_2,          TRAINER_ENCOUNTER_MUSIC_SWIMMER},
    {FACILITY_CLASS_BOARDER,          TRAINER_ENCOUNTER_MUSIC_SWIMMER},
    {FACILITY_CLASS_PKMN_BREEDER_2,   TRAINER_ENCOUNTER_MUSIC_HIKER},
    {FACILITY_CLASS_PICNICKER,        TRAINER_ENCOUNTER_MUSIC_HIKER},
    {FACILITY_CLASS_LEADER_7,         TRAINER_ENCOUNTER_MUSIC_MALE},
    {FACILITY_CLASS_ELITE_FOUR,       TRAINER_ENCOUNTER_MUSIC_FEMALE},
    {FACILITY_CLASS_AQUA_ADMIN,       TRAINER_ENCOUNTER_MUSIC_HIKER},
    {FACILITY_CLASS_ELITE_FOUR_2,     TRAINER_ENCOUNTER_MUSIC_MALE}
};

static const struct WindowTemplate sTimeBoardWindowTemplate[] = {
    {0, 3, 1, 27, 18, 15, 0x001},
    DUMMY_WIN_TEMPLATE
};

static const u32 sUnused_847A228 = 0x70;

static const u8 sTextColors[3] = {TEXT_COLOR_TRANSPARENT, TEXT_COLOR_DARK_GREY, TEXT_COLOR_LIGHT_GREY};

static void (*const sTrainerTowerFunctions[])(void) = {
    [TRAINER_TOWER_FUNC_INIT_FLOOR]             = InitTrainerTowerFloor,
    [TRAINER_TOWER_FUNC_GET_SPEECH]             = BufferTowerOpponentSpeech,
    [TRAINER_TOWER_FUNC_DO_BATTLE]              = DoTrainerTowerBattle,
    [TRAINER_TOWER_FUNC_GET_CHALLENGE_TYPE]     = TrainerTowerGetChallengeType,
    [TRAINER_TOWER_FUNC_CLEARED_FLOOR]          = TrainerTowerAddFloorCleared,
    [TRAINER_TOWER_FUNC_GET_FLOOR_CLEARED]      = GetFloorAlreadyCleared,
    [TRAINER_TOWER_FUNC_START_CHALLENGE]        = StartTrainerTowerChallenge,
    [TRAINER_TOWER_FUNC_GET_OWNER_STATE]        = GetOwnerState,
    [TRAINER_TOWER_FUNC_GIVE_PRIZE]             = GiveChallengePrize,
    [TRAINER_TOWER_FUNC_CHECK_FINAL_TIME]       = CheckFinalTime,
    [TRAINER_TOWER_FUNC_RESUME_TIMER]           = TrainerTowerResumeTimer,
    [TRAINER_TOWER_FUNC_SET_LOST]               = TrainerTowerSetPlayerLost,
    [TRAINER_TOWER_FUNC_GET_CHALLENGE_STATUS]   = GetTrainerTowerChallengeStatus,
    [TRAINER_TOWER_FUNC_GET_TIME]               = GetCurrentTime,
    [TRAINER_TOWER_FUNC_SHOW_RESULTS]           = ShowResultsBoard,
    [TRAINER_TOWER_FUNC_CLOSE_RESULTS]          = CloseResultsBoard,
    [TRAINER_TOWER_FUNC_CHECK_DOUBLES]          = TrainerTowerGetDoublesEligiblity,
    [TRAINER_TOWER_FUNC_GET_NUM_FLOORS]         = TrainerTowerGetNumFloors,
    [TRAINER_TOWER_FUNC_SHOULD_WARP_TO_COUNTER] = ShouldWarpToCounter,
    [TRAINER_TOWER_FUNC_ENCOUNTER_MUSIC]        = PlayTrainerTowerEncounterMusic,
    [TRAINER_TOWER_FUNC_GET_BEAT_CHALLENGE]     = HasSpokenToOwner
};

// - 1 excludes Mixed challenge, which just uses one of the 3 other types
static const u16 sFloorLayouts[MAX_TRAINER_TOWER_FLOORS][NUM_TOWER_CHALLENGE_TYPES - 1] = {
    {LAYOUT_TRAINER_TOWER_1F, LAYOUT_TRAINER_TOWER_1F_DOUBLES, LAYOUT_TRAINER_TOWER_1F_KNOCKOUT},
    {LAYOUT_TRAINER_TOWER_2F, LAYOUT_TRAINER_TOWER_2F_DOUBLES, LAYOUT_TRAINER_TOWER_2F_KNOCKOUT},
    {LAYOUT_TRAINER_TOWER_3F, LAYOUT_TRAINER_TOWER_3F_DOUBLES, LAYOUT_TRAINER_TOWER_3F_KNOCKOUT},
    {LAYOUT_TRAINER_TOWER_4F, LAYOUT_TRAINER_TOWER_4F_DOUBLES, LAYOUT_TRAINER_TOWER_4F_KNOCKOUT},
    {LAYOUT_TRAINER_TOWER_5F, LAYOUT_TRAINER_TOWER_5F_DOUBLES, LAYOUT_TRAINER_TOWER_5F_KNOCKOUT},
    {LAYOUT_TRAINER_TOWER_6F, LAYOUT_TRAINER_TOWER_6F_DOUBLES, LAYOUT_TRAINER_TOWER_6F_KNOCKOUT},
    {LAYOUT_TRAINER_TOWER_7F, LAYOUT_TRAINER_TOWER_7F_DOUBLES, LAYOUT_TRAINER_TOWER_7F_KNOCKOUT},
    {LAYOUT_TRAINER_TOWER_8F, LAYOUT_TRAINER_TOWER_8F_DOUBLES, LAYOUT_TRAINER_TOWER_8F_KNOCKOUT}
};

static const u16 sPrizeList[] = {
    ITEM_HP_UP,
    ITEM_PROTEIN,
    ITEM_IRON,
    ITEM_CARBOS,
    ITEM_CALCIUM,
    ITEM_ZINC,
    ITEM_BRIGHT_POWDER,
    ITEM_WHITE_HERB,
    ITEM_MENTAL_HERB,
    ITEM_CHOICE_BAND,
    ITEM_KINGS_ROCK,
    ITEM_SCOPE_LENS,
    ITEM_METAL_COAT,
    ITEM_DRAGON_SCALE,
    ITEM_UP_GRADE
};

static const u16 sTrainerTowerEncounterMusic[] = {
    [TRAINER_ENCOUNTER_MUSIC_MALE]        = MUS_ENCOUNTER_BOY,
    [TRAINER_ENCOUNTER_MUSIC_FEMALE]      = MUS_ENCOUNTER_GIRL,
    [TRAINER_ENCOUNTER_MUSIC_GIRL]        = MUS_ENCOUNTER_GIRL,
    [TRAINER_ENCOUNTER_MUSIC_SUSPICIOUS]  = MUS_ENCOUNTER_ROCKET,
    [TRAINER_ENCOUNTER_MUSIC_INTENSE]     = MUS_ENCOUNTER_BOY,
    [TRAINER_ENCOUNTER_MUSIC_COOL]        = MUS_ENCOUNTER_BOY,
    [TRAINER_ENCOUNTER_MUSIC_AQUA]        = MUS_ENCOUNTER_ROCKET,
    [TRAINER_ENCOUNTER_MUSIC_MAGMA]       = MUS_ENCOUNTER_ROCKET,
    [TRAINER_ENCOUNTER_MUSIC_SWIMMER]     = MUS_ENCOUNTER_BOY,
    [TRAINER_ENCOUNTER_MUSIC_TWINS]       = MUS_ENCOUNTER_GIRL,
    [TRAINER_ENCOUNTER_MUSIC_ELITE_FOUR]  = MUS_ENCOUNTER_BOY,
    [TRAINER_ENCOUNTER_MUSIC_HIKER]       = MUS_ENCOUNTER_BOY,
    [TRAINER_ENCOUNTER_MUSIC_INTERVIEWER] = MUS_ENCOUNTER_BOY,
    [TRAINER_ENCOUNTER_MUSIC_RICH]        = MUS_ENCOUNTER_BOY
};

static const u8 sSingleBattleChallengeMonIdxs[][2] = {
    {0x00, 0x02},
    {0x01, 0x03},
    {0x02, 0x04},
    {0x03, 0x05},
    {0x04, 0x01},
    {0x05, 0x02},
    {0x00, 0x03},
    {0x01, 0x04}
};

static const u8 sDoubleBattleChallengeMonIdxs[][2] = {
    {0x00, 0x01},
    {0x01, 0x03},
    {0x02, 0x00},
    {0x03, 0x04},
    {0x04, 0x02},
    {0x05, 0x02},
    {0x00, 0x03},
    {0x01, 0x05}
};

static const u8 sKnockoutChallengeMonIdxs[][3] = {
    {0x00, 0x02, 0x04},
    {0x01, 0x03, 0x05},
    {0x02, 0x03, 0x01},
    {0x03, 0x04, 0x00},
    {0x04, 0x01, 0x02},
    {0x05, 0x00, 0x03},
    {0x00, 0x05, 0x02},
    {0x01, 0x04, 0x05}
};

extern const struct EReaderTrainerTowerSetSubstruct gUnknown_84827AC;
extern const struct TrainerTowerFloor *const gUnknown_84827B4[][MAX_TRAINER_TOWER_FLOORS];

void CallTrainerTowerFunc(void)
{
    SetUpTrainerTowerDataStruct();
    sTrainerTowerFunctions[gSpecialVar_0x8004]();
    FreeTrainerTowerDataStruct();
}

u8 GetTrainerTowerOpponentClass(void)
{
    return gFacilityClassToTrainerClass[sTrainerTowerOpponent->facilityClass];
}

void GetTrainerTowerOpponentName(u8 *dest)
{
    StringCopyN(dest, sTrainerTowerOpponent->name, 11);
}

u8 GetTrainerTowerTrainerFrontSpriteId(void)
{
    return gFacilityClassToPicIndex[sTrainerTowerOpponent->facilityClass];
}

void InitTrainerTowerBattleStruct(void)
{
    u16 trainerId;
    int i;

    SetUpTrainerTowerDataStruct();
    sTrainerTowerOpponent = AllocZeroed(sizeof(*sTrainerTowerOpponent));
    trainerId = VarGet(VAR_TEMP_1);
    StringCopyN(sTrainerTowerOpponent->name, CURR_FLOOR.trainers[trainerId].name, 11);

    for (i = 0; i < 6; i++)
    {
        sTrainerTowerOpponent->speechWin[i] = CURR_FLOOR.trainers[trainerId].speechWin[i];
        sTrainerTowerOpponent->speechLose[i] = CURR_FLOOR.trainers[trainerId].speechLose[i];

        if (CURR_FLOOR.challengeType == CHALLENGE_TYPE_DOUBLE)
        {
            sTrainerTowerOpponent->speechWin2[i] = CURR_FLOOR.trainers[trainerId + 1].speechWin[i];
            sTrainerTowerOpponent->speechLose2[i] = CURR_FLOOR.trainers[trainerId + 1].speechLose[i];
        }
    }

    sTrainerTowerOpponent->battleType = CURR_FLOOR.challengeType;
    sTrainerTowerOpponent->facilityClass = CURR_FLOOR.trainers[trainerId].facilityClass;
    sTrainerTowerOpponent->gender = CURR_FLOOR.trainers[trainerId].gender;
    SetVBlankCounter1Ptr(&TRAINER_TOWER.timer);
    FreeTrainerTowerDataStruct();
}

void FreeTrainerTowerBattleStruct(void)
{
    FREE_AND_SET_NULL(sTrainerTowerOpponent);
}

void GetTrainerTowerOpponentWinText(u8 *dest, u8 opponentIdx)
{
    VarSet(VAR_TEMP_3, opponentIdx);
    TrainerTowerGetOpponentTextColor(sTrainerTowerOpponent->battleType, sTrainerTowerOpponent->facilityClass);
    if (opponentIdx == 0)
        TT_ConvertEasyChatMessageToString(sTrainerTowerOpponent->speechWin, dest);
    else
        TT_ConvertEasyChatMessageToString(sTrainerTowerOpponent->speechWin2, dest);
}

void GetTrainerTowerOpponentLoseText(u8 *dest, u8 opponentIdx)
{
    VarSet(VAR_TEMP_3, opponentIdx);
    TrainerTowerGetOpponentTextColor(sTrainerTowerOpponent->battleType, sTrainerTowerOpponent->facilityClass);
    if (opponentIdx == 0)
        TT_ConvertEasyChatMessageToString(sTrainerTowerOpponent->speechLose, dest);
    else
        TT_ConvertEasyChatMessageToString(sTrainerTowerOpponent->speechLose2, dest);
}

static void SetUpTrainerTowerDataStruct(void) // fakematching
{
    u32 challengeType = gSaveBlock1Ptr->towerChallengeId;
    s32 r4;
    const struct TrainerTowerFloor *const * r7;

    sTrainerTowerState = AllocZeroed(sizeof(*sTrainerTowerState));
    sTrainerTowerState->floorIdx = gMapHeader.mapLayoutId - 42;
    if (ReadTrainerTowerAndValidate() == TRUE)
        CEReaderTool_LoadTrainerTower(&sTrainerTowerState->unk_0004);
    else
    {
        struct UnkStruct_203F458 * r0_ = sTrainerTowerState;
        const struct EReaderTrainerTowerSetSubstruct * r1 = &gUnknown_84827AC;
        memcpy(&r0_->unk_0004, r1, sizeof(struct EReaderTrainerTowerSetSubstruct));
        r7 = gUnknown_84827B4[challengeType];
        for (r4 = 0; r4 < MAX_TRAINER_TOWER_FLOORS; r4++)
        {
        #ifndef NONMATCHING
            void * r5;
            register void * r0 asm("r0") = sTrainerTowerState;
            r5 = (void *)(r4 * sizeof(struct TrainerTowerFloor));
            r0  = r5 + (uintptr_t)r0;
            r0 += offsetof(struct UnkStruct_203F458, unk_0004.floors);
            memcpy(r0, r7[r4], sizeof(struct TrainerTowerFloor));
        #else
            memcpy(&sTrainerTowerState->unk_0004.floors[r4], r7[r4], sizeof(struct TrainerTowerFloor));
        #endif
        }
        sTrainerTowerState->unk_0004.checksum = CalcByteArraySum((void *)sTrainerTowerState->unk_0004.floors, sizeof(sTrainerTowerState->unk_0004.floors));
        ValidateOrResetCurTrainerTowerRecord();
    }
}

static void FreeTrainerTowerDataStruct(void)
{
    FREE_AND_SET_NULL(sTrainerTowerState);
}

static void InitTrainerTowerFloor(void)
{
    if (gMapHeader.mapLayoutId - LAYOUT_TRAINER_TOWER_LOBBY > sTrainerTowerState->unk_0004.numFloors)
    {
        gSpecialVar_Result = 3; // Skip past usable challenge types
        SetCurrentMapLayout(LAYOUT_TRAINER_TOWER_ROOF);
    }
    else
    {
        gSpecialVar_Result = CURR_FLOOR.challengeType;
        SetCurrentMapLayout(sFloorLayouts[sTrainerTowerState->floorIdx][gSpecialVar_Result]);
        SetTrainerTowerNPCGraphics();
    }
}

static void SetTrainerTowerNPCGraphics(void)
{
    s32 i, j;
    u8 trainerGfx1, trainerGfx2, facilityClass;
    switch (CURR_FLOOR.challengeType)
    {
    case CHALLENGE_TYPE_SINGLE:
        facilityClass = CURR_FLOOR.trainers[0].facilityClass;
        for (i = 0; i < NELEMS(sSingleBattleTrainerInfo); i++)
        {
            if (sSingleBattleTrainerInfo[i].facilityClass == facilityClass)
                break;
        }

        if (i != NELEMS(sSingleBattleTrainerInfo))
            trainerGfx1 = sSingleBattleTrainerInfo[i].objGfx;
        else
            trainerGfx1 = OBJ_EVENT_GFX_YOUNGSTER;

        VarSet(VAR_OBJ_GFX_ID_1, trainerGfx1);
        break;
    case CHALLENGE_TYPE_DOUBLE:
        facilityClass = CURR_FLOOR.trainers[0].facilityClass;
        for (i = 0; i < NELEMS(sDoubleBattleTrainerInfo); i++)
        {
            if (sDoubleBattleTrainerInfo[i].facilityClass == facilityClass)
                break;
        }
        if (i != NELEMS(sDoubleBattleTrainerInfo))
        {
            trainerGfx1  = sDoubleBattleTrainerInfo[i].objGfx1;
            trainerGfx2 = sDoubleBattleTrainerInfo[i].objGfx2;
        }
        else
        {
            trainerGfx1  = OBJ_EVENT_GFX_YOUNGSTER;
            trainerGfx2 = OBJ_EVENT_GFX_YOUNGSTER;
        }
        VarSet(VAR_OBJ_GFX_ID_0, trainerGfx1);
        VarSet(VAR_OBJ_GFX_ID_3, trainerGfx2);
        break;
    case CHALLENGE_TYPE_KNOCKOUT:
        for (j = 0; j < MAX_TRAINERS_PER_FLOOR; j++)
        {
            facilityClass = CURR_FLOOR.trainers[j].facilityClass;
            for (i = 0; i < NELEMS(sSingleBattleTrainerInfo); i++)
            {
                if (sSingleBattleTrainerInfo[i].facilityClass == facilityClass)
                    break;
            }

            if (i != NELEMS(sSingleBattleTrainerInfo))
                trainerGfx1 = sSingleBattleTrainerInfo[i].objGfx;
            else
                trainerGfx1 = OBJ_EVENT_GFX_YOUNGSTER;

            switch (j)
            {
            case 0:
                VarSet(VAR_OBJ_GFX_ID_2, trainerGfx1);
                break;
            case 1:
                VarSet(VAR_OBJ_GFX_ID_0, trainerGfx1);
                break;
            case 2:
                VarSet(VAR_OBJ_GFX_ID_1, trainerGfx1);
                break;
            }
        }
    }
}

static void TT_ConvertEasyChatMessageToString(u16 *ecWords, u8 *dest)
{
    s32 r1;
    ConvertEasyChatWordsToString(dest, ecWords, 3, 2);
    if ((unsigned)GetStringWidth(2, dest, -1) > 196)
    {
        ConvertEasyChatWordsToString(dest, ecWords, 2, 3);
        r1 = 0;
        while (dest[r1++] != CHAR_NEWLINE)
            ;
        while (dest[r1] != CHAR_NEWLINE)
            r1++;
        dest[r1] = CHAR_PROMPT_SCROLL;
    }
}

static void BufferTowerOpponentSpeech(void)
{
    u16 trainerId = gSpecialVar_0x8006;
    u8 facilityClass;
    u8 challengeType = CURR_FLOOR.challengeType;
    
    if (challengeType != CHALLENGE_TYPE_DOUBLE)
        facilityClass = CURR_FLOOR.trainers[trainerId].facilityClass;
    else
        facilityClass = CURR_FLOOR.trainers[0].facilityClass;

    switch (gSpecialVar_0x8005)
    {
    case TRAINER_TOWER_TEXT_INTRO:
        TrainerTowerGetOpponentTextColor(challengeType, facilityClass);
        TT_ConvertEasyChatMessageToString(CURR_FLOOR.trainers[trainerId].speechBefore, gStringVar4);
        break;
    case TRAINER_TOWER_TEXT_PLAYER_LOST:
        TrainerTowerGetOpponentTextColor(challengeType, facilityClass);
        TT_ConvertEasyChatMessageToString(CURR_FLOOR.trainers[trainerId].speechWin, gStringVar4);
        break;
    case TRAINER_TOWER_TEXT_PLAYER_WON:
        TrainerTowerGetOpponentTextColor(challengeType, facilityClass);
        TT_ConvertEasyChatMessageToString(CURR_FLOOR.trainers[trainerId].speechLose, gStringVar4);
        break;
    case TRAINER_TOWER_TEXT_AFTER:
        TT_ConvertEasyChatMessageToString(CURR_FLOOR.trainers[trainerId].speechAfter, gStringVar4);
        break;
    }
}

static void TrainerTowerGetOpponentTextColor(u8 challengeType, u8 facilityClass)
{
    u16 gender = MALE;
    int i;
    switch (challengeType)
    {
    case CHALLENGE_TYPE_SINGLE:
    case CHALLENGE_TYPE_KNOCKOUT:
        for (i = 0; i < NELEMS(sSingleBattleTrainerInfo); i++)
        {
            if (sSingleBattleTrainerInfo[i].facilityClass == facilityClass)
                break;
        }
        if (i != NELEMS(sSingleBattleTrainerInfo))
            gender = sSingleBattleTrainerInfo[i].gender;
        break;
    case CHALLENGE_TYPE_DOUBLE:
        for (i = 0; i < NELEMS(sDoubleBattleTrainerInfo); i++)
        {
            if (sDoubleBattleTrainerInfo[i].facilityClass == facilityClass)
                break;
        }
        if (i != NELEMS(sDoubleBattleTrainerInfo))
        {
            if (VarGet(VAR_TEMP_3))
                gender = sDoubleBattleTrainerInfo[i].gender2;
            else
                gender = sDoubleBattleTrainerInfo[i].gender1;
        }
        break;
    }
    gSpecialVar_PrevTextColor = gSpecialVar_TextColor;
    gSpecialVar_TextColor = gender;
}

static void CB2_EndTrainerTowerBattle(void)
{
    SetMainCallback2(CB2_ReturnToFieldContinueScriptPlayMapMusic);
}

static void Task_DoTrainerTowerBattle(u8 taskId)
{
    if (BT_IsDone() == TRUE)
    {
        gMain.savedCallback = CB2_EndTrainerTowerBattle;
        CleanupOverworldWindowsAndTilemaps();
        SetMainCallback2(CB2_InitBattle);
        DestroyTask(taskId);
    }
}

static void DoTrainerTowerBattle(void)
{
    gBattleTypeFlags = BATTLE_TYPE_TRAINER | BATTLE_TYPE_TRAINER_TOWER;

    if (CURR_FLOOR.challengeType == CHALLENGE_TYPE_DOUBLE)
        gBattleTypeFlags |= BATTLE_TYPE_DOUBLE;

    gTrainerBattleOpponent_A = 0;
    BuildEnemyParty();
    CreateTask(Task_DoTrainerTowerBattle, 1);
    PlayMapChosenOrBattleBGM(0);
    BT_StartOnField(BattleSetup_GetBattleTowerBattleTransition());
}

static void TrainerTowerGetChallengeType(void)
{
    if (!gSpecialVar_0x8005)
        gSpecialVar_Result = CURR_FLOOR.challengeType;
}

static void TrainerTowerAddFloorCleared(void)
{
    TRAINER_TOWER.floorsCleared++;
}

// So the player can safely go back through defeated floors to use the Poke Center (or exit challenge)
static void GetFloorAlreadyCleared(void)
{
    u16 mapLayoutId = gMapHeader.mapLayoutId;
    if (mapLayoutId - LAYOUT_TRAINER_TOWER_1F == TRAINER_TOWER.floorsCleared 
     && mapLayoutId - LAYOUT_TRAINER_TOWER_LOBBY <= CURR_FLOOR.floorIdx)
        gSpecialVar_Result = FALSE;
    else
        gSpecialVar_Result = TRUE;
}

static void StartTrainerTowerChallenge(void)
{
    gSaveBlock1Ptr->towerChallengeId = gSpecialVar_0x8005;
    if (gSaveBlock1Ptr->towerChallengeId >= NUM_TOWER_CHALLENGE_TYPES)
        gSaveBlock1Ptr->towerChallengeId = 0;
    ValidateOrResetCurTrainerTowerRecord();
    if (!ReadTrainerTowerAndValidate())
        TRAINER_TOWER.validated = TRUE;
    else
        TRAINER_TOWER.validated = FALSE;
    TRAINER_TOWER.floorsCleared = 0;
    SetVBlankCounter1Ptr(&TRAINER_TOWER.timer);
    TRAINER_TOWER.timer = 0;
    TRAINER_TOWER.spokeToOwner = FALSE;
    TRAINER_TOWER.checkedFinalTime = FALSE;
}

static void GetOwnerState(void)
{
    DisableVBlankCounter1();
    gSpecialVar_Result = 0;

    if (TRAINER_TOWER.spokeToOwner)
        gSpecialVar_Result++;
    if (TRAINER_TOWER.receivedPrize && TRAINER_TOWER.checkedFinalTime)
        gSpecialVar_Result++;

    TRAINER_TOWER.spokeToOwner = TRUE;
}

static void GiveChallengePrize(void)
{
    u16 itemId = sPrizeList[sTrainerTowerState->unk_0004.floors->prize];

    if (TRAINER_TOWER.receivedPrize)
    {
        gSpecialVar_Result = 2;
    }
    else if (AddBagItem(itemId, 1) == TRUE)
    {
        CopyItemName(itemId, gStringVar2);
        TRAINER_TOWER.receivedPrize = TRUE;
        gSpecialVar_Result = 0;
    }
    else
    {
        gSpecialVar_Result = 1;
    }
}

static void CheckFinalTime(void)
{
    if (TRAINER_TOWER.checkedFinalTime)
    {
        gSpecialVar_Result = 2;
    }
    else if (GetTrainerTowerRecordTime(&TRAINER_TOWER.bestTime) > TRAINER_TOWER.timer)
    {
        SetTrainerTowerRecordTime(&TRAINER_TOWER.bestTime, TRAINER_TOWER.timer);
        gSpecialVar_Result = 0;
    }
    else
    {
        gSpecialVar_Result = 1;
    }

    TRAINER_TOWER.checkedFinalTime = TRUE;
}

static void TrainerTowerResumeTimer(void)
{
    if (!TRAINER_TOWER.spokeToOwner)
    {
        if (TRAINER_TOWER.timer >= TRAINER_TOWER_MAX_TIME)
            TRAINER_TOWER.timer = TRAINER_TOWER_MAX_TIME;
        else
            SetVBlankCounter1Ptr(&TRAINER_TOWER.timer);
    }
}

static void TrainerTowerSetPlayerLost(void)
{
    TRAINER_TOWER.hasLost = TRUE;
}

static void GetTrainerTowerChallengeStatus(void)
{
    if (TRAINER_TOWER.hasLost)
    {
        TRAINER_TOWER.hasLost = FALSE;
        gSpecialVar_Result = CHALLENGE_STATUS_LOST;
    }
    else if (TRAINER_TOWER.unkA_4)
    {
        TRAINER_TOWER.unkA_4 = FALSE;
        gSpecialVar_Result = CHALLENGE_STATUS_UNK;
    }
    else
    {
        gSpecialVar_Result = CHALLENGE_STATUS_NORMAL;
    }
}

#define PRINT_TOWER_TIME(src) ({                                                           \
    s32 minutes, seconds, centiseconds, frames;                                            \
                                                                                           \
    frames = (src);                                                                        \
                                                                                           \
    minutes = frames / (60 * 60);                                                          \
    frames %= (60 * 60);                                                                   \
    seconds = frames / 60;                                                                 \
    frames %= 60;                                                                          \
    centiseconds = frames * 168 / 100;                                                     \
                                                                                           \
    ConvertIntToDecimalStringN(gStringVar1, minutes, STR_CONV_MODE_RIGHT_ALIGN, 2);        \
    ConvertIntToDecimalStringN(gStringVar2, seconds, STR_CONV_MODE_RIGHT_ALIGN, 2);        \
    ConvertIntToDecimalStringN(gStringVar3, centiseconds, STR_CONV_MODE_LEADING_ZEROS, 2); \
})

static void GetCurrentTime(void)
{
    if (TRAINER_TOWER.timer >= TRAINER_TOWER_MAX_TIME)
    {
        DisableVBlankCounter1();
        TRAINER_TOWER.timer = TRAINER_TOWER_MAX_TIME;
    }

    PRINT_TOWER_TIME(TRAINER_TOWER.timer);
}

static void ShowResultsBoard(void)
{
    u8 windowId;
    s32 i;

    ValidateOrResetCurTrainerTowerRecord();
    windowId = AddWindow(sTimeBoardWindowTemplate);
    LoadStdWindowFrameGfx();
    DrawStdWindowFrame(windowId, FALSE);
    AddTextPrinterParameterized(windowId, 2, gText_TimeBoard, 0x4A, 0, 0xFF, NULL);

    for (i = 0; i < NUM_TOWER_CHALLENGE_TYPES; i++)
    {
        PRINT_TOWER_TIME(GetTrainerTowerRecordTime(&TRAINER_TOWER.bestTime));

        StringExpandPlaceholders(gStringVar4, gText_XMinYZSec);
        AddTextPrinterParameterized(windowId, 2, gTrainerTowerChallengeTypeTexts[i - 1], 0x18, 0x24 + 0x14 * i, 0xFF, NULL);
        AddTextPrinterParameterized(windowId, 2, gStringVar4, 0x60, 0x2E + 0x14 * i, 0xFF, NULL);
    }

    PutWindowTilemap(windowId);
    CopyWindowToVram(windowId, COPYWIN_BOTH);
    VarSet(VAR_TEMP_1, windowId);
}

static void CloseResultsBoard(void)
{
    u8 windowId = VarGet(VAR_TEMP_1);
    ClearStdWindowAndFrameToTransparent(windowId, TRUE);
    RemoveWindow(windowId);
}

static void TrainerTowerGetDoublesEligiblity(void)
{
    gSpecialVar_Result = GetMonsStateToDoubles();
}


static void TrainerTowerGetNumFloors(void)
{
    if (sTrainerTowerState->unk_0004.numFloors != sTrainerTowerState->unk_0004.floors[0].floorIdx)
    {
        ConvertIntToDecimalStringN(gStringVar1, sTrainerTowerState->unk_0004.numFloors, STR_CONV_MODE_LEFT_ALIGN, 1);
        gSpecialVar_Result = TRUE;
    }
    else
    {
        gSpecialVar_Result = FALSE;
    }
}

// Dummied? equivalent to gSpecialVar_Result = FALSE
// If it were to return TRUE the player would be warped back to the lobby
static void ShouldWarpToCounter(void)
{
    if (gMapHeader.mapLayoutId == LAYOUT_TRAINER_TOWER_LOBBY && VarGet(VAR_MAP_SCENE_TRAINER_TOWER) == 0)
        gSpecialVar_Result = FALSE;
    else
        gSpecialVar_Result = FALSE;
}

static void PlayTrainerTowerEncounterMusic(void)
{
    s32 i;
    u16 idx = VarGet(VAR_TEMP_1);
    u8 facilityClass = CURR_FLOOR.trainers[idx].facilityClass;

    for (i = 0; i < NELEMS(sTrainerEncounterMusicLUT); i++)
    {
        if (sTrainerEncounterMusicLUT[i].facilityClass == gFacilityClassToTrainerClass[facilityClass])
            break;
    }

    if (i != NELEMS(sTrainerEncounterMusicLUT))
    {
        idx = sTrainerEncounterMusicLUT[i].musicId;
    }
    else
    {
        idx = 0;
    }
    PlayNewMapMusic(sTrainerTowerEncounterMusic[idx]);
}

static void HasSpokenToOwner(void)
{
    gSpecialVar_Result = TRAINER_TOWER.spokeToOwner;
}

static void BuildEnemyParty(void)
{
    u16 trainerIdx = VarGet(VAR_TEMP_1);
    s32 level = GetPartyMaxLevel();
    u8 floorIdx = TRAINER_TOWER.floorsCleared;
    s32 i;
    u8 monIdx;

    ZeroEnemyPartyMons();

    switch (CURR_FLOOR.challengeType)
    {
    case CHALLENGE_TYPE_SINGLE:
    default:
        for (i = 0; i < 2; i++)
        {
            monIdx = sSingleBattleChallengeMonIdxs[floorIdx][i];
            CURR_FLOOR.trainers[trainerIdx].mons[monIdx].level = level;
            CreateBattleTowerMon(&gEnemyParty[i], &CURR_FLOOR.trainers[trainerIdx].mons[monIdx]);
        }
        break;
    case CHALLENGE_TYPE_DOUBLE:
        monIdx = sDoubleBattleChallengeMonIdxs[floorIdx][0];
        CURR_FLOOR.trainers[0].mons[monIdx].level = level;
        CreateBattleTowerMon(&gEnemyParty[0], &CURR_FLOOR.trainers[0].mons[monIdx]);

        monIdx = sDoubleBattleChallengeMonIdxs[floorIdx][1];
        CURR_FLOOR.trainers[1].mons[monIdx].level = level;
        CreateBattleTowerMon(&gEnemyParty[1], &CURR_FLOOR.trainers[1].mons[monIdx]);
        break;
    case CHALLENGE_TYPE_KNOCKOUT:
        monIdx = sKnockoutChallengeMonIdxs[floorIdx][trainerIdx];
        CURR_FLOOR.trainers[trainerIdx].mons[monIdx].level = level;
        CreateBattleTowerMon(&gEnemyParty[0], &CURR_FLOOR.trainers[trainerIdx].mons[monIdx]);
        break;
    }
}

static s32 GetPartyMaxLevel(void)
{
    s32 topLevel = 0;
    s32 i;

    for (i = 0; i < PARTY_SIZE; i++)
    {
        if (GetMonData(&gPlayerParty[i], MON_DATA_SPECIES, NULL) != 0 && GetMonData(&gPlayerParty[i], MON_DATA_SPECIES2, NULL) != SPECIES_EGG)
        {
            s32 currLevel = GetMonData(&gPlayerParty[i], MON_DATA_LEVEL, NULL);
            if (currLevel > topLevel)
                topLevel = currLevel;
        }
    }

    return topLevel;
}

static void ValidateOrResetCurTrainerTowerRecord(void)
{
    if (TRAINER_TOWER.unk9 != sTrainerTowerState->unk_0004.id)
    {
        TRAINER_TOWER.unk9 = sTrainerTowerState->unk_0004.id;
        SetTrainerTowerRecordTime(&TRAINER_TOWER.bestTime, TRAINER_TOWER_MAX_TIME);
        TRAINER_TOWER.receivedPrize = FALSE;
    }
}

void PrintTrainerTowerRecords(void)
{
    s32 i;
    u8 windowId = 0;

    SetUpTrainerTowerDataStruct();
    FillWindowPixelRect(0, 0, 0, 0, 0xd8, 0x90);
    ValidateOrResetCurTrainerTowerRecord();
    AddTextPrinterParameterized3(0, 2, 0x4a, 0, sTextColors, 0, gText_TimeBoard);

    for (i = 0; i < NUM_TOWER_CHALLENGE_TYPES; i++)
    {
        PRINT_TOWER_TIME(GetTrainerTowerRecordTime(&gSaveBlock1Ptr->trainerTower[i].bestTime));
        StringExpandPlaceholders(gStringVar4, gText_XMinYZSec);
        AddTextPrinterParameterized3(windowId, 2, 0x18, 0x24 + 0x14 * i, sTextColors, 0, gTrainerTowerChallengeTypeTexts[i]);
        AddTextPrinterParameterized3(windowId, 2, 0x60, 0x24 + 0x14 * i, sTextColors, 0, gStringVar4);
    }

    PutWindowTilemap(windowId);
    CopyWindowToVram(windowId, COPYWIN_BOTH);
    FreeTrainerTowerDataStruct();
}

static u32 GetTrainerTowerRecordTime(u32 *counter)
{
    return *counter ^ gSaveBlock2Ptr->encryptionKey;
}

static void SetTrainerTowerRecordTime(u32 *counter, u32 value)
{
    *counter = value ^ gSaveBlock2Ptr->encryptionKey;
}

void ResetTrainerTowerResults(void)
{
    s32 i;

    for (i = 0; i < NUM_TOWER_CHALLENGE_TYPES; i++)
    {
        SetTrainerTowerRecordTime(&gSaveBlock1Ptr->trainerTower[i].bestTime, TRAINER_TOWER_MAX_TIME);
    }
}
