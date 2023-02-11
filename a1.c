#include "a1.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
	Add your functions to this file.
	Make sure to review a1.h.
	Do NOT include a main() function in this file
	when you submit.
*/

/*
	Intializers to "build" all the
	parts of our restaurant
*/
Restaurant* initialize_restaurant(char* name){
  
	Restaurant *r = (Restaurant *)malloc(sizeof(Restaurant));
	
	char * n = (char *)malloc(sizeof(char) * (strlen(name) + 1));
	strcpy(n, name);
	r->name = n;
  
	r->num_completed_orders = 0;
	r->num_pending_orders = 0;
  
  Queue *q = (Queue *)malloc(sizeof(Queue));
  q->head = NULL;
	q->tail = NULL;
	r->pending_orders = q;

	r->menu = load_menu(MENU_FNAME);
	
	return r;
  
}

char* get_substring(char* original, char* str, size_t start, size_t end){
	return strncpy(original, str+start, end);
}

int count_leading_spaces(char *str){
  int i = 0;
  int count = 0;
  while(i<strlen(str)){
    if(str[i] == ' '){
      count++;
    }
    else{
      break;
    }
    i++;
  }
  return count;
}

// ending Null
Menu* load_menu(char* fname){
	Menu* m = (Menu *)calloc(1,sizeof(Menu));

	FILE* fp = fopen(fname, "r");
	char* line = NULL;
  size_t line_len = 0;

  int count = 0;
  while(getline(&line, &line_len, fp)!=-1){
    count++;
  }
  free(line);

  fclose(fp);
  (*m).num_items = count;

  (*m).item_cost_per_unit = (double *)calloc(count, sizeof(double));
	(*m).item_names = (char **)calloc(count, sizeof(char *));
	(*m).item_codes = (char **)calloc(count, sizeof(char *));

  FILE* f = fopen(fname, "r");
	char* l = NULL;
  size_t l_len = 0;
  int i = 0;
	while(getline(&l, &l_len, f)!=-1){
		char* l1 = (char *)calloc(strlen(l)+1, sizeof(char));
		char* l2 = (char *)calloc(strlen(l)+1, sizeof(char));
		char* a = NULL;
		char* n = NULL;
		int c = count_leading_spaces(l);
		for(int i = 0; i<strlen(l); i++){
			if(l[i] == '$'){
				a = get_substring(l1, l, i+1, strlen(l)-i);
				n = get_substring(l2, l, c + ITEM_CODE_LENGTH, i-4-c);
			}
		}
		(*m).item_cost_per_unit[i] = atof(a);

		(*m).item_names[i] = (char *)calloc(MAX_ITEM_NAME_LENGTH, sizeof(char));
		for(int j = 0; j<strlen(n); j++){
			(*m).item_names[i][j] = n[j];
		}

		(*m).item_codes[i] = (char *)calloc(ITEM_CODE_LENGTH, sizeof(char));

    for(int k = 0; k<ITEM_CODE_LENGTH-1; k++){
      (*m).item_codes[i][k] = l[k+c];
    }
    (*m).item_codes[i][ITEM_CODE_LENGTH-1] = '\0';

    i++;
		free(l1);
		free(l2);

  }
  free(l);
  fclose(f);
	return m;

}


Order* build_order(char* items, char* quantities){
	Order* od = (Order *)malloc(sizeof(Order));

	int count = 0;
	for(int i = 0; i<strlen(quantities); i++){
		if(quantities[i] == *MENU_DELIM){
			count++;
		}
	}
	(*od).num_items = count+1;
	
	(*od).item_codes = (char **)malloc(sizeof(char *)*(count+1));
	for(int j = 0; j<strlen(items); j+=(ITEM_CODE_LENGTH-1)){
		*((*od).item_codes+j/(ITEM_CODE_LENGTH-1)) = (char *)malloc(sizeof(char)*(ITEM_CODE_LENGTH));
    for(int k = 0; k<ITEM_CODE_LENGTH-1; k++){
      (*((*od).item_codes+j/(ITEM_CODE_LENGTH-1)))[k] = items[j+k];
    }
    (*((*od).item_codes+j/(ITEM_CODE_LENGTH-1)))[ITEM_CODE_LENGTH-1] = '\0';
	}
  

	(*od).item_quantities = (int *)malloc(sizeof(int)*(count+1));
  
	char * token = strtok(quantities, MENU_DELIM);
	int i = 0;
	while(token != NULL){
		(*od).item_quantities[i] = atoi(token);
		token = strtok(NULL, MENU_DELIM);
    i++;
	}
  
	return od;

}

/*
	Managing our order queue
*/

void enqueue_order(Order* order, Restaurant* restaurant){
	QueueNode* qn = (QueueNode *)malloc(sizeof(QueueNode));
	(*qn).order = order;
  (*qn).next = NULL;
	if((*(*restaurant).pending_orders).head == NULL){
		(*(*restaurant).pending_orders).head = (*(*restaurant).pending_orders).tail = qn;
	}
  else{
  	(*(*(*restaurant).pending_orders).tail).next = qn;
  	(*(*restaurant).pending_orders).tail = qn;
  }
  (*restaurant).num_pending_orders++;
}

