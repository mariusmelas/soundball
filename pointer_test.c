#include <stdio.h>

/*
    Learning how to have functions as a member in a struct.
*/
typedef void (*mypfunc_type)();
typedef struct user_data {
    int *counter;
    mypfunc_type pfunc;

} user_data;

/*
    mypfunc and mypfunc2 will be added as members to the struct.
*/    
void mypfunc(int time) {
    printf("time is %d!\n", time);
}

void mypfunc2(int time) {
    printf("time is now %d\n", time*2);
}

/*
    mycallfunc takes a pointer to the struct as argument
    and will call the function that the struct contains.

*/
void mycallfunc(void *data) {
    
    user_data *userdata = data;
    printf("My call func\n");

    int *i = userdata->counter;
    (*userdata).pfunc(60);
    *((*userdata).counter) += 5;

    printf("\tcounter from mycallfunck: %d\n", *((*userdata).counter));
    printf("i is %d\n", *i);

}

int main() {

    int i = 0;
    struct user_data ud; 

    ud.counter = &i;
    ud.pfunc = mypfunc2;

    // Call mycallfunc with pointer to ud as argument.
    mycallfunc(&ud);
    
    // Observe that i has changed.
    printf("i is now: %d\n", i);
}