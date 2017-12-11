/* ************************************************************************
*  file: comm.h , Communication module.                   Part of DIKUMUD *
*  Usage: Prototypes for the common functions in comm.c                   *
************************************************************************* */

#ifndef _COMM_H_
#define _COMM_H_

#define COPYOVER_FILE "copyover.dat"

void send_to_outdoor(char *messg);
void close_socket(struct descriptor_data *d);
void send_to_all(char *messg);
void send_to_world(char *arg);
void send_to_world_except(char *arg, struct char_data *ch);
void send_to_char(char *messg, struct char_data *ch);
void send_to_char_by_type(char *messg, struct char_data *ch, int type);
void printf_to_char(struct char_data *ch, char *fmt, ...) __attribute__ ((format(printf, 2,3)));
void send_to_except(char *messg, struct char_data *ch);
void send_to_room(char *messg, int room);
void send_to_room_except(char *messg, int room, struct char_data *ch);
void send_to_room_except_two
        (char *messg, int room, struct char_data *ch1, struct char_data *ch2);
void perform_to_all(char *messg, struct char_data *ch);
void perform_complex(struct char_data *ch1, struct char_data *ch2,
                     struct obj_data *obj1, struct obj_data *obj2,
                     char *mess, byte mess_type, bool hide);

void act(char *str, int hide_invisible, struct char_data *ch,
  void *ob, void *vict_obj, int type);

void act_by_type(char *str, int hide_invisible, struct char_data *ch,
  void *ob, void *vict_obj, int type, int type2);

int signal_room(int room, struct char_data *ch,int cmd,char *arg);
int signal_char(struct char_data *ch, struct char_data *signaler, int cmd, char *arg);
int signal_object(struct obj_data *obj, struct char_data *ch, int cmd, char *arg);
int signal_world(struct char_data *ch, int cmd, char* arg);
int signal_zone(struct char_data *ch, int zone,int cmd, char*arg);


#define TO_ROOM    0
#define TO_VICT    1
#define TO_NOTVICT 2
#define TO_CHAR    3
#define TO_OTHER   4

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
