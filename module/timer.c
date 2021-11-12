#include <linux/kernel.h> 
#include <linux/module.h> 
#include <linux/time.h> 
#include <linux/spinlock.h>
#include <linux/vmalloc.h>
#include <linux/fs.h>
#include <linux/slab.h>
#define MODULE_NAME "TIMER" 
#define TIME_CHECK (1 * HZ ) 
 
typedef struct{
	struct timer_list timer;
	bool touch_check;
} __attribute__((packed)) KTIMER_STRUCT;

static KTIMER_STRUCT *timeptr = NULL;

static int count = 0;
static int mod_time = 1;

void kerneltimer_handler(unsigned long arg);
void ktimeover_handler(unsigned long arg);

void kerneltimer_registertimer(KTIMER_STRUCT *time, unsigned long timecheck);

void kerneltimer_handler(unsigned long arg){
	KTIMER_STRUCT *pdata = NULL;
	if( arg ){
		pdata = ( KTIMER_STRUCT *) arg;			
		//printk("\t%d",TIME_CHECK);
		count+=1;
		kerneltimer_registertimer(pdata, mod_time);
	}
}

void kerneltimer_registertimer(KTIMER_STRUCT *time, unsigned long timecheck){
	init_timer( &(time->timer));
	time->timer.expires	= timecheck;
	time->timer.data = (unsigned long) time;
	if(count <3000){
		time->timer.function = kerneltimer_handler;
	}else{
		time->timer.function = ktimeover_handler;
	}
	add_timer( &(time->timer));
	//mod_time -= 1;
	//mod_timer(&(time->timer), mod_time);
	printk("%d expire settings devide count%d\n",time->timer.expires,count);
}


void ktimeover_handler(unsigned long arg){

	
}
int kerneltimer_init(void){
	timeptr = kmalloc( sizeof( KTIMER_STRUCT), GFP_KERNEL);
	if( timeptr == NULL ) return -ENOMEM;
	memset( timeptr, 0, sizeof( KTIMER_STRUCT));
	timeptr-> touch_check = 0;
	kerneltimer_registertimer( timeptr, mod_time);
	return 0;
}

void kerneltimer_exit(void){
	if( timeptr != NULL){
		del_timer( &(timeptr->timer));
		kfree( timeptr);
	}
}

module_init(kerneltimer_init);
module_exit(kerneltimer_exit);
MODULE_LICENSE("GPL");
