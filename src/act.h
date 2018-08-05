/* ************************************************************************
*  file: act.h , Implementation of commands.              Part of DIKUMUD *
*  Usage : Communication.                                                 *
*  Copyright (C) 1990, 1991 - see 'license.doc' for complete information. *
************************************************************************* */

/*
$Author: void $
$Date: 2004/11/17 19:21:52 $
$Header: /home/ronin/cvs/ronin/act.h,v 2.3 2004/11/17 19:21:52 void Exp $
$Id: act.h,v 2.3 2004/11/17 19:21:52 void Exp $
$Name:  $
$Log: act.h,v $
Revision 2.3  2004/11/17 19:21:52  void
Added Nomad Skill Cover (1/2 Damage -10 Hitroll)

Revision 2.2  2004/06/02 13:39:21  ronin
Added zmult.

Revision 2.1  2004/03/13 05:27:14  pyro
updated for olc commands oname and owear

Revision 2.0.0.1  2004/02/05 16:13:01  ronin
Reinitialization of cvs archives


Revision 19-Dec-03 Ranger
Addition of do_email

Revision 22-May-03 Ranger
Addition of do_hunt

Revison 10-Mar-03 Ranger
Addition of do_social

Revision 1.2  2002/03/31 07:42:14  ronin
Addition of header lines.

$State: Exp $
*/

/* Subclass skill commands */
void do_awareness(CHAR *ch, char *argument, int cmd);
void do_victimize(CHAR *ch, char *argument, int cmd);
void do_clobber(CHAR *ch, char *argument, int cmd);
void do_throatstrike(CHAR *ch, char *argument, int cmd);
/*void do_execute(CHAR *ch, char *argument, int cmd);*/
void do_defend(CHAR *ch, char *argument, int cmd);
void do_hostile(CHAR *ch, char *argument, int cmd);
void do_zeal(CHAR *ch, char *argument, int cmd);
/*void do_impair(CHAR *ch, char *argument, int cmd);*/
void do_vehemence(CHAR *ch, char *argument, int cmd);
void do_meditate(CHAR *ch, char *argument, int cmd);
void do_protect(CHAR *ch, char *argument, int cmd);
void do_whirlwind(CHAR *ch, char *argument, int cmd);
/*void do_retreat(CHAR *ch, char *argument, int cmd);*/
void do_backfist(CHAR *ch, char *argument, int cmd);
void do_mantra(CHAR *ch, char *argument, int cmd);
void do_banzai(CHAR *ch, char *argument, int cmd);
void do_pray(CHAR *ch, char *argument, int cmd);
void do_trusty_steed(CHAR *ch, char *argument, int cmd);
/*void do_bandage(CHAR *ch, char *argument, int cmd);*/
/*void do_evade(CHAR *ch, char *argument, int cmd);*/
void do_trip(CHAR *ch, char *argument, int cmd);
void do_dirty_tricks(CHAR *ch, char *argument, int cmd);
void do_tigerkick(CHAR *ch, char *argument, int cmd);
void do_scan(CHAR *ch, char *argument, int cmd);
void do_camp(CHAR *ch, char *argument, int cmd);
void do_blitz(CHAR *ch, char *argument, int cmd);
void do_lunge(CHAR *ch, char *argument, int cmd);
void do_switch(CHAR *ch, char *argument, int cmd);
void do_smite(CHAR *ch, char *argument, int cmd);
void do_fade(CHAR *ch, char *argument, int cmd);
void do_flank(CHAR *ch, char *argument, int cmd);
void do_assassinate(CHAR *ch, char *argument, int cmd);
void do_batter(CHAR *ch, char *argument, int cmd);
void do_trophy(CHAR *ch, char *argument, int cmd);
void do_shadowstep(CHAR *ch, char *argument, int cmd);
void do_snipe(CHAR *ch, char *argument, int cmd);
void do_headbutt(CHAR *ch, char *argument, int cmd);
void do_frenzy(CHAR *ch, char *argument, int cmd);
void do_berserk(CHAR *ch, char *argument, int cmd);
void do_evasion(CHAR *ch, char *argument, int cmd);
/* End Subclass skill commands */

