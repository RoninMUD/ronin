/* ************************************************************************
*  file: comm.h , Communication module.                   Part of DIKUMUD *
*  Usage: Prototypes for the common functions in comm.c                   *
************************************************************************* */

#ifndef _COMM_H_
#define _COMM_H_

#define COPYOVER_FILE "copyover.dat"

#define COMM_COLOR_ENABLED         0
#define COMM_COLOR_FOREGROUND      1
#define COMM_COLOR_PROMPT          2
#define COMM_COLOR_BACKGROUND      13
#define COMM_COLOR_CODE_GREY       8
#define COMM_COLOR_CODE_FIRST      1
#define COMM_COLOR_CODE_LAST       17

void close_socket(struct descriptor_data *d);

void send_to_char_by_type(char *message, struct char_data *ch, int type);
void send_to_char(char *message, struct char_data *ch);
void send_to_group(char *messg, struct char_data *ch, bool same_room);
void send_to_except(char *message, struct char_data *ch);
void send_to_room(char *message, int room);
void send_to_room_except(char *message, int room, struct char_data *ch);
void send_to_room_except_two(char *message, int room, struct char_data *ch1, struct char_data *ch2);
void send_to_world(char *message);
void send_to_world_except(char *message, struct char_data *ch);
void send_to_outdoor(char *message);
void send_to_all(char *message);

void printf_to_char(struct char_data *ch, char *message, ...) __attribute__((format(printf, 2, 3)));
void printf_to_room(int room, char *message, ...) __attribute__((format(printf, 2, 3)));
void printf_to_room_except(int room, struct char_data *ch, char *message, ...) __attribute__((format(printf, 3, 4)));
void printf_to_world(char *message, ...) __attribute__((format(printf, 1, 2)));
void printf_to_all(char *message, ...) __attribute__((format(printf, 1, 2)));

void act_by_type(char *message, int hide, struct char_data *ch, void *other_or_obj, void *vict_or_obj, int type, int type2);
void act(char *message, int hide, struct char_data *ch, void *other_or_obj, void *vict_or_obj, int type);

void perform_to_all(char *message, struct char_data *ch);
void perform_complex(struct char_data *ch1, struct char_data *ch2,
                     struct obj_data *obj1, struct obj_data *obj2,
                     char *mess, byte mess_type, bool hide);



int signal_world(struct char_data *signaler, int cmd, char *arg);
int signal_zone(int zone_rnum, struct char_data *signaler, int cmd, char *arg);
int signal_room(int room_rnum, struct char_data *signaler, int cmd, char *arg);
int signal_char(struct char_data *ch, struct char_data *signaler, int cmd, char *arg);
int signal_object(struct obj_data *obj, struct char_data *signaler, int cmd, char *arg);


#define TO_ROOM    0
#define TO_VICT    1
#define TO_NOTVICT 2
#define TO_CHAR    3
#define TO_OTHER   4
#define TO_GROUP   5

#define COMM_ACT_HIDE_NONE     0
#define COMM_ACT_HIDE_INVIS    1
#define COMM_ACT_HIDE_SUPERBRF 2

int write_to_descriptor(int desc, char *txt);
void write_to_q(char *txt, struct txt_q *queue);
#define SEND_TO_Q(messg, desc)  write_to_q((messg), &(desc)->output)

extern struct descriptor_data *descriptor_list;
extern int uptime;

extern int tics;

extern int TOKENCOUNT;
extern int REBOOT_AT;
extern int avail_descs;
extern int maxdesc;
extern int disablereboot;
extern int reboot_type;
extern int cleanshutdown;
extern int cleanreboot;
extern int chreboot;
extern int no_specials;
extern int noroomdesc;
extern int noextradesc;
extern int slow_death;
extern char CREATEIMP[];

#endif /* _COMM_H_ */
