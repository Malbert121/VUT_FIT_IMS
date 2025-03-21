#include <simlib.h>             //TODO FIX name
#include <iostream>
#include <cmath>


Store WashingMachine("Washing Machine", 60);
Store Chopper("Apples chopper", 60);
Store Juicer("Juicer", 65);
Store PulpDestroyer("PulpDestroyer", 10000);
Store DistillStation("DistillStation", 10000);
Store ConcentrateBarrel("ConcentrateBarrel", 10000);
Store AromaBarrel("AromaBarrel", 1000);
Store Storage("Factory storage", 2000);
Store ConcentrateCheck("Concentrate check", 5);


Facility AppleControle("Controle if the apple is spoiled");
Facility Pour("Pour");
Facility PourAroma("Pour Aroma");


bool BadState = false;

Queue aromaQueue("BarrelAroma waits if i can pour to Juice");
Queue concentrateQueue("Barrelconcentrate waits if i can pour to Juice");

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
        Enter(ConcentrateCheck, 1);
        if (BadState){
            BadState = false;
            barrel_spoiled++;
            Cancel();
        }
        Leave(ConcentrateCheck, 1);
        Wait(100);
        Into(concentrateQueue);
        Passivate();
    }
};

BarrelConcentrate::BarrelConcentrate(double Size) : Value(Size) {}
unsigned long cnt = 0;
class BarrelAroma : public Process{

public:
    double Value;
    explicit BarrelAroma(double Value);
    void Behavior() override{
        if(cnt++ % 10 == 0){
            Wait(100);
            Into(aromaQueue);
            Passivate();
        }
        else{
            Wait(10);
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
        Wait(Uniform(4,6));       
        Leave(WashingMachine, 1);
        apples_washed++;

        Seize(AppleControle);
        Wait(Uniform(0.075, 0.09));
        if (isSpoiled) {
            apples_spoiled++;
            Release(AppleControle);
            Cancel();
        }
        Release(AppleControle);


        Enter(Chopper, 1);

        Wait(Uniform(4,6));
        Leave(Chopper, 1);
        apples_chopped++;

        Enter(Juicer, 1);
        Wait(Uniform(5,6));
        Leave(Juicer, 1);

        apples_juiced++;
        Enter(PulpDestroyer, Liquid);
        Wait(Uniform(6,7));
        total_liquid += Liquid;
        Leave(PulpDestroyer, Liquid);

        Enter(DistillStation, Liquid);
        Wait(Uniform(6,7));
        Leave(DistillStation, Liquid);


        auto concentrateLiquid = std::lround(Liquid * Uniform(0.10, 0.15)); //Uniform(0.10, 0.15)
        auto aromaLiquid = std::lround(Liquid * Uniform(0.10, 0.15));
        Wait(10);
        Enter(ConcentrateBarrel, concentrateLiquid);  //  TODO ADD AROMA PROCESS?
        Seize(Pour);
        if (ConcentrateBarrel.Used() >= 9800){
            (new BarrelConcentrate((double)ConcentrateBarrel.Used()))->Activate();
            barrel_created++;
            Leave(ConcentrateBarrel, ConcentrateBarrel.Used());
        }
        Wait(1);
        Release(Pour);
        Enter(AromaBarrel, aromaLiquid);
        Seize(PourAroma);
        if (AromaBarrel.Used() >= 980){
            (new BarrelAroma((double )AromaBarrel.Used()))->Activate();
            aroma_created++;
            Leave(AromaBarrel, AromaBarrel.Used());
        }
        Wait(1);
        Release(PourAroma);

    }
};


class AppleGenerator : public Event 
{
    public:
        double juiciness;
    AppleGenerator(double juiciness);
    void Behavior() override   // do not stop
    {
        double genJuice = (int)Exponential(juiciness) % 300;
        if (juiciness < 100){
            genJuice += 100;
        }
        (new Apple(Random() > 0.97, genJuice))->Activate();
        double d = 0.08333;
        Activate(Time + Exponential(d));
    }
};
AppleGenerator::AppleGenerator(double juiciness) : juiciness(juiciness) {}

class BadGenerator : public Event
{
    void Behavior() override
    {
        if (Random() < 0.4){
            BadState = true;
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
        idle:
        while (!concentrateQueue.Empty() && !aromaQueue.Empty()){
            Wait(400);
            auto *concentrate = (BarrelConcentrate *)concentrateQueue.GetFirst();
            auto *aroma = (BarrelAroma *)aromaQueue.GetFirst();
            auto usingConcentrate =100;
            auto usingAroma = 0.5;
            while (concentrate->Value > usingConcentrate){
                if (aroma->Value >= usingAroma){
                    concentrate->Value -= usingConcentrate;
                    aroma->Value -= usingAroma;
                    juice_packets_created++;
                    
                }
                else if (!aromaQueue.Empty()){
                    aroma = (BarrelAroma *)aromaQueue.GetFirst();

                } else {
                    break;
                }
                if (concentrate->Value < usingConcentrate && !concentrateQueue.Empty()){
                    concentrate = (BarrelConcentrate *)concentrateQueue.GetFirst();
                }
            }
        }
        if (Time < 86400){
            Wait(1);
            goto idle;
        }
        Passivate();
    }

};

JuiceMadeProcess::JuiceMadeProcess() = default;




// Main function to run the simulation
int main(int argc, char **argv)
{
    if (argc != 2) {
        std::cout << "Please, enter average juiciness of every apple" << std::endl;
        std::cout << "Acceptable interval is from 100 to 300" << std::endl;
        std::cout << "For example: ./juiceFactory 200" << std::endl;
        return 1;
    }

    double juiciness = atof(argv[1]); 
    if (juiciness < 100 || juiciness > 300) {
        std::cout << "Juiciness value must be between 100 and 300." << std::endl;
        return 1;
    }
    RandomSeed(time(nullptr));
    SetOutput("apple_simulation.out");
    Init(0, 86400);
    (new AppleGenerator(juiciness))->Activate();
    (new BadGenerator)->Activate();
    (new JuiceMadeProcess)->Activate();
    // Run the simulation
    Run();

    // Output simulation statistics
    Print("Apples created: %d\n", apples_created);
    Print("Apples washed: %d\n", apples_washed);
    Print("Apples spoiled: %d\n", apples_spoiled);
    Print("Apples waiting in queue to washing machine: %d\n", WashingMachine.QueueLen());
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
    AromaBarrel.Output();
    concentrateQueue.Output();
    aromaQueue.Output();

    Storage.Output();

    return 0;
}
