
/***************************************************************************************
 * Copyright (c) 2014-2022 Zihao Yu, Nanjing University
 *
 * NEMU is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2.
 * You may obtain a copy of Mulan PSL v2 at:
 *          http://license.coscl.org.cn/MulanPSL2
 *
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 *
 * See the Mulan PSL v2 for more details.
 ***************************************************************************************/

#include "sdb.h"

#define NR_WP 32

typedef struct watchpoint
{
    int NO;
    struct watchpoint *next;

    /* TODO: Add more members if necessary */
    int use;
    //char * e;
    char E[100];
    // char * e2;
    int v;
    //int v2;

} WP;

static WP wp_pool[NR_WP] = {};
static WP *head = NULL, *free_ = NULL;


void init_wp_pool();
bool check_all_wp();
WP *new_wp(char *e, int v);
void free_wp(WP *wp);
void watchpoint_info();
void watchpoint_w(char * expr, int num_from_sdb);
void watchpoint_d(int num_wp);
bool compare_v(int v_idx);
int check_use(int idx_wp);

bool compare_v(int v_idx){
	bool su = false;
	if(wp_pool[v_idx].v != expr(wp_pool[v_idx].E, &su)) {
		assert(su == true);
		wp_pool[v_idx].v = expr(wp_pool[v_idx].E, &su);
		assert(su == true);
		return false;
	}
	return true;
}

int check_use(int idx_wp){
	return wp_pool[idx_wp].use;
}

bool check_all_wp(){
for(int i = 0; i < NR_WP; i ++){
  	if(check_use(i) == 1){
  		if(compare_v(i) == false){
  			return true;
  		}
  		else {
  			;
  			//printf("v1 equal to v2\n");
  			//return false;
  		}
  	}
  }
  
  return false;
}


void watchpoint_w(char * exprs, int num_from_sdb){
	WP * new_WP = new_wp(exprs, num_from_sdb);
	assert(new_WP != NULL);
	return ;
}

void watchpoint_d(int num_wp){
	free_wp( &wp_pool[num_wp]);
	return ;
}

void watchpoint_info(){
	WP*tmp = head;
	if(tmp == NULL){
		printf("empty..\n");
		return ;
	}
	else{
		while(tmp != NULL){
			printf("NO: %d\texprssion: %s\tvalue: %d\n", tmp->NO, tmp->E, tmp->v);
			tmp = tmp->next;
			}
	}
	return ;
}

bool init = false;
WP *new_wp(char *e, int v)
{
    if(init == false){
    	init_wp_pool();
    	init = true;
    }
    if(free_ == NULL){
	    printf("No space to malloc..\n");
	    assert(0);
	    return NULL;
    }
    WP *tmp = NULL;
    tmp = free_;
    free_ = free_->next;
    tmp->v = v;    
    //tmp->e = e;
    strcpy(tmp->E, e);
    tmp->use = 1;
    tmp->next = NULL;
    if(head == NULL) {
      head = tmp;
    }
    else{
    	tmp->next = head;
    	head = tmp;
    }
    //tmp->v = v;
    //tmp->e = e;
    printf("successfully create a new WP pointer.\n");
    return tmp;
} 



void free_wp(WP *wp)
{
    WP * tmp = head;
    bool flag = false;
    if(head == NULL){
    	printf("head is empty..\n");
    	//assert(0);
    }
    else if(tmp->NO == wp->NO){
    	tmp->use = 0;
    	strcpy(tmp->E, " ");
    	tmp->v = 0;
    	head = head->next;
    	flag = true;
    }
    else{
    	WP * pre = tmp; // head
    	for(; tmp != NULL; tmp = tmp->next){
    		if(tmp->NO == wp->NO){
	    		pre->next = tmp->next;
	    		tmp->use = 0;
	    		strcpy(tmp->E, "  ");
	    		tmp->v = 0;
	    		flag = true;
	    		break;
    			
    		}
    		pre = tmp;
    	}
    }
    
    if(flag == true){
    	if(free_ == NULL){
    	  free_ = tmp;
     	}
    	 else{
      	   tmp->next = free_;
	   free_ = tmp;
    	 }
    	 printf("delete successfully !!\n");
    	 return ;
    }
    
     printf("no pointer to delete..\n");
     return ;	
}

void init_wp_pool()
{
    int i;
    for (i = 0; i < NR_WP; i++)
    {
        wp_pool[i].NO = i;
        wp_pool[i].next = (i == NR_WP - 1 ? NULL : &wp_pool[i + 1]);
        wp_pool[i].use = 0;
        //wp_pool[i].E = "777";
    }

    head = NULL;
    free_ = wp_pool;
}

/* TODO: Implement the functionality of watchpoint */

