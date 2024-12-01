#include <simlib.h>             //TODO FIX ALL TIME
#include <iostream>
#include <cmath>


Store WashingMachine("Washing Machine", 100);
Store Chopper("Apples chopper", 100);
Store Juicer("Juicer", 100);
Store PulpDestroyer("PulpDestroyer", 170000);
Store DistillStation("DistillStation", 170000);
Store ConcentrateBarrel("ConcentrateBarrel", 10000);
Store AromaBarrel("AromaBarrel", 1000);
Store Storage("Factory storage", 2000);

Facility Pour("Pour");
Facility ConcentrateCheck("ConcentrateCheck");


Queue aromaQueue("BarrelAroma waits if i can pour to Juice");
Queue concentrateQueue("BarrelAroma waits if i can pour to Juice");

// Counters
unsigned apples_created = 0;
unsigned apples_washed = 0;
unsigned apples_spoiled = 0;
unsigned apples_chopped = 0;
unsigned apples_juiced = 0;
double total_liquid = 0;
unsigned barrel_created = 0;
unsigned spoilage_events = 0;
unsigned barrel_spoiled = 0;
unsigned aroma_created = 0;
unsigned juice_packets_created = 0;

// Apple class to simulate apple creation and washing
class BarrelConcentrate : public Process {

public:
    double Value;
    explicit BarrelConcentrate(double Size);
    void Behavior() override{
        Seize(ConcentrateCheck);
        Wait(100);
        Release(ConcentrateCheck);
        Into(concentrateQueue);
        Passivate();
    }
};

BarrelConcentrate::BarrelConcentrate(double Size) : Value(Size) {}

class BarrelAroma : public Process{

public:
    double Value;
    explicit BarrelAroma(double Value);
    void Behavior() override{
        if (Random() < 0.60){
            Wait(10);
            Into(aromaQueue);
            Passivate();
        }
        else{
            Wait(100);
            Enter(Storage, 1);
        }

    }
};

BarrelAroma::BarrelAroma(double Value) : Value(Value) {}


class Apple : public Process
{
private:
    bool isSpoiled;
    double Liquid;

public:
    Apple(bool isSpoiled, double Liquid) : isSpoiled(isSpoiled), Liquid(Liquid) {}

    void Behavior() override {
        apples_created++;

        Enter(WashingMachine, 1);
        Wait(1);        //wash 1 second
        Leave(WashingMachine, 1);
        apples_washed++;

        if (isSpoiled) {
            apples_spoiled++;
            Cancel();
        }


        Enter(Chopper, 1);

        Wait(1.5);
        Leave(Chopper, 1);
        apples_chopped++;

        Enter(Juicer, 1);
        Wait(2);
        Leave(Juicer, 1);
        apples_juiced++;
        Enter(PulpDestroyer, Liquid);
        Wait(2);
        total_liquid += Liquid;
        Leave(PulpDestroyer, Liquid);

        Enter(DistillStation, Liquid);
        Wait(2);
        Leave(DistillStation, Liquid);


        auto concentrateLiquid = std::lround(Liquid * Uniform(0.10, 0.15));
        auto aromaLiquid = std::lround(concentrateLiquid * Uniform(0.10, 0.15));
        Enter(ConcentrateBarrel, concentrateLiquid);  //  TODO ADD AROMA PROCESS?
        Seize(Pour);
        if (ConcentrateBarrel.Used() >= 9800){
            (new BarrelConcentrate((double)ConcentrateBarrel.Used()))->Activate();
            barrel_created++;
            Leave(ConcentrateBarrel, ConcentrateBarrel.Used());
        }
        Enter(AromaBarrel, aromaLiquid);
        if (AromaBarrel.Used() >= 980){
            (new BarrelAroma((double )AromaBarrel.Used()))->Activate();
            aroma_created++;
            Leave(AromaBarrel, AromaBarrel.Used());
        }
        Wait(1);
        Release(Pour);

    }
};


class AppleGenerator : public Event
{
    void Behavior() override   // do not stop  
    {
        (new Apple(Random() > 0.97, 100 + ((int)Exponential(100) % 200)))->Activate();
        double d = Exponential(0.01);
        Activate(Time + d);
    }
};

class BadGenerator : public Event
{
    void Behavior() override
    {
        if (Random() < 0.4){
            if(ConcentrateCheck.Busy()){
                Entity * barrelSpoiled = ConcentrateCheck.In();
                barrelSpoiled->Cancel();
                ConcentrateCheck.Clear();
                barrel_spoiled++;
            }
        } else{

        }

        Activate(Time + Exponential(3600 * 6));                   // TODO FIX TIME
    }
};

class JuiceMadeProcess : public Process
        {
    public:
            JuiceMadeProcess();
            void Behavior() override{
                Wait(1000);
                idle:
                while (!concentrateQueue.Empty() && !aromaQueue.Empty()){
                    auto *concentrate = (BarrelConcentrate *)concentrateQueue.GetFirst();
                    auto *aroma = (BarrelAroma *)aromaQueue.GetFirst();
                    auto usingConcentrate =Uniform(100, 150);
                    auto usingAroma = Uniform(0.1, 0.2);
                    while (concentrate->Value > usingConcentrate){
                        if (aroma->Value >= usingAroma){
                            concentrate->Value -= usingConcentrate;
                            aroma->Value -= usingAroma;
                            juice_packets_created++;
                            Wait(0.001);
                        }
                        else if (!aromaQueue.Empty()){
                            aroma = (BarrelAroma *)aromaQueue.GetFirst();
                            Wait(0.001);
                        } else {
                            break;
                        }
                        if (concentrate->Value < usingConcentrate && !concentrateQueue.Empty()){
                            concentrate = (BarrelConcentrate *)concentrateQueue.GetFirst();
                            Wait(0.001);
                        }
                    }
                }
                if (Time < 86000){
                    Wait(1);
                    goto idle;
                }
                Passivate();
            }

};

JuiceMadeProcess::JuiceMadeProcess() = default;




// Main function to run the simulation
int main()
{
    RandomSeed(time(nullptr));
    SetOutput("apple_simulation.out");
    Init(0, 86000);

    (new AppleGenerator)->Activate();
    (new BadGenerator)->Activate();
    (new JuiceMadeProcess)->Activate();
    // Run the simulation
    Run();

    // Output simulation statistics
    Print("Apples created: %d\n", apples_created);
    Print("Apples washed: %d\n", apples_washed);
    Print("Apples spoiled: %d\n", apples_spoiled);
    Print("Apples waiting in queue: %d\n", WashingMachine.QueueLen());
    Print("Apples chopped: %d\n", apples_chopped);
    Print("Apples juiced: %d\n", apples_juiced);
    Print("Total juice: %.2f \n", total_liquid);
    Print("ConcentrateBarrel created: %d\n", barrel_created);
    Print("ConcentrateBarrel spoiled: %d\n", barrel_spoiled);
    Print("aroma created: %d\n", aroma_created);
    Print("juice maded: %d\n", juice_packets_created);
    WashingMachine.Output();
    Chopper.Output();
    Juicer.Output();
    PulpDestroyer.Output();
    DistillStation.Output();
    ConcentrateBarrel.Output();
    return 0;
}
