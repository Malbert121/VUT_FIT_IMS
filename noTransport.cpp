#include <simlib.h>             //TODO FIX ALL TIME
#include <iostream>


Store WashingMachine("Washing Machine", 2);
Store Chopper("Apples chopper", 2);
Store Juicer("Juicer", 2);
Store PulpDestroyer("PulpDestroyer", 170000);
Store DistillStation("DistillStation", 170000);
Store Barrel("Barrel", 10000);


Facility Pour("Pour");
Facility ConcentrateCheck("ConcentrateCheck");

// Counters
unsigned apples_created = 0;
unsigned apples_washed = 0;
unsigned apples_spoiled = 0;
unsigned apples_chopped = 0;
unsigned apples_juiced = 0;
double total_liquid = 0;
unsigned barrel_created = 0;
unsigned spoilage_events = 0;
unsigned long concentrate = 0;
unsigned long aromaLiquid = 0;
unsigned barrel_spoiled = 0;
unsigned aroma_created = 0;

// Apple class to simulate apple creation and washing
class BarrelEntity : public Process {
    double Size;
    public:
    BarrelEntity(double Size);
    void Behavior() override{
        Seize(ConcentrateCheck);
        Wait(5);
        Release(ConcentrateCheck);
    }
};

BarrelEntity::BarrelEntity(double Size) : Size(Size) {}

class Aroma : public Process{
    double Value;
    public:
    Aroma(double Value);
    void Behavior() override{

    }
};

Aroma::Aroma(double Value) : Value(Value) {}
 

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

        Enter(Barrel, Liquid* Uniform(0.10, 0.15));  //  TODO ADD AROMA PROCESS?
        Seize(Pour);
        if (Barrel.Used() >= 9800){
            (new BarrelEntity(Barrel.Used()))->Activate();
            barrel_created++;
            Leave(Barrel, Barrel.Used());
            aroma_created++;
        }
        Release(Pour);
    }
};


class AppleGenerator : public Event
{
    void Behavior() override   // do not stop  
    {   
        (new Apple(Random() > 0.995, 100 + ((int)Exponential(100) % 200)))->Activate();
        double d = Exponential(1);
        Activate(Time + d);
    }
};

class BadGenerator : public Event
{
    void Behavior() override
    {
        if(ConcentrateCheck.Busy()){
           Entity * barrelSpoiled = ConcentrateCheck.In();
           barrelSpoiled->Cancel();
           ConcentrateCheck.Clear();
           barrel_spoiled++;
        }
        Activate(Time + Exponential(10));                   // TODO FIX TIME
    }
};


 

// Main function to run the simulation
int main()
{
    RandomSeed(time(nullptr)); 
    SetOutput("apple_simulation.out");  
    Init(0, 5000);  
   
    (new AppleGenerator)->Activate();
    (new BadGenerator)->Activate();

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
    Print("Barrel created: %d\n", barrel_created);
    Print("Barrel spoiled: %d\n", barrel_spoiled);
    Print("aroma created: %d\n", aroma_created);
    WashingMachine.Output();
    Chopper.Output();
    Juicer.Output();
    PulpDestroyer.Output();
    DistillStation.Output();
    Barrel.Output();
    return 0;
}
