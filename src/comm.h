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

void send_to_char_by_type(char *message, CHAR *ch, int type);
void act_by_type(char *message, int hide, CHAR *ch, void *other_or_obj, void *vict_or_obj, int type, int type2);

//void act(const char *message, int hide, CHAR *ch, void *other_or_obj, void *vict_or_obj, int type, ...) __attribute__((format(printf, 1, 7)));
void act(char *message, int hide, CHAR *ch, void *other_or_obj, void *vict_or_obj, int type);
void act_f(int hide, CHAR *ch, void *other_or_obj, void *vict_or_obj, int type, char *message, ...) __attribute__((format(printf, 6, 7)));

void send_to_char(char *message, CHAR *ch);
void send_to_group(char *messge, CHAR *ch, bool same_room);
void send_to_room(char *message, int room);
void send_to_room_except(char *message, int room, CHAR *ch);
void send_to_room_except_two(char *message, int room, CHAR *ch1, CHAR *ch2);
void send_to_world(char *message);
void send_to_world_except(char *message, CHAR *ch);
void send_to_outdoor(char *message);
void send_to_all(char *message);
void send_to_all_except(char *message, CHAR *ch);

/*
void send_to_char(const char *message, CHAR *ch, ...) __attribute__((format(printf, 1, 3)));
void send_to_group(const char *messge, CHAR *ch, bool same_room, ...) __attribute__((format(printf, 1, 4)));
void send_to_room(const char *message, int room, ...) __attribute__((format(printf, 1, 3)));
void send_to_room_except(const char *message, int room, CHAR *ch, ...) __attribute__((format(printf, 1, 4)));
void send_to_room_except_two(const char *message, int room, CHAR *ch1, CHAR *ch2, ...) __attribute__((format(printf, 1, 5)));
void send_to_world(const char *message, ...) __attribute__((format(printf, 1, 2)));
void send_to_world_except(const char *message, CHAR *ch, ...) __attribute__((format(printf, 1, 3)));
void send_to_outdoor(const char *message, ...) __attribute__((format(printf, 1, 2)));
void send_to_all(const char *message, ...) __attribute__((format(printf, 1, 2)));
void send_to_all_except(const char *message, CHAR *ch, ...) __attribute__((format(printf, 1, 3)));
*/

void printf_to_char(CHAR *ch, char *message, ...) __attribute__((format(printf, 2, 3)));
void printf_to_group(CHAR *ch, bool same_room, char *message, ...) __attribute__((format(printf, 3, 4)));
void printf_to_room(int room, char *message, ...) __attribute__((format(printf, 2, 3)));
void printf_to_room_except(int room, CHAR *ch, char *message, ...) __attribute__((format(printf, 3, 4)));
void printf_to_room_except_two(int room, CHAR *ch1, CHAR *ch2, char *message, ...) __attribute__((format(printf, 4, 5)));
void printf_to_world(char *message, ...) __attribute__((format(printf, 1, 2)));
void printf_to_world_except(CHAR *ch, char *message, ...) __attribute__((format(printf, 2, 3)));
void printf_to_outdoor(char *message, ...) __attribute__((format(printf, 1, 2)));
void printf_to_all(char *message, ...) __attribute__((format(printf, 1, 2)));
void printf_to_all_except(CHAR *ch, char *message, ...) __attribute__((format(printf, 2, 3)));

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

/* A bit of a hack here.*/
#define COMM_ACT_HIDE_NORMAL   0
#define COMM_ACT_HIDE_CANT_SEE 1
#define COMM_ACT_HIDE_SUPERBRF 2
#define COMM_ACT_HIDE_NON_MORT 4 // Overrides CANT_SEE if combined with it.
#define COMM_ACT_HIDE_PRETITLE 8

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
extern int FREEMORT;

#endif /* _COMM_H_ */
