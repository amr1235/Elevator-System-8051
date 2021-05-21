#include "C8051F020.h"
#include "Headers/BIT_MATH.h"
#include "Headers/STD_TYPES.h"

u8 CURRENT_FLOOR = 0; //GROUND
u8 requests[5] = {0};
u8 queue[5] = {0};
u8 index = 0;   // the place in array where the new value will be placed
u8 q_index = 0; // the place in array where the new value will be placed
u8 q_size = 0;

u8 motor_state = 0;      // on or off
u8 motion_direction = 0; // up -> 1  down -> 2


// assume step angle = 45 so it needs 4 steps to complete 1 cycle
// assume the number of steps = 16 step to mave one completed floor
// assume rotating clockwise will make the elevator go up and anti-clockwise will go down
void seven_segment(u8 floor_num)
{
    switch (floor_num)
    {
    case 0:
        P1 = 0x3F;
        break;
    case 1:
        P1 = 0x06;
        break;
    case 2:
        P1 = 0x5b;
        break;
    case 3:
        P1 = 0x4f;
        break;
    case 4:
        P1 = 0x66;
        break;
    }
}

void delay()
{
    unsigned int i;
    for (i = 0; i < 20000; i++)
    {
        continue;
    }
}
const NUMBER_OF_STEPS = 16; // we can change to the desire number

void elevate(unsigned int current_floor, unsigned int destination)
{
    // number of floors needed
    signed int floors_number = destination - current_floor;
    unsigned int i,j;
    unsigned int steps;
    // if (P3 == 0x00)
    // {
    //     return;
    // }
    if (floors_number > 0)
    {
        j = 4;
        // we will go up
        // number of steps to reach the destination
        // 0001 -> 0010 -> 0100 -> 1000 -> 0001 -> ...
        steps = NUMBER_OF_STEPS * (floors_number);
        
        P3 = 0x1f & P3;
        for (i = 1; i <= steps; i++)
        {
            if(i % NUMBER_OF_STEPS == 0)
            {
                CURRENT_FLOOR++;
                seven_segment(CURRENT_FLOOR);
            }

            if (j == 7)
            {
                CLR_BIT(P3, j);
                j = 4;
                SET_BIT(P3, j);  //0001 xxxx j=4
            }
            else
            {
                CLR_BIT(P3, j);
                j++;
                SET_BIT(P3, j);  //0001 xxxx j=4
            }
            //0001 xxxx j=5
            delay();
        }
        P3 = 0x0f & P3;
    }
    else if (floors_number < 0)
    {
        j = 7;
        // we will go down
        // number of steps to reach the destination
        // 1000 -> 0100 -> 0010 -> 0001 -> 1000 -> ...
        steps = NUMBER_OF_STEPS * (floors_number * (-1));
        P3 = 0x0f & P3; // 1000 xxxx
        for (i = 1; i <= steps; i++)
        {

            if(i % NUMBER_OF_STEPS == 0)
            {
                CURRENT_FLOOR--;
                seven_segment(CURRENT_FLOOR);
            }    


            if (j == 4)
            {
                CLR_BIT(P3, j);
                j = 7;
                SET_BIT(P3, j);  //0001 xxxx j=4
            }
            else
            {
                CLR_BIT(P3, j);
                j--;
                SET_BIT(P3, j);  //0001 xxxx j=4
            }
            //0001 xxxx j=5
            delay();
        }
        P3 = 0x0f & P3;
    }
}



// void Delay_MS(u16 ms)
// {
//     u16 i;
//     u16 j;
//     for (i = 0; i < ms; i++)
//     {
//         for (j = 0; j < 120; j++)
//         {
//         }
//     }
// }

// u8 IsMotorOn(void)
// {
//     /* check on global variable "motor_state" */
//     return motor_state;
// }

// /* 1. This function checks for queue requests and implement them in a specific manner.
//  * 2. It accesses the Q requests which acts as the input.
//  * 3. It uses "stepper" function to rotate the motor.
//  */
// void elevator(void)
// {
//     // if going up ignore "going down" request for a while

