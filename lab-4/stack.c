/*
 * Stack Operations using Dynamic Memory Allocation
 */
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
 
struct node
{
    int32_t data;
    struct node *link;
}*top = NULL;
 
#define MAX 5
 
void push();
void pop();
void stack_size();
 
// to insert elements in stack
void push()
{
    int val,count;
    struct node *temp;
    temp = (struct node*)malloc(sizeof(struct node));
 
    count = st_count();
    if (count <= MAX - 1)
    {
        printf("\nEnter value which you want to push into the stack :\n");
        scanf("%d",&val);
        temp->data = val;
        temp->link = top;
        top = temp;
    }
    else {
        errno = ENOSPC;
	    return 0;
    }
}
 
// to delete elements from stack
void pop()
{
    struct node *temp;
    if (top == NULL){
        errno = ENODEV;
	    return 0;
    }
    else
    {
        temp = top;
        printf("Value popped out is %d \n",temp->data);
        top = top->link;
        free(temp);
    }
}
 
// to count the number of elements
void stack_size()
{
    int count = 0;
    struct node *temp;
 
    temp = top;
    while (temp != NULL)
    {
        printf(" %d\n",temp->data);
        temp = temp->link;
        count++;
    }
    printf("size of stack is %d \n",count);
}
 
int st_count()
{
    int count = 0;
    struct node *temp;
    temp = top;
    while (temp != NULL)
    {
        temp = temp->link;
        count++;
    }
    return count;
}