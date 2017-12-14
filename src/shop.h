/* shop.h
**
** Declares the Shop module
*/

#ifndef _SHOP_H_
#define _SHOP_H_

#define SHOP_FILE "tinyworld.shp"
#define MAX_TRADE 5
#define MAX_PROD 5

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

extern struct shop_data *shop_index;
extern int number_of_shops;

#endif /* _SHOP_H_ */
