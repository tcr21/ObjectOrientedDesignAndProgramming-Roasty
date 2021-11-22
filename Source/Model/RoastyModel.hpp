#ifndef ROASTYMODEL_HPP
#define ROASTYMODEL_HPP

#pragma once
#include <iostream>
#include <string>

//=============================================================================

// I. BEAN CLASS
class Bean
{
    private: 
        // 1. DATA MEMBERS
        // a. Declare name
        std::string name; 
    
    public: 
        // 2. GETTER FUNCTIONS
        // a. Declare getter function so name can be accessed. Use reference
        // parameter for memory efficiency (string considered large
        // data member). Make function and return type const to
        // ensure name is not modified
        std::string const& getName() const; 

        // 3. CONSTRUCTORS/ DESTRUCTORS
        // a. Declare constructor explicitly
        Bean(std::string const& name);
};

//=============================================================================

// II. INGREDIENT CLASS
class Ingredient
{
    private:
        // 1. DATA MEMBERS
        // a. Declare bean and amount
        Bean bean;
        int amount; 
        
    public:
        // 3. GETTER FUNCTIONS
        // a. Declare getter function so type can be accessed. Use reference
        // parameter for memory efficiency (Bean considered large data type).
        // Make function and return type const to ensure event type is not
        // modified
        Bean const& getBean() const;
        
        // b. Declare getter function so amount can be accessed. Use pass by
        // value parameter (int not considered large data type). Make
        // function const to ensure it cannot modify anything
        int getAmount() const;
        
        // 4. SETTER FUNCTIONS
        // a. Declare setter function so ingredient amount can be set. Use pass
        // by value parameter (int not considered large data type)
        void setAmount(const int newAmount);
        
        // 5. CONSTRUCTORS/ DESTRUCTORS
        // a. Declare constructor explicitly
        Ingredient(Bean const& bean, int amount); 
}; 

//=============================================================================

// III. INDGREDIENT NODE CLASS
class IngredientNode
{
    public:
        // 1. DATA MEMBERS
        // a. Declare ingredient and pointer
        Ingredient ingredient;
        IngredientNode* next = nullptr;

        // 2. CONSTRUCTORS/ DESTRUCTORS
        // b. Declare constructor explicitly
        IngredientNode(Ingredient& ingredient, IngredientNode* next);
}; 

//=============================================================================

// IV. EVENT VALUE CLASS
class EventValue
{
    private:
        // 1. DATA MEMBERS
        // a. Declare value
        long value; 

    public:
        // 2. GETTER FUNCTIONS
        // a. Declare getter function so event integer value can be accessed.
        // Use pass by value parameter (int not considered large data type).
        // Make function const to ensure it cannot modify anything
        long getValue() const;

        // 3. SETTER FUNCTIONS
        // a. Declare setter function so event value can be set. Use pass value
        // parameter (int not considered large data type)
        void setValue(const long newValue);

        // 4. CONSTRUCTORS/ DESTRUCTORS

        // a. Declare constructor explicitly
        EventValue(long value); 

}; 

//=============================================================================

// V. EVENT CLASS
class Event
{
    private:
        // 1. DATA MEMBERS
        // a. Declare type and timestamp 
        std::string type; 
        long timestamp=0;
        
        // b. Declare optional event value
        EventValue* eventValue = nullptr;

    public:
        // 3. GETTER FUNCTIONS
        // a. Declare getter function so type can be accessed. Use reference
        // parameter for memory efficiency (string considered large data type).
        // Make function and return type const to ensure event type is not
        // modified
        std::string const& getType() const;
        
        // b. Declare getter function so timestamp can be accessed. Use pass by
        // value parameter (long int not considered large data type). Make
        // function const to ensure it cannot modify anything
        long const& getTimestamp() const;

        // c. Declare getter function so event value can be accessed. Use pass
        // by value parameter (pointer to int not considered large data type).
        // Make function const to ensure it cannot modify anything
        EventValue* getValue() const;

        // d. Declare hasValue function to check if event has value
        bool hasValue() const; 

        // 4. CONSTRUCTORS/ DESTRUCTORS
        // a. Declare constructors explicitly (for event without/with value)
        Event(std::string const& type, long timestamp);
        Event(std::string const& type, long timestamp, EventValue* eventValuePtr);

        // Declare copy assigment, copy constructor, and destructor because
        // event points to optional event value, which it needs to make deep
        // copies of. Follow rule of three 
        // b. Declare copy assignment
        Event& operator=(Event const& otherEvent);

        // c. Declare copy constructor
        Event(Event const& otherEvent);

        // d. Declare destructor
        ~Event();
}; 

//=============================================================================

// VI. EVENT NODE CLASS
class EventNode
{
    public:
        // 1. DATA MEMBERS
        // a. Declare event and pointer
        Event event; 
        EventNode* next = nullptr;

        // 2. CONSTRUCTORS/ DESTRUCTORS
        // a. Declare constructor explicitly
        EventNode(Event& event, EventNode* next);
}; 

//=============================================================================

// VII. ROAST CLASS
class Roast
{
    private: 
        // 1. DATA MEMBERS
        // a. Declare timestamp
        long timestamp;

        // b. Declare ingredient and event count
        int ingredientsCount = 0;
        int eventCount = 0;

        // 2. POINTERS FOR LINKED LISTS
        // a. Declare ingredient and event pointers for heads of linked lists 
        IngredientNode* ingredients = nullptr;
        EventNode* events = nullptr;

    public: 
        // 3. GETTER FUNCTIONS
        // a. Declare getter function so timestamp, ingredient count and event
        // count can be accessed. Use pass by value parameter (long int and int
        // not considered large data types). Make function const to ensure it
        // cannot modify anything
        long getTimestamp() const;
        int getIngredientsCount() const; 
        int getEventCount() const; 

        // b. Declare getter functions so ingredients and events can be
        // accessed. Use pass by reference parameter for memory efficiency
        // (Ingredient and Event are considered large data type)
        Ingredient& getIngredient(int i) const;
        Event const& getEvent(int i) const; 
        Event const& getEventByTimestamp(long timestamp) const; 

        // 4. ADD/ REMOVE FUNCTIONS
        // a. Declare add functions for ingredients and events
        void addIngredient(Ingredient ingredient);
        void addEvent(Event event);

        // b. Declare remove functions for ingredients and events
        void removeIngredientByBeanName(std::string const& beanName);
        void removeEventByTimestamp(long timestamp);

        // 5. CONSTRUCTORS/ DESTRUCTORS
        // a. Declare constructor explicitly
        Roast(long timestamp, int countIngredients = 0, int countEvents = 0);

        // Declare copy assigment, copy constructor, and destructor, because
        // roasty points to ingredients and events which it needs to make deep
        // copies of. Follow rule of three 
        // b. Declare copy assignment
        Roast& operator=(Roast const& otherRoast);

        // c. Declare copy constructor
        Roast(Roast const& otherRoast);

        // d. Declare destructor
        ~Roast();
}; 

#endif