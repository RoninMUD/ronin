
/*
$Author: ronin $
$Date: 2004/02/05 16:13:21 $
$Header: /home/ronin/cvs/ronin/shop.h,v 2.0.0.1 2004/02/05 16:13:21 ronin Exp $
$Id: shop.h,v 2.0.0.1 2004/02/05 16:13:21 ronin Exp $
$Name:  $
$Log: shop.h,v $
Revision 2.0.0.1  2004/02/05 16:13:21  ronin
Reinitialization of cvs archives

Revision 1.2  2002/03/31 07:42:15  ronin
Addition of header lines.

$State: Exp $
*/

#define SHOP_FILE "tinyworld.shp"
#define MAX_TRADE 5
#define MAX_PROD 5

extern struct str_app_type str_app[];

struct shop_data
{
	int producing[MAX_PROD];/* Which item to produce (virtual)      */
	float profit_buy;       /* Factor to multiply cost with.        */
	float profit_sell;      /* Factor to multiply cost with.        */
	ubyte type[MAX_TRADE];   /* Which item to trade.                 */
	char *no_such_item1;    /* Message if keeper hasn't got an item */
	char *no_such_item2;    /* Message if player hasn't got an item */
	char *missing_cash1;    /* Message if keeper hasn't got cash    */
	char *missing_cash2;    /* Message if player hasn't got cash    */
	char *do_not_buy;			/* If keeper dosn't buy such things. 	*/
	char *message_buy;      /* Message when player buys item        */
	char *message_sell;     /* Message when player sells item       */
	int temper1;           	/* How does keeper react if no money    */
	int temper2;           	/* How does keeper react when attacked  */
	int keeper;             /* The mobil who owns the shop (real)*/
	int with_who;		/* Who does the shop trade with?	*/
	int in_room;		/* Where is the shop?			*/
	int open1,open2;	/* When does the shop open?		*/
	int close1,close2;	/* When does the shop close?		*/
};


extern struct room_data *world;
extern struct time_info_data time_info;

extern struct shop_data *shop_index;
extern int number_of_shops;