/* New olc commands */
void do_rlook(CHAR *ch, char *argument, int cmd);
void do_dreset(CHAR *ch, char *argument, int cmd);
void do_mattack(CHAR *ch, char *argument, int cmd);
void do_mresist(CHAR *ch, char *argument, int cmd);
void do_attdel(CHAR *ch, char *argument, int cmd);
void do_zone(CHAR *ch, char *argument, int cmd);
void do_zname(CHAR *ch, char *argument, int cmd);
void do_zfind(CHAR *ch, char *argument, int cmd);
void do_zcreators(CHAR *ch, char *argument, int cmd);
void do_zpurge(CHAR *ch, char *argument, int cmd);
void do_zmult(CHAR *ch, char *argument, int cmd);
void do_zlife(CHAR *ch, char *argument, int cmd);
void do_zmode(CHAR *ch, char *argument, int cmd);
void do_zrange(CHAR *ch, char *argument, int cmd);
void do_zreset(CHAR *ch, char *argument, int cmd);
void do_cmddel(CHAR *ch, char *argument, int cmd);
void do_cmdlist(CHAR *ch, char *argument, int cmd);
void do_mput(CHAR *ch, char *argument, int cmd);
void do_mtag(CHAR *ch, char *argument, int cmd);
void do_oput(CHAR *ch, char *argument, int cmd);
void do_otake(CHAR *ch, char *argument, int cmd);
void do_ogive(CHAR *ch, char *argument, int cmd);
void do_oequip(CHAR *ch, char *argument, int cmd);
void do_oinsert(CHAR *ch, char *argument, int cmd);
void do_mfollow(CHAR *ch, char *argument, int cmd);
void do_mride(CHAR *ch, char *argument, int cmd);
void do_rextra(CHAR *ch, char *argument, int cmd);
void do_exitdesc(CHAR *ch, char *argument, int cmd);
void do_exitname(CHAR *ch, char *argument, int cmd);
void do_exittype(CHAR *ch, char *argument, int cmd);
void do_exitkey(CHAR *ch, char *argument, int cmd);
void do_maff(CHAR *ch, char *argument, int cmd);
void do_mact(CHAR *ch, char *argument, int cmd);
void do_mimmune(CHAR *ch, char *argument, int cmd);
void do_mpos(CHAR *ch, char *argument, int cmd);
void do_mdefpos(CHAR *ch, char *argument, int cmd);
void do_mreset(CHAR *ch, char *argument, int cmd);
void do_mclass(CHAR *ch, char *argument, int cmd);
void do_mdamage(CHAR *ch, char *argument, int cmd);
void do_mhps(CHAR *ch, char *argument, int cmd);
void do_mmana(CHAR *ch, char *argument, int cmd);
void do_rsect(CHAR *ch, char *argument, int cmd);
void do_rname(CHAR *ch, char *argument, int cmd);
void do_mname(CHAR *ch, char *argument, int cmd);
void do_msex(CHAR *ch, char *argument, int cmd);
void do_mshort(CHAR *ch, char *argument, int cmd);
void do_mlevel(CHAR *ch, char *argument, int cmd);
void do_mskin(CHAR *ch, char *argument, int cmd);
void do_mhitroll(CHAR *ch, char *argument, int cmd);
void do_marmor(CHAR *ch, char *argument, int cmd);
void do_malign(CHAR *ch, char *argument, int cmd);
void do_mgold(CHAR *ch, char *argument, int cmd);
void do_mexp(CHAR *ch, char *argument, int cmd);
void do_mlevel(CHAR *ch, char *argument, int cmd);
void do_mlong(CHAR *ch, char *argument, int cmd);
void do_mfull(CHAR *ch, char *argument, int cmd);
void do_rdesc(CHAR *ch, char *argument, int cmd);
void do_rflag(CHAR *ch, char *argument, int cmd);
void do_rlink(CHAR *ch, char *argument, int cmd);
void do_dlink(CHAR *ch, char *argument, int cmd);
void do_runlink(CHAR *ch, char *argument, int cmd);
void do_dunlink(CHAR *ch, char *argument, int cmd);
void do_rdelete(CHAR *ch, char *argument, int cmd);
void do_mdelete(CHAR *ch, char *argument, int cmd);
void do_odelete(CHAR *ch, char *argument, int cmd);
void do_sdelete(CHAR *ch, char *argument, int cmd);

