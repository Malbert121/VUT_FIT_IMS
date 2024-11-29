#include <simlib.h>
#include <iostream>


Store WashingMachine("Washing Machine", 2);
Store Chopper("Apples chopper", 2);
Store Juicer("Juicer", 2);

// Counters
unsigned apples_created = 0;
unsigned apples_washed = 0;
unsigned apples_spoiled = 0;
unsigned apples_chopped = 0;
unsigned apples_juiced = 0;
double total_liquid = 0;

// Apple class to simulate apple creation and washing
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
        Wait(1);        //wash 1-2 seconds
        Leave(WashingMachine, 1);
        apples_washed++;

        if (isSpoiled) {
            apples_spoiled++;
            Passivate();  
            return;
        }

        Enter(Chopper, 1);
        Wait(1.5);
        Leave(Chopper, 1);
        apples_chopped++;

        Enter(Juicer, 1);
        Wait(2);
        Leave(Juicer, 1);
        apples_juiced++;

        total_liquid += Liquid;


        Passivate(); 
    }
};


class AppleGenerator : public Event
{
    void Behavior() override
    {   if(Time >= 1000){
        return;
    }
        (new Apple(Random() > 0.995, 100 + ((int)Exponential(100) % 200)))->Activate();
        double d = Exponential(1);
        Activate(Time + d);
    }
};


// Main function to run the simulation
int main()
{
    RandomSeed(time(nullptr)); 
    SetOutput("apple_simulation.out");  
    Init(0, 5000);  
   
    (new AppleGenerator)->Activate();

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
    WashingMachine.Output();
    Chopper.Output();
    Juicer.Output();
    return 0;
}
