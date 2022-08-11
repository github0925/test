#include <stdio.h>
#include <app.h>

int hello_world(void)
{
    while(1)
        printf("Hello World!\n");

    return 0;
}

APP_START(hello_world_sample)
 .entry = (app_entry)hello_world,
APP_END