void do_oname(CHAR *ch, char *argument, int cmd);
void do_oshort(CHAR *ch, char *argument, int cmd);
void do_olong(CHAR *ch, char *argument, int cmd);
void do_oaction(CHAR *ch, char *argument, int cmd);
void do_oaction_nt(CHAR *ch, char *argument, int cmd);
void do_ochar_wear_desc(CHAR *ch, char *argument, int cmd);
void do_oroom_wear_desc(CHAR *ch, char *argument, int cmd);
void do_ochar_rem_desc(CHAR *ch, char *argument, int cmd);
void do_oroom_rem_desc(CHAR *ch, char *argument, int cmd);
void do_oextra(CHAR *ch, char *argument, int cmd);
void do_otype(CHAR *ch, char *argument, int cmd);
void do_owear(CHAR *ch, char *argument, int cmd);
void do_osubclass_res(CHAR *ch, char *argument, int cmd);
void do_ovalues(CHAR *ch, char *argument, int cmd);
void do_oflags(CHAR *ch, char *argument, int cmd);
void do_obitvect(CHAR *ch, char *argument, int cmd);
void do_oaffects(CHAR *ch, char *argument, int cmd);
void do_otimer(CHAR *ch, char *argument, int cmd);
void do_oweight(CHAR *ch, char *argument, int cmd);
void do_orent(CHAR *ch, char *argument, int cmd);
void do_ocost(CHAR *ch, char *argument, int cmd);
void do_orepop(CHAR *ch, char *argument, int cmd);
void do_omaterial(CHAR *ch, char *argument, int cmd);
void do_oreset(CHAR *ch, char *argument, int cmd);

void do_board(CHAR *ch, char *argument, int cmd);
void do_social(CHAR *ch, char *argument, int cmd);
void do_scoreboard(CHAR *ch, char *argument, int cmd);
void do_skin(CHAR *ch, char *argument, int cmd);
void do_aquest(CHAR *ch, char *argument, int cmd);
void do_zrate(CHAR *ch, char *argument, int cmd);
void do_freeze(CHAR *ch, char *argument, int cmd);
void do_reindex(CHAR *ch, char *argument, int cmd);
void do_rip(CHAR *ch, char *argument, int cmd);
void do_deputize(CHAR *ch, char *argument, int cmd);
void do_plock(CHAR *ch, char *argument, int cmd);
void do_subclass(CHAR *ch, char *argument, int cmd);
void do_doas(CHAR *ch, char *argument, int cmd);
void do_rankfile(CHAR *ch, char *argument, int cmd);
void do_rank(CHAR *ch, char *argument, int cmd);
void do_core(CHAR *ch, char *argument, int cmd);
void do_rezone(CHAR *ch, char *argument, int cmd);
void do_home(CHAR *ch, char *argument, int cmd);
void do_bleed(CHAR *ch, char *argument, int cmd);
void do_vicious(CHAR *ch, char *argument, int cmd);
void do_reply(CHAR *ch, char *argument, int cmd);
void do_walkin(CHAR *ch, char *argument, int cmd);
void do_walkout(CHAR *ch, char *argument, int cmd);
void do_backflip(CHAR *ch, char *argument, int cmd);
void do_clan(CHAR *ch, char *argument, int cmd);
void do_logon_char(CHAR *ch, char *argument, int cmd);
void do_logoff_char(CHAR *ch, char *argument, int cmd);
void do_warn(CHAR *ch, char *argument, int cmd);
void read_dlist(CHAR *ch, char *argument,int cmd);
void do_reimb(CHAR *ch, char *argument, int cmd);
void do_zbrief(CHAR *ch, char *argument, int cmd); /* Brief list of stats of things in a zone - Quack Jan 97 */
void do_wound(CHAR *ch, char *argument, int cmd); /* wound NPC */
void do_disembowel(CHAR *ch, char *argument, int cmd);
void do_move_keyword(CHAR *ch, char *argument, int cmd);
void do_jump(CHAR *ch, char *argument, int cmd);
void do_climb(CHAR *ch, char *argument, int cmd);
void do_crawl(CHAR *ch, char *argument, int cmd);
void do_olchelp(CHAR *ch, char *argument, int cmd);
void do_qfunction(CHAR *ch, char *argument, int cmd); /* These two are for IMM+ run quests */
void do_quest(CHAR *ch, char *argument, int cmd);     /* Ranger June 96 */
void do_zlist(CHAR *ch, char *argument, int cmd); /* List all loaded zones - Ranger June 96 */
void auction_identify(CHAR *ch, char *arg, int cmd); /* Identify in Auction - Ranger May 96 */
void do_oclone(CHAR *ch, char *argument, int cmd); /* Clone an obj - Ranger May 97 */
void do_mclone(CHAR *ch, char *argument, int cmd); /* Clone a mob - Ranger May 96 */
void do_rclone(CHAR *ch, char *argument, int cmd); /* Clone a room - Ranger May 96 */
void do_zshow(CHAR *ch, char *argument, int cmd); /* List things in a zone - Ranger May 96 */
void do_rshow(CHAR *ch, char *argument, int cmd); /* List rooms by name - Ranger May 96 */
void do_ostat(CHAR *ch,char *argument,int cmd);
void do_mstat(CHAR *ch,char *argument,int cmd);
void do_create(CHAR *ch,char *argument,int cmd);
void do_edit(CHAR *ch,char *argument,int cmd);
void do_setcolor(CHAR *ch,char *argument,int cmd);
void do_say(CHAR *ch, char *argument, int cmd);
void do_title(CHAR *ch, char *arg, int cmd);
void do_report(CHAR *ch, char *argument, int cmd);
void do_shout(CHAR *ch, char *argument, int cmd);
void do_yell(CHAR *ch, char *argument, int cmd);
void do_gossip(CHAR *ch, char *argument, int cmd);
void do_chaos(CHAR *ch, char *argument, int cmd);
void do_auction(CHAR *ch, char *argument, int cmd);
void do_channel(CHAR *ch, char *argument, int cmd);
void do_tell(CHAR *ch, char *argument, int cmd);
void do_gtell(CHAR *ch, char *argument, int cmd);
void do_whisper(CHAR *ch, char *argument, int cmd);
void do_ask(CHAR *ch, char *argument, int cmd);
void do_write(CHAR *ch, char *argument, int cmd);
void do_save(CHAR *ch, char *argument, int cmd);
void do_wizhelp(CHAR *ch, char *argument, int cmd);
void do_look(CHAR *ch, char *argument, int cmd);
void do_hunt(CHAR *ch, char *argument, int cmd);
void do_playeravg(CHAR *ch, char *argument, int cmd);
void do_email(CHAR *ch, char *argument, int cmd);
void do_idname(CHAR *ch, char *argument, int cmd);
void do_gf(CHAR *ch, char *argument, int cmd);
void do_identify(CHAR *ch, char *arg, int cmd);

