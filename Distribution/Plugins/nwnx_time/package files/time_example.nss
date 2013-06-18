#include "nwnx_time"

void Stop(object oPC);

void main()
{
    object oPC = GetFirstPC();

    if(oPC == OBJECT_INVALID) return;

    StartTimer(oPC, "mytimer_name");

    DelayCommand(1.5f, Stop(oPC));


}


void Stop(object oPC) {
    string sTime;

    sTime = StopTimer(oPC, "mytimer_name");
    SendMessageToPC(oPC, sTime);

}
