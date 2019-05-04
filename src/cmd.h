
/*
$Author: void $
$Date: 2004/11/17 19:21:52 $
$Header: /home/ronin/cvs/ronin/cmd.h,v 2.1 2004/11/17 19:21:52 void Exp $
$Id: cmd.h,v 2.1 2004/11/17 19:21:52 void Exp $
$Name:  $
$Log: cmd.h,v $
Revision 2.1  2004/11/17 19:21:52  void
Added Nomad Skill Cover (1/2 Damage -10 Hitroll)

Revision 2.0.0.1  2004/02/05 16:13:03  ronin
Reinitialization of cvs archives


Revision 19-Dec-03 Ranger
Addition of CMD_EMAIL

Revision 22-May-03 Ranger
Addition of CMD_HUNT

Revision 10-Mar-03 Ranger
Addition of CMD_SOCIAL

Revision 12-Feb-03 Ranger
Addition of CMD_BOARD

Revision 1.3  2002/10/14 21:00:33  ronin
Addition of MSG_RECONNECT.

Revision 1.2  2002/03/31 07:42:14  ronin
Addition of header lines.

$State: Exp $
*/

#define MSG_VIOLENCE           -99
#define MSG_ROUND              -98 /* Similar to MSG_VIOLENCE. Used for 3 second pulse signaling. Sent only to enchantments for now. */
#define MSG_DAMAGED            -30
#define MSG_OBJ_DISARMED       -29
#define MSG_OBJ_JUNKED         -28
#define MSG_OBJ_DONATED        -27
#define MSG_OBJ_PUT            -26
#define MSG_AUTORENT           -25
#define MSG_MIRACLE            -24 /* Allows a PC/PC's items to react to being miraed */
#define MSG_UNKNOWN            -23 /* Allows a mob to react after someone has done a social - Skeena 1/10/2011 */
#define MSG_SAID               -22 /* Allows a mob to react after someone has said something - Skeena 1/10/2011 */
#define MSG_GET                -21 /* Allows an object to know if it's being got - Sane 09/29/2006*/
#define MSG_RECONNECT          -20 /* For reconnect link room check - Liner 101402 */
#define MSG_SPELL_CAST         -19
#define MSG_OBJ_DROPPED        -18
#define MSG_OBJ_WORN           -17
#define MSG_GAVE_OBJ           -16
#define MSG_CORPSE             -15
#define MSG_OBJ_ENTERING_GAME  -14
#define MSG_DEAD               -13
#define MSG_DEATHCRY           -12
#define MSG_TARGET             -11
#define MSG_STONE              -10
#define MSG_LEAVE              -9
#define MSG_ENTER              -8
#define MSG_BEING_REMOVED      -7
#define MSG_SHOW_PRETITLE      -6
#define MSG_SHOW_AFFECT_TEXT   -5
#define MSG_ZONE_RESET         -4
#define MSG_TICK               -3
#define MSG_REMOVE_ENCH        -2
#define MSG_DIE                -1
#define MSG_MOBACT              0
#define CMD_NORTH               1
#define CMD_EAST                2
#define CMD_SOUTH               3
#define CMD_WEST                4
#define CMD_UP                  5
#define CMD_DOWN                6
#define CMD_ENTER               7
#define CMD_EXITS               8
#define CMD_NOKILL              9
#define CMD_GET                10
#define CMD_DRINK              11
#define CMD_EAT                12
#define CMD_WEAR               13
#define CMD_WIELD              14
#define CMD_LOOK               15
#define CMD_SCORE              16
#define CMD_SAY                17
#define CMD_SHOUT              18
#define CMD_TELL               19
#define CMD_INVENTORY          20
#define CMD_QUI                21
#define CMD_PUMMEL             22
#define CMD_INSULT             23
#define CMD_CIRCLE             24
#define CMD_KILL               25
#define CMD_FIREBREATH         26
#define CMD_NEWS               27
#define CMD_UNFOLLOW           28
#define CMD_SETSKILL           29
#define CMD_TIME               30
#define CMD_USEIDEA            31
#define CMD_CORE               32
#define CMD_TYPO               33
#define CMD_WHISPER            34
#define CMD_LEAVE              35
#define CMD_WRITE              36
#define CMD_REZONE             37
#define CMD_HELP               38
#define CMD_WHO                39
#define CMD_EMOTE              40
#define CMD_ECHO               41
#define CMD_STAND              42
#define CMD_SIT                43
#define CMD_REST               44
#define CMD_SLEEP              45
#define CMD_WAKE               46
#define CMD_FORCE              47
#define CMD_TRANSFER           48
#define CMD_LEVELS             49
#define CMD_REROLL             50
#define CMD_BRIEF              51
#define CMD_WIZNET             52
#define CMD_RETURN             53
#define CMD_MOBSWITCH          54
#define CMD_EQUIPMENT          55
#define CMD_BUY                56
#define CMD_SELL               57
#define CMD_VALUE              58
#define CMD_LIST               59
#define CMD_DROP               60
#define CMD_GOTO               61
#define CMD_WEATHER            62
#define CMD_READ               63
#define CMD_POUR               64
#define CMD_GRAB               65
#define CMD_REMOVE             66
#define CMD_PUT                67
#define CMD_USERS              68
#define CMD_SAVE               69
#define CMD_HIT                70
#define CMD_STRING             71
#define CMD_GIVE               72
#define CMD_QUIT               73
#define CMD_STAT               74
#define CMD_POSE               75
#define CMD_NOSHOUT            76
#define CMD_LOAD               77
#define CMD_PURGE              78
#define CMD_SHUTDOWN           79
#define CMD_WIZHELP            80
#define CMD_CREDITS            81
#define CMD_COMPACT            82
#define CMD_GROUPTELL          83
#define CMD_CAST               84
#define CMD_AT                 85
#define CMD_ASK                86
#define CMD_ORDER              87
#define CMD_SIP                88
#define CMD_TASTE              89
#define CMD_SNOOP              90
#define CMD_PUNCH              91
#define CMD_RENT               92
#define CMD_OFFER              93
#define CMD_REPORT             94
#define CMD_ADVANCE            95
#define CMD_TITLE              96
#define CMD_SPLIT              97
#define CMD_FILL               98
#define CMD_OPEN               99
#define CMD_CLOSE              100
#define CMD_LOCK               101
#define CMD_UNLOCK             102
#define CMD_TAP                103
#define CMD_WITHDRAW           104
#define CMD_DEPOSIT            105
#define CMD_BALANCE            106
#define CMD_GLANCE             107
#define CMD_WIMPY              108
#define CMD_GAMEMODE           109
#define CMD_WALKIN             110
#define CMD_WALKOUT            111
#define CMD_KILLER             112
#define CMD_THIEF              113
#define CMD_LECHO              114
#define CMD_WIZLIST            115
#define CMD_ROOMLOCK           116
#define CMD_DEMOTE             117
#define CMD_SETSTAT            118
#define CMD_SESSIONS           119
#define CMD_WIZINV             120
#define CMD_PEEK               121
#define CMD_GOSSIP             122
#define CMD_AUCTION            123
#define CMD_KNOCK              124
#define CMD_SUBDUE             125
#define CMD_RIDE               126
#define CMD_DISMOUNT           127
#define CMD_DISARM             128
#define CMD_TRAP               129
#define CMD_BUTCHER            130
#define CMD_CHANNEL            131
#define CMD_NOSUMMON           132
#define CMD_NOMESSAGE          133
#define CMD_THROW              134
#define CMD_BLOCK              135
#define CMD_SHOOT              136
#define CMD_RELOAD             137
#define CMD_ASSIST             138
#define CMD_FOLLOW             139
#define CMD_AFFECT             140
#define CMD_PUNISH             141
#define CMD_NOGOLD             142
#define CMD_CLASS              143
#define CMD_ALLOWIN            144
#define CMD_WAITLIST           145
#define CMD_BAMFIN             146
#define CMD_BAMFOUT            147
#define CMD_JUNK               148
#define CMD_PASSWORD           149
#define CMD_MAIL               150
#define CMD_FLEE               151
#define CMD_SNEAK              152
#define CMD_HIDE               153
#define CMD_BACKSTAB           154
#define CMD_PICK               155
#define CMD_STEAL              156
#define CMD_BASH               157
#define CMD_RESCUE             158
#define CMD_KICK               159
#define CMD_RELEASE            160
#define CMD_HANDBOOK           161
#define CMD_CLUB               162
#define CMD_WIZINFO            163
#define CMD_PRACTICE           164
#define CMD_YELL               165
#define CMD_EXAMINE            166
#define CMD_TAKE               167
#define CMD_UPTIME             168
#define CMD_REPLY              169
#define CMD_EMOTE2             170
#define CMD_SYSTEM             171
#define CMD_USE                172
#define CMD_WHERE              173
#define CMD_SOS                174
#define CMD_VICIOUS            175
#define CMD_ALERT              176
#define CMD_WHOIS              177
#define CMD_DISPLAY            178
#define CMD_SETFLAG            179
#define CMD_BLEED              180
#define CMD_WIZLOG             181
#define CMD_ASSAULT            182
#define CMD_WORLD              183
#define CMD_SPIN               184
#define CMD_BID                185
#define CMD_COLLECT            186
#define CMD_DONATE             187
#define CMD_WEMOTE             188
#define CMD_WIZACT             189
#define CMD_COLOR              190
#define CMD_CONSIDER           191
#define CMD_GROUP              192
#define CMD_RESTORE            193
#define CMD_QUAFF              194
#define CMD_RECITE             195
#define CMD_SONG               196
#define CMD_REFOLLOW           197
#define CMD_ACTION             198
#define CMD_AMBUSH             199
#define CMD_DESCRIPT           200
#define CMD_VOTE               201
#define CMD_OSTAT              202
#define CMD_RANKFILE           203
#define CMD_DOAS               204
#define CMD_MSTAT              205
#define CMD_FREE               206 /* For Stables - Ranger April 96 */
#define CMD_PLOCK              207 /* Ranger Aug 99*/
#define CMD_DEPUTIZE           208 /* Ranger Sept 99 */
#define CMD_SCOREBOARD         209 /* Ranger Oct 99*/
#define CMD_RIP                210 /* Ranger Feb 29,2000*/
#define CMD_IDENTIFY           211 /* To identify in auction - Ranger May 96 */
#define CMD_ZLIST              212 /* List all loaded zones - Ranger June 96 */
#define CMD_QUEST              213 /* These two are for IMM+ run quests */
#define CMD_QFUNCT             214 /* Ranger - June 96 */
#define CMD_OLCHELP            215
#define CMD_UNKNOWN            216 /* For specs - Ranger Oct 96 */
#define CMD_CRAWL              217 /* next are for new exit types */
#define CMD_JUMP               218 /* Like "jump chasm" */
#define CMD_CLIMB              219
#define CMD_MOVE               220
#define CMD_DISEMBOWEL         221
#define CMD_POST               222
#define CMD_DLIST              223 /* Quack - Dec 96 */
#define CMD_STORE              224 /* Quack - Dec 96 */
#define CMD_RECOVER            225 /* Quack - Dec 96 */
#define CMD_REIMB              226 /* Quack - Jan 97 */
#define CMD_FREEZE             227 /* Ranger - March 2000 */
#define CMD_WARN               228 /* Quack - Jan 97 */
#define CMD_CHAOS              229
#define CMD_HOME               230 /* Ranger - Sep 98*/
#define CMD_WOUND              231
#define CMD_LOGON              232
#define CMD_LOGOFF             233
#define CMD_CLAN               234
#define CMD_BACKFLIP           235
#define CMD_REINDEX            236 /* Ranger - March 2000 */
#define CMD_SNOOPLIST          237 /* Quack - Jan 99 - To list active snoops */
#define CMD_ZRATE              238 /* Ranger - Oct 2000 */
#define CMD_SKIN               239 /* Ranger - Feb 2001 */
#define CMD_RANK               240 /* Ranger - Mar 2001 */
#define CMD_BOARD              241
#define CMD_SOCIAL             242
#define CMD_EMAIL              243 /* Ranger - 19-Dec-03 */
#define CMD_IDNAME             244 /* Ranger - 05-Feb-04 */
#define CMD_CUNNING            245
#define CMD_GF                 246 /* Gossip w/social action */
#define CMD_PRAY               247
#define CMD_COIN_TOSS          248
#define CMD_LOCATE             249

