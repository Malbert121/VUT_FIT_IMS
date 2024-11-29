//
// Pool simulation before reconstruction
//
#include "simlib.h"
#include <iostream>
#include <vector>
#include <algorithm>

#define GROUP_SIZE 32
#define TRUCK_CAPACITY 10
#define Norm(a, b) (std::max(0.0, Normal(a, b)))

Store Crates("Boxes for apple", 20);
Store Trucks("Trucks for apple transport", 2);
Store People("People that collect apple from tree", 50);
Store Forklift("Forklift that transports apple crates to truck", 1);
Store Washer("Forklift that transports apple crates to truck", 100);
Store JuiceMaker("Juicemaker in factory to generate juice", 250);
Store Distill("Distill juice to get concentrate and aroma", 1000);


Queue appleToCrateQueue("Apple go to crate queue");
Queue applesToFactory("Apple in factory queue");
Queue appleToWasher("Apple go to washer queue");
Queue appleToJuiceMaker("Apple go to washer queue");
Queue juiceToDistillQueue("Juice from juice maker");





// 6-9 morning groups
int MorningGroups = 2;
// 9-15 no groups
int AfternoonNoGroups = 6;
// 15-20 evening groups
int EveningGroups = 5;
// total people got in
int people_got_in = 0;

double concentrate = 0;
double aromaLiquid = 0;
double juice_maded = 0;
unsigned long barrels_of_concentrate = 0;
unsigned long barrels_of_aroma = 0;
unsigned long apples_made = 0;
unsigned long apples_were_spoiled = 0;
unsigned long apples_ready_to_box = 0;
unsigned long boxes_in_truck = 0;


Histogram TimeInFactory("Time spent in the factory", 0, 600, 20);
Histogram ApplesSpentOnMakingJuice("Apple spent on making juice", 0, 600, 20);
Histogram JuiceMade("Juice made in litres", 0, 600, 20);
Histogram ApplesWereRejected("Amount of swimmer's interruptions", 0, 1, 10);


/**
 * Apple class
 */
class Apple : public Process
{
private:
    double Liquid;
    bool isSpoiled;
    bool isDirty = true;
public:
    Apple(bool isSpoiled, double Liquid);
    void Behavior() override;
    double getLiquid() {
        return Liquid;
    };
    void setClean(){
        isDirty = false;
    }
};


class Box {

public:
    std::vector<Apple *> apples_in_crate;
    int capacity = 1000 + ((int)Exponential(100) % 200);
    Box();
    void insertAppleToBox(Apple *apple){
        apples_in_crate.push_back(apple);
    }
};
Box::Box() = default;

class Truck {

public:
    std::vector<Apple *> apples_in_truck;
    int size = 0;
    Truck();
    void addAppleToTruck(std::vector<Apple *> apples){
        apples_in_truck.insert(apples_in_truck.end(), apples.begin(), apples.end());
        apples.clear();
        this->size++;
    }
};
Truck::Truck() = default;

std::vector<Apple *> applesInFactory;

std::vector<Box *> boxQueue;

std::vector<Truck *> factoryTrucks;

Truck *truck = new Truck();

Box *box = new Box();

Apple::Apple(bool isSpoiled, double Liquid) : isSpoiled(isSpoiled), Liquid(Liquid) {}