Order* dequeue_order(Restaurant* restaurant){
	Order* o = (*(*(*restaurant).pending_orders).head).order;
	QueueNode* new = (*(*restaurant).pending_orders).head;
	(*(*restaurant).pending_orders).head = (*(*(*restaurant).pending_orders).head).next;
	free(new);
	(*restaurant).num_completed_orders++;
	(*restaurant).num_pending_orders--;
	return o;
}

/*
	Getting information about our orders and order status
*/
double get_item_cost(char* item_code, Menu* menu){
	double n;
	for(int i = 0; i<(*menu).num_items; i++){
		if(*((*menu).item_codes[i]) == *item_code){
			n = (*menu).item_cost_per_unit[i];
		}
	}
	return n;
}

double get_order_subtotal(Order* order, Menu* menu){
	double sum = 0;
	for(int i = 0; i<(*order).num_items; i++){
		sum += get_item_cost((*order).item_codes[i], menu) * (*order).item_quantities[i];
	}
	return sum;
}

double get_order_total(Order* order, Menu* menu){
	return get_order_subtotal(order, menu)*(100+TAX_RATE)/100;
}

int get_num_completed_orders(Restaurant* restaurant){
	return (*restaurant).num_completed_orders;
}
int get_num_pending_orders(Restaurant* restaurant){
	return (*restaurant).num_pending_orders;
}

/*
	Closing down and deallocating memory
*/
void clear_order(Order** order){
  if(*order == NULL){
    return;
  }
  for(int i = 0; i<(*order)->num_items; i++){
    free((*order)->item_codes[i]);
  }
  free((*order)->item_codes);
  free((*order)->item_quantities);
  free(*order);
  *order = NULL; // don't have to free this, set it to NULL because you already freed the memories inside it
}

void clear_menu(Menu** menu){

 //  Menu *clear = *menu;
	// if (*menu == NULL)
	// {
	// 	return;
	// }
	// for (int i = 0; i < (clear->num_items); i++)
	// {
	// 	free((clear->item_codes)[i]);
	// 	free((clear->item_names)[i]);
	// }

	// free(clear->item_cost_per_unit);
	// free(clear->item_codes);
	// free(clear->item_names);
	// free(clear);
	// *menu = NULL;
  
	if(*menu == NULL){
		return;
	}
	
	for(int i = 0; i<(*menu)->num_items; i++){
		free((*menu)->item_names[i]);
		free((*menu)->item_codes[i]);
	}
	free((*menu)->item_names);
	free((*menu)->item_codes);
	free((*menu)->item_cost_per_unit);
  free(*menu);
	*menu = NULL;
  
}

void close_restaurant(Restaurant** restaurant){
	Restaurant *clear = *restaurant;
	clear_menu(&(clear->menu));

	QueueNode *new_head = clear->pending_orders->head;
	QueueNode *old_head = clear->pending_orders->head;

	while (new_head != NULL)
	{
		old_head = new_head;
		new_head = new_head->next;
		old_head->next = NULL;

		clear_order(&(old_head->order));
		free(old_head);
	}
	free(clear -> pending_orders);

	free(clear -> name);
	free(clear);
	*restaurant = NULL;
  
	// free((*restaurant)->name);
	// clear_menu(&(*restaurant)->menu);
	
	// QueueNode * new = (*restaurant)->pending_orders->head;
	// QueueNode * old = (*restaurant)->pending_orders->head;

	// while(new != NULL){
	// 	old = new;
	// 	new = new->next;
 //    old->next = NULL;
	// 	clear_order(&old->order);
	// 	free(old);
	// }
	// free((*restaurant)->pending_orders);
	// free(*restaurant);
	// *restaurant = NULL;
	
}

void print_menu(Menu* menu){
	fprintf(stdout, "--- Menu ---\n");
	for (int i = 0; i < menu->num_items; i++){
		fprintf(stdout, "(%s) %s: %.2f\n", 
			(*menu).item_codes[i], 
			(*menu).item_names[i], 
			(*menu).item_cost_per_unit[i]	
		);
	}
}


void print_order(Order* order){
	for (int i = 0; i < order->num_items; i++){
		fprintf(
			stdout, 
			"%d x (%s)\n", 
			order->item_quantities[i], 
			order->item_codes[i]
		);
	}
}


void print_receipt(Order* order, Menu* menu){
	for (int i = 0; i < order->num_items; i++){
		double item_cost = get_item_cost(order->item_codes[i], menu);
		fprintf(
			stdout, 
			"%d x (%s)\n @$%.2f ea \t %.2f\n", 
			order->item_quantities[i],
			order->item_codes[i], 
			item_cost,
			item_cost * order->item_quantities[i]
		);
	}
	double order_subtotal = get_order_subtotal(order, menu);
	double order_total = get_order_total(order, menu);
	
	fprintf(stdout, "Subtotal: \t %.2f\n", order_subtotal);
	fprintf(stdout, "               -------\n");
	fprintf(stdout, "Tax %d%%: \t$%.2f\n", TAX_RATE, order_total);
	fprintf(stdout, "              ========\n");
}