/* Subclasses */
#define CMD_MEDITATE           300
#define CMD_PROTECT            301
#define CMD_SUBCLASS           302
#define CMD_WHIRLWIND          303
#define CMD_DIRTY_TRICKS       304 
#define CMD_BACKFIST           305
#define CMD_AWARENESS          306
#define CMD_BLITZ              307
#define CMD_FLANK              308
#define CMD_TRIP               309
#define CMD_TIGERKICK          310
#define CMD_EVASION            311
#define CMD_CAMP               312
#define CMD_LUNGE              313
#define CMD_314                314
#define CMD_315                315
#define CMD_SMITE              316
#define CMD_HEROES             317
#define CMD_TRUSTY_STEED       318
#define CMD_ZEAL               319
#define CMD_AQUEST             320
#define CMD_DEFEND             321
#define CMD_HOSTILE            322
#define CMD_ASSASSINATE        323
#define CMD_BATTER             324
#define CMD_SHADOWSTEP         325
#define CMD_SNIPE              326
#define CMD_HEADBUTT           327
#define CMD_MANTRA             328
#define CMD_BANZAI             329
#define CMD_BERSERK            330
#define CMD_FRENZY             331
#define CMD_TROPHY             332
#define CMD_SCAN               333
#define CMD_VEHEMENCE          334

#define CMD_HUNT               335
#define CMD_PLRAVG             336

#define CMD_CLOBBER            337
#define CMD_VICTIMIZE          338

#define CMD_NEWOLC             400
#define CMD_NEWOLCM            401
#define CMD_NEWOLCO            402
#define CMD_NEWOLCR            403
#define CMD_NEWOLCZ            404
#define CMD_MOVESTAT           405

#define CMD_NOP                999