void do_special_move(struct char_data *ch, char *arg, int cmd);

/* Utility things */

int do_simple_move(CHAR *ch, int cmd, int spec_check);
void do_move(CHAR *ch, char *argument, int cmd);
void do_peek(CHAR *ch, char *argument, int cmd);
int find_door(CHAR *ch, char *type, char *dir);
void do_open(CHAR *ch, char *argument, int cmd);
void do_close(CHAR *ch, char *argument, int cmd);
int has_key(CHAR *ch, int key);
void do_lock(CHAR *ch, char *argument, int cmd);
void do_unlock(CHAR *ch, char *argument, int cmd);
void do_pick(CHAR *ch, char *argument, int cmd);
void do_knock(CHAR *ch, char *argument, int cmd);
void do_enter(CHAR *ch, char *argument, int cmd);
void do_leave(CHAR *ch, char *argument, int cmd);
void do_stand(CHAR *ch, char *argument, int cmd);
void do_sit(CHAR *ch, char *argument, int cmd);
void do_rest(CHAR *ch, char *argument, int cmd) ;
void do_sleep(CHAR *ch, char *argument, int cmd) ;
void do_wake(CHAR *ch, char *argument, int cmd);
void do_follow(CHAR *ch, char *argument, int cmd);
void do_subdue(CHAR *ch, char *argument, int cmd);
void do_ride(CHAR *ch, char *argument, int cmd);
/* do_free for stable mounts - Ranger April 96 */
void do_free(CHAR *ch, char *argument, int cmd);
void do_dismount(CHAR *ch, char *argument, int cmd);
void do_action(CHAR *ch, char *argument, int cmd);
void do_emote(CHAR *ch, char *argument, int cmd);
void do_drop(CHAR *ch, char *argument, int cmd);
void do_get(CHAR *ch, char *argument, int cmd);
void do_move(CHAR *ch, char *argument, int cmd);
void do_look(CHAR *ch, char *argument, int cmd);
void do_read(CHAR *ch, char *argument, int cmd);
void do_say(CHAR *ch, char *argument, int cmd);
void do_exit(CHAR *ch, char *argument, int cmd);
void do_snoop(CHAR *ch, char *argument, int cmd);
void do_insult(CHAR *ch, char *argument, int cmd);
void do_quit(CHAR *ch, char *argument, int cmd);
void do_qui(CHAR *ch, char *argument, int cmd);
void do_help(CHAR *ch, char *argument, int cmd);
void do_who(CHAR *ch, char *argument, int cmd);
void do_emote(CHAR *ch, char *argument, int cmd);
void do_echo(CHAR *ch, char *argument, int cmd);
void do_trans(CHAR *ch, char *argument, int cmd);
void do_kill(CHAR *ch, char *argument, int cmd);
void do_stand(CHAR *ch, char *argument, int cmd);
void do_sit(CHAR *ch, char *argument, int cmd);
void do_rest(CHAR *ch, char *argument, int cmd);
void do_sleep(CHAR *ch, char *argument, int cmd);
void do_wake(CHAR *ch, char *argument, int cmd);
void do_force(CHAR *ch, char *argument, int cmd);
void do_get(CHAR *ch, char *argument, int cmd);
void do_drop(CHAR *ch, char *argument, int cmd);
void do_motd(CHAR *ch, char *argument, int cmd);
void do_score(CHAR *ch, char *argument, int cmd);
void do_inventory(CHAR *ch, char *argument, int cmd);
void do_equipment(CHAR *ch, char *argument, int cmd);
void do_shout(CHAR *ch, char *argument, int cmd);
void do_not_here(CHAR *ch, char *argument, int cmd);
void do_unknown(CHAR *ch, char *argument, int cmd);
void do_tell(CHAR *ch, char *argument, int cmd);
void do_wear(CHAR *ch, char *argument, int cmd);
void do_wield(CHAR *ch, char *argument, int cmd);
void do_grab(CHAR *ch, char *argument, int cmd);
void do_remove(CHAR *ch, char *argument, int cmd);
void do_put(CHAR *ch, char *argument, int cmd);
void do_shutdown(CHAR *ch, char *argument, int cmd);
void do_save(CHAR *ch, char *argument, int cmd);
void do_hit(CHAR *ch, char *argument, int cmd);
void do_string(CHAR *ch, char *arg, int cmd);
void do_give(CHAR *ch, char *arg, int cmd);
void do_stat(CHAR *ch, char *arg, int cmd);
void do_setskill(CHAR *ch, char *arg, int cmd);
void do_time(CHAR *ch, char *arg, int cmd);
void do_weather(CHAR *ch, char *arg, int cmd);
void do_load(CHAR *ch, char *arg, int cmd);
void do_purge(CHAR *ch, char *arg, int cmd);
void do_useidea(CHAR *ch, char *arg, int cmd);
void do_typo(CHAR *ch, char *arg, int cmd);
/*void do_bug(CHAR *ch, char *arg, int cmd);*/
void do_whisper(CHAR *ch, char *arg, int cmd);
void do_cast(CHAR *ch, char *arg, int cmd);
void do_at(CHAR *ch, char *arg, int cmd);
void do_goto(CHAR *ch, char *arg, int cmd);
void do_ask(CHAR *ch, char *arg, int cmd);
void do_drink(CHAR *ch, char *arg, int cmd);
void do_eat(CHAR *ch, char *arg, int cmd);
void do_pour(CHAR *ch, char *arg, int cmd);
void do_sip(CHAR *ch, char *arg, int cmd);
void do_taste(CHAR *ch, char *arg, int cmd);
void do_order(CHAR *ch, char *arg, int cmd);
void do_follow(CHAR *ch, char *arg, int cmd);
void do_rent(CHAR *ch, char *arg, int cmd);
void do_offer(CHAR *ch, char *arg, int cmd);
void do_advance(CHAR *ch, char *arg, int cmd);
void do_close(CHAR *ch, char *arg, int cmd);
void do_open(CHAR *ch, char *arg, int cmd);
void do_lock(CHAR *ch, char *arg, int cmd);
void do_unlock(CHAR *ch, char *arg, int cmd);
void do_exits(CHAR *ch, char *arg, int cmd);
void do_enter(CHAR *ch, char *arg, int cmd);
void do_leave(CHAR *ch, char *arg, int cmd);
void do_write(CHAR *ch, char *arg, int cmd);
void do_flee(CHAR *ch, char *arg, int cmd);
void do_sneak(CHAR *ch, char *arg, int cmd);
void do_hide(CHAR *ch, char *arg, int cmd);
void do_backstab(CHAR *ch, char *arg, int cmd);
void do_pick(CHAR *ch, char *arg, int cmd);
void do_steal(CHAR *ch, char *arg, int cmd);
void do_bash(CHAR *ch, char *arg, int cmd);
void do_rescue(CHAR *ch, char *arg, int cmd);
void do_kick(CHAR *ch, char *arg, int cmd);
void do_examine(CHAR *ch, char *arg, int cmd);
void do_users(CHAR *ch, char *arg, int cmd);
void do_where(CHAR *ch, char *arg, int cmd);
void do_lstat(CHAR *ch, char *arg, int cmd);
void do_snooplist(CHAR *ch, char *arg, int cmd);
void do_levels(CHAR *ch, char *arg, int cmd);
void do_reroll(CHAR *ch, char *arg, int cmd);
void do_brief(CHAR *ch, char *arg, int cmd);
void do_wizlist(CHAR *ch, char *arg, int cmd);
void do_consider(CHAR *ch, char *arg, int cmd);
void do_group(CHAR *ch, char *arg, int cmd);
void do_restore(CHAR *ch, char *arg, int cmd);
void do_return(CHAR *ch, char *argument, int cmd);
void do_mobswitch(CHAR *ch, char *argument, int cmd);
void do_quaff(CHAR *ch, char *argument, int cmd);
void do_recite(CHAR *ch, char *argument, int cmd);
void do_use(CHAR *ch, char *argument, int cmd);
void do_pose(CHAR *ch, char *argument, int cmd);
void do_noshout(CHAR *ch, char *argument, int cmd);
void do_wizhelp(CHAR *ch, char *argument, int cmd);
void do_credits(CHAR *ch, char *argument, int cmd);
void do_heroes(CHAR *ch, char *argument, int cmd);
void do_compact(CHAR *ch, char *argument, int cmd);
void do_gtell(CHAR *ch, char *argument, int cmd);
void do_report(CHAR *ch, char *argument, int cmd);
void do_title(CHAR *ch, char *argument, int cmd);
void do_split(CHAR *ch, char *argument, int cmd);
void do_fill(CHAR *ch, char *argument, int cmd);
void do_tap(CHAR *ch, char *argument, int cmd);
void do_withdraw(CHAR *ch, char *argument, int cmd);
void do_deposit(CHAR *ch, char *argument, int cmd);
void do_balance(CHAR *ch, char *argument, int cmd);
void do_glance(CHAR *ch, char *argument, int cmd);
void do_wimpy(CHAR *ch, char *argument, int cmd);
void do_nokill(CHAR *ch, char *argument, int cmd);
void do_gamemode(CHAR *ch, char *argument, int cmd);
void do_killer(CHAR *ch, char *argument, int cmd);
void do_thief(CHAR *ch, char *argument, int cmd);
void do_lecho(CHAR *ch, char *argument, int cmd);
void do_wiznet(CHAR *ch, char *argument, int cmd);
void do_roomlock(CHAR *ch, char *argument, int cmd);
void do_demote(CHAR *ch, char *argument, int cmd);
void do_setstat(CHAR *ch, char *argument, int cmd);
void do_session(CHAR *ch, char *argument, int cmd);
void do_wizinv(CHAR *ch, char *argument, int cmd);
void do_peek(CHAR *ch, char *argument, int cmd);
void do_gossip(CHAR *ch, char *argument, int cmd);
void do_auction(CHAR *ch, char *argument, int cmd);
void do_knock(CHAR *ch, char *argument, int cmd);
void do_subdue(CHAR *ch, char *argument, int cmd);
void do_ride(CHAR *ch, char *argument, int cmd);
void do_dismount(CHAR *ch, char *argument, int cmd);
void do_punch(CHAR *ch, char *argument, int cmd);
void do_disarm(CHAR *ch, char *argument, int cmd);
void do_trap(CHAR *ch, char *argument, int cmd);
void do_butcher(CHAR *ch, char *argument, int cmd);
void do_channel(CHAR *ch, char *argument, int cmd);
void do_nosummon(CHAR *ch, char *argument, int cmd);
void do_nomessage(CHAR *ch, char *argument, int cmd);
void do_throw(CHAR *ch, char *argument, int cmd);
void do_block(CHAR *ch, char *argument, int cmd);
void do_shoot(CHAR *ch, char *argument, int cmd);
void do_reload(CHAR *ch, char *argument, int cmd);
void do_assist(CHAR *ch, char *argument, int cmd);
void do_refollow(CHAR *ch, char *argument, int cmd);
void do_unfollow(CHAR *ch, char *argument, int cmd);
void do_affect(CHAR *ch, char *argument, int cmd);
void do_circle(CHAR *ch, char *argument, int cmd);
void do_punish(CHAR *ch, char *argument, int cmd);
void do_nogold(CHAR *ch, char *argument, int cmd);
void do_class(CHAR *ch, char *argument, int cmd);
void do_allowin(CHAR *ch, char *argument, int cmd);
void do_waitlist(CHAR *ch, char *argument, int cmd);
void do_bamfin(CHAR *ch, char *argument, int cmd);
void do_bamfout(CHAR *ch, char *argument, int cmd);
void do_jun(CHAR *ch, char *argument, int cmd);
void do_junk(CHAR *ch, char *argument, int cmd);
void do_password(CHAR *ch, char *argument, int cmd);
void do_mail(CHAR *ch, char *argument, int cmd);
void do_song(CHAR *ch, char *argument, int cmd);
void do_action(CHAR *ch, char *arg, int cmd);
void do_practice(CHAR *ch, char *arg, int cmd);
void do_release(CHAR *ch, char *arg, int cmd);
void do_handbook(CHAR *ch, char *arg, int cmd);
void do_club(CHAR *ch, char *arg, int cmd);
void do_wizinfo(CHAR *ch, char *arg, int cmd);
void do_yell(CHAR *ch, char *arg, int cmd);
void do_uptime(CHAR *ch, char *arg, int cmd);
void do_system(CHAR *ch, char *arg, int cmd);
void do_setobjstat(CHAR *ch, char *arg, int cmd);
void do_alert(CHAR *ch, char *arg, int cmd);
void do_crent(CHAR *ch, char *arg, int cmd);
void do_whois(CHAR *ch, char *arg, int cmd);
void do_display(CHAR *ch, char *arg, int cmd);
void do_setflag(CHAR *ch, char *arg, int cmd);
void do_wizlog(CHAR *ch, char *arg, int cmd);
void do_world(CHAR *ch, char *arg, int cmd);
void do_ambush(CHAR *ch, char *arg, int cmd);
void do_spin_kick(CHAR *ch, char *arg, int cmd);
void do_assault(CHAR *ch, char *arg, int cmd);
void do_pummel(CHAR *ch, char *arg, int cmd);
void do_bid(CHAR *ch, char *arg, int cmd);
void do_wemote(CHAR *ch,char *argument, int cmd);
void do_donate(CHAR *ch,char *argument, int cmd);
void do_wizact(CHAR *ch,char *argument, int cmd);
void do_setcolor(CHAR *ch,char *argument, int cmd);
void do_descr(CHAR *ch,char *argument, int cmd);
bool circle_follow(CHAR *ch, CHAR *victim);
void stop_follower(CHAR *ch);
void die_follower(CHAR *ch);
void add_follower(CHAR *ch, CHAR *leader);
void wear(CHAR *ch, OBJ *obj_object, int keyword);
void do_cunning(CHAR *ch, char *argument, int cmd);
void do_coin_toss(CHAR *ch, char *arg, int cmd);
void do_movestat(CHAR *ch, char *arg, int cmd);

void channel_comm(struct char_data *ch, char *arg, int comm);

#define CHANNEL_COMM_GOSSIP  0
#define CHANNEL_COMM_AUCTION 1
#define CHANNEL_COMM_QUESTC  2
#define CHANNEL_COMM_CHAOS   3

typedef struct channel_t {
  char *channel_name;
  long channel_flag;
  int channel_color;
} channel_t;