void Apple::Behavior() {

    Enter(People, 1);
    apples_made++;
    if (isSpoiled){
        Leave(People, 1);
        apples_were_spoiled++;
        Cancel();
    }else{
        Into(appleToCrateQueue);
    }
    Wait(Uniform(5, 10));
    Leave(People, 1);
    for (int i = 0; i < box->capacity; i++) {
        if (!appleToCrateQueue.Empty()){
            Apple *apple = (Apple *)appleToCrateQueue.GetFirst();
            box->insertAppleToBox(apple);
        }
    }
    if (box->apples_in_crate.size() == box->capacity){
        Enter(Crates, 1);
        boxQueue.push_back(box);
        box = new Box();
        Enter(Forklift, 1);
        Box *boxToTruck = boxQueue.front();
        boxQueue.erase(boxQueue.begin());
        truck->addAppleToTruck(boxToTruck->apples_in_crate);
        Leave(Crates, 1);
        delete boxToTruck;
        Wait(Exponential(120));
        Leave(Forklift, 1);
    }
    if (truck->size == TRUCK_CAPACITY){
        Enter(Trucks, 1);
        factoryTrucks.push_back(truck);
        truck = new Truck();
        if (!Trucks.Empty()){
            Truck* truckInFactory = factoryTrucks.front();
            factoryTrucks.erase(factoryTrucks.begin());
            Into(applesToFactory);
            truckInFactory->apples_in_truck.clear();
            delete truckInFactory;
            Wait(Exponential(3600));
            Leave(Trucks, 1);
        }
    }


}


class AppleGenerator : public Event
{
    void Behavior() override
    {
        (new Apple(Random() > 0.995, 100 + ((int)Exponential(100) % 200)))->Activate();
        double d = Exponential(0.1);
        Activate(Time + d);
    }
};

class AppleWasher : public Process{

    void Behavior() override{
        while (!appleToWasher.Empty()){
                Enter(Washer, 1);
                Apple *apple = (Apple *)appleToWasher.GetFirst();
                apple->setClean();
                apple->Into(appleToJuiceMaker);
                Wait(Exponential(10));
                Leave(Washer, 1);
        }
    }
};

class JuiceApple : public Process{

    void Behavior() override{
        Passivate();
    }
    public:
        double juice;
        JuiceApple(double juice);
};
JuiceApple::JuiceApple(double juice) : juice(juice) {}

class AppleJuiceMaker : public Process{

    void Behavior() override{
//        while (!appleToJuiceMaker.Empty()){
//
//                Enter(JuiceMaker, 1);
//                Apple *apple = (Apple *)appleToJuiceMaker.GetFirst();
//                JuiceApple *juice = (new JuiceApple(apple->getLiquid()));
//                apple->Cancel();
//                juice->Into(juiceToDistillQueue);
//                Wait(620);
//                Leave(JuiceMaker, 1);
//
//        }
    }
};

class DistillStation : public Process{

    void Behavior() override{
        while (!juiceToDistillQueue.Empty()){
            Enter(Distill, 1);
            auto appleJuice = (JuiceApple *)juiceToDistillQueue.GetFirst();
            double ratio = Uniform(0.30, 0.35);
            concentrate += appleJuice->juice * ratio;
            aromaLiquid += appleJuice->juice * ratio * ratio;
            appleJuice->Cancel();
            Leave(Distill, 1);
            if (concentrate >= 1000){
                concentrate -= 1000;
                barrels_of_concentrate++;
            }
            if (aromaLiquid >= 1000){
                aromaLiquid -= 1000;
                aromaLiquid++;
            }
        }
    }
};

class Factory : public Process
{
    void Behavior() override
    {
        Wait(5000);
        (new AppleWasher)->Activate();
        (new AppleJuiceMaker)->Activate();
        (new DistillStation)->Activate();
        while (!applesInFactory.empty()){
            Apple * apple = applesInFactory.front();
            applesInFactory.erase(applesInFactory.begin());
            apple->Into(appleToWasher);
        }


    }
public:
    Factory();
};
Factory::Factory() {}


int main()
{
    RandomSeed(time(nullptr));
    SetOutput("apple.out");
    Init(0, 50000); // 24 hours
    (new AppleGenerator)->Activate();
    (new Factory)->Activate();
    Run();
    Crates.Output();
    Print("Apples got in: %d\n\n", apples_made);
    Print("Apples spoiled: %d\n\n", apples_were_spoiled);
    Print("Apples in factory: %d\n\n", applesInFactory.size());
    Trucks.Output();
    Forklift.Output();
    People.Output();
    appleToWasher.Output();
    appleToJuiceMaker.Output();
    juiceToDistillQueue.Output();
    return 0;

}