//     // if at floor that is higher than the one requested, ignore it (moving up)

//     //
// }

//3FH,06H,5BH,4FH,66H,


u8 Is_value_exist(u8 value)
{
    u8 i = 0; // counter
    for (i; i <= 4; i++)
    {
        if (value == requests[i])
        {
            return 1;
        }
    }
    return 0;
}

u8 push_value(u8 value)
{
    if (Is_value_exist(value))
        return 0;

    if (index <= 4)
    {
        requests[index] = value;
        index++;
        return 1;
    }
    else
    {
        u8 counter = 0;
        for (counter; counter <= 4; counter++)
        {
            if (requests[counter] == 0)
            {
                requests[counter] = value;
                return 1;
            }
        }
        return 0;
    }
}
u8 enqueue(u8 value)
{
    if (q_size == 0)
        q_index = 0;
    if (q_index < 5)
    {
        queue[q_index] = value;
        q_index++;
        q_size++;
        return 1;
    }
    else
    {
        return 0;
    }
}

u8 dequeue()
{
    u8 counter = 0;
    u8 poped_data;
    for (counter; counter <= 5; counter++)
    {
        if (queue[counter] != 0)
        {
            poped_data = queue[counter];
            queue[counter] = 0;
            if (counter == 4)
            {
                q_index = 0;
            }
            q_size--;
            return poped_data;
        }
    }
    return 0;
}

/* 
 * returned Value = 1? value exists
 * returned Value = 0? value isn't exist
 */
void remove_value(u8 value)
{
    u8 i = 0;
    for (i; i <= 4; i++)
    {
        if (value == requests[i])
        {
            requests[i] = 0;
        }
    }
}
int main()
{
    // diable the watch dog
    WDTCN = 0x0DE;
    WDTCN = 0x0AD;

    OSCICN = 0x014; // 2MH clock

    // config cross bar
    XBR0 = 0x00;
    XBR1 = 0x14;
    XBR2 = 0x040; // Cross bar enabled , weak Pull-up enabled

    IT0 = 1; // Configure interrupt 0 for falling edge on INT0
    EX0 = 1; // Enabling the EX0 interrupt
    EA = 1;  // Enabling the global interrupt flag

    P2MDOUT = 0xff;
    P1MDOUT = 0xff; //seven segment
    P0MDOUT = 0xff; //seven segment
    P0 = 0x00;      //seven segment
    P1 = 0x00;      //seven segment

    // 01111111
    P2 = 0x7f; //CONTROL_PORTS
    seven_segment(0);
    index = 0;
    while (1)
    {
    }
    return 0;
}
void request(void) interrupt 0
{
    if (!GET_BIT(P2, 0))
    {
        // going down request
        enqueue(1);
    }
    if (!GET_BIT(P2, 1))
    {
        // going up request
        enqueue(2);
    }
    if (!GET_BIT(P2, 2))
    {
        // Ground
        push_value(10);
        if (CURRENT_FLOOR != 0)
        {
            elevate(CURRENT_FLOOR, 0);
        }
        
    }
    if (!GET_BIT(P2, 3))
    {
        // floor 1
        push_value(1);
        if (CURRENT_FLOOR != 1)
        {
            elevate(CURRENT_FLOOR, 1);
        }

    }
    if (!GET_BIT(P2, 4))
    {
        // floor 2
        push_value(2);
        if (CURRENT_FLOOR != 2)
        {
            elevate(CURRENT_FLOOR, 2);
        }

    }
    if (!GET_BIT(P2, 5))
    {
        // floor 3
        push_value(3);
        if (CURRENT_FLOOR != 3)
        {
            elevate(CURRENT_FLOOR, 3);
        }
    }
    if (!GET_BIT(P2, 6))
    {
        // floor 4
        push_value(4);
        if (CURRENT_FLOOR != 4)
        {
            elevate(CURRENT_FLOOR, 4);
        }
    }
}

