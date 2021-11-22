

#include "RoastyModel.hpp"
#include <iostream>
#include <string>

using namespace std; 

// PLEASE REFER TO HPP FILE FOR HIGH LEVEL COMMENTS ON EACH FUNCTION

//=============================================================================

// I. BEAN
// 1. GETTER FUNCTIONS
string const& Bean::getName() const
{
    return name; 
}

// 2. CONSTRUCTORS/ DESTRUCTORS
Bean::Bean(string const& name) : name(name) {}

//=============================================================================

// II. INGREDIENT NODE CLASS
// 1. CONSTRUCTORS/ DESTRUCTORS
IngredientNode::IngredientNode(Ingredient& ingredient, IngredientNode* next) : ingredient(ingredient), next(next) {} 

//=============================================================================

// III. INGREDIENT CLASS
// 1. GETTER FUNCTIONS
// a.
Bean const& Ingredient::getBean() const 
{
    return bean;
}
// b.     
int Ingredient::getAmount() const
{
    return amount;
}

// 2. SETTER FUNCTIONS
void Ingredient::setAmount(const int newAmount)
{
    amount = newAmount; 
} 

// 3. CONSTRUCTORS/ DESTRUCTORS
Ingredient::Ingredient(Bean const& bean, int amount) : bean(bean), amount(amount) {}

//=============================================================================

// IV. EVENT VALUE CLASS
// 1. GETTER FUNCTIONS
long EventValue::getValue() const
{
    return value;
}

// 2. SETTER FUNCTIONS
void EventValue::setValue(const long newValue)
{
    value = newValue; 
}

// 3. CONSTRUCTORS/ DESTRUCTORS
EventValue::EventValue(long value) : value(value) {}

//=============================================================================

// V. EVENT NODE CLASS
// 1. CONSTRUCTORS/ DESTRUCTORS
EventNode::EventNode(Event& event, EventNode* next) : event(event), next(next) {}

//=============================================================================

// VI. EVENT CLASS
// 1. GETTER FUNCTIONS
// a.
string const& Event::getType() const
{
    return type;
}
// b.
long const& Event::getTimestamp() const
{
    return timestamp; 
}
// c.
EventValue* Event::getValue() const
{
    return eventValue; 
}
// d.
bool Event::hasValue() const
{
    if (eventValue != nullptr)
    {
        return true;
    }
    return false; 
}

// 2. CONSTRUCTORS/ DESTRUCTORS
// a.
Event::Event(std::string const& type, const long timestamp) : type(type), timestamp(timestamp) {}
Event::Event(string const& type, const long timestamp, EventValue* eventValue) : type(type), timestamp(timestamp), eventValue(eventValue) {}
// b.
Event& Event::operator=(Event const& otherEvent)
{
    if (otherEvent.eventValue != nullptr)
    {
        this->eventValue = new EventValue(otherEvent.eventValue->getValue());
    }
    this->type = otherEvent.type;
    this->timestamp = otherEvent.timestamp; 
    return *this;
}
// c.
Event::Event(Event const& otherEvent)
{
    // Delegate to copy assignment
    *this = otherEvent;
}
// d.
Event::~Event()
{
    if (eventValue != nullptr)
    {
        delete eventValue; 
    }
}

//=============================================================================

// VII. ROAST
// 1. GETTER FUNCTIONS
// a. 
long Roast::getTimestamp() const
{
    return timestamp; 
}

int Roast::getIngredientsCount() const
{
    return ingredientsCount; 
}

int Roast::getEventCount() const
{
    return eventCount; 
}

// b. 
Ingredient& Roast::getIngredient(int i) const
{
    IngredientNode* nextIngredient = ingredients; 
    int count = 0; 
    while (nextIngredient != nullptr)
    {
        if (i == count)
        {
            return nextIngredient->ingredient;
        }
        count++; 
        nextIngredient = nextIngredient->next; 
    }
    return ingredients->ingredient;
}

Event const& Roast::getEvent(int i) const
{
    EventNode* nextEvent = events; 
    int count = 0; 
    while (nextEvent != nullptr)
    {
        if (i == count)
        {
            return nextEvent->event;
        }
        count++; 
        nextEvent = nextEvent->next; 
    } 
    return events->event;
}

Event const & Roast::getEventByTimestamp(long timestamp) const
{
    EventNode* nextEvent = events; 
    int count = 0; 
    while (nextEvent != nullptr)
    {
        if (timestamp == nextEvent->event.getTimestamp())
        {
            return nextEvent->event; 
        }
        count++; 
        nextEvent = nextEvent->next; 
    }
    return events->event;
}

// 2. ADD/ REMOVE FUNCTIONS
// a. 
void Roast::addIngredient(Ingredient ingredient) 
{
    IngredientNode* nextIngredient = ingredients;  
    ingredients = new IngredientNode(ingredient, nextIngredient); 
    ingredients->next = nextIngredient;   
    ingredientsCount++;
}

void Roast::addEvent(Event event)
{
    EventNode* nextEvent = events;  
    events = new EventNode(event, nextEvent); 
    events->next = nextEvent; 
    eventCount++;
}

// b. 
void Roast::removeIngredientByBeanName(string const& beanName) 
{
    IngredientNode* ingredientToRemove = ingredients; 
    IngredientNode* ingredientPrevious = nullptr; 
    if (ingredientToRemove == nullptr)
    {
        return;
    }
    if (ingredientToRemove->ingredient.getBean().getName() == beanName)
    {
        ingredients = ingredientToRemove->next;
        delete ingredientToRemove;
        ingredientsCount--;
        return;
    } 
    else 
    {

        while(ingredientToRemove != nullptr && ingredientToRemove->ingredient.getBean().getName() != beanName)
        {
            ingredientPrevious = ingredientToRemove;
            ingredientToRemove = ingredientToRemove->next; 
        }
        ingredientPrevious->next = ingredientToRemove->next; 
        delete ingredientToRemove;
        ingredientsCount--; 
    }
}

void Roast::removeEventByTimestamp(long timestamp)
{
    EventNode* eventToRemove = events; 
    EventNode* eventPrevious = nullptr;
    if (eventToRemove == nullptr) 
    {
        return;
    }
    if(eventToRemove->event.getTimestamp() == timestamp) 
    {
        events = eventToRemove->next;
        delete eventToRemove;
        eventCount--;
        return;
    } 
    else 
    {

        while(eventToRemove != nullptr && eventToRemove->event.getTimestamp() != timestamp) {
            eventPrevious = eventToRemove;
            eventToRemove = eventToRemove->next; 
        } 
        eventPrevious->next = eventToRemove->next; 
        delete eventToRemove;
        eventCount--; 
    }
}

// 3. CONSTRUCTORS/ DESTRUCTORS  
// a. 
Roast::Roast(long timestamp, int ingredientsCount, int eventCount) : timestamp(timestamp), ingredientsCount(ingredientsCount), eventCount(eventCount) {}

// b.
Roast& Roast::operator=(Roast const& otherRoast)
{
    // Ingredient
    if (otherRoast.ingredients != nullptr)
    {
        IngredientNode* currentIngredient = otherRoast.ingredients; 
        while (currentIngredient != nullptr)
        {
            // Note: addIngredient adjusts ingredient count
            this->addIngredient(currentIngredient->ingredient); 
            currentIngredient = currentIngredient->next; 
        }
    }
    else
    {   
        this->ingredients = nullptr; 
    }
    // Event
    if (otherRoast.events != nullptr)
    {
        EventNode* currentEvent = otherRoast.events; 
        while (currentEvent != nullptr)
        {
            // Note: addEvent adjusts event count
            this->addEvent(currentEvent->event);
            currentEvent = currentEvent->next; 
        }
    }
    else
    {   
        this->events = nullptr; 
    }
    // Timestamp
    this->timestamp = otherRoast.timestamp;

    return *this; 
}

// c. 
Roast::Roast(Roast const& otherRoast)
{
    // Delegate to copy assignment
    *this = otherRoast; 
}

// d. 
Roast::~Roast()
{
    // Ingredient
    IngredientNode* currentIngredient = ingredients; 
    while (currentIngredient != nullptr) 
    {
        IngredientNode* nextIngredient = currentIngredient->next; 
        delete currentIngredient;
        currentIngredient = nextIngredient; 
    }

    // Event
    EventNode* currentEvent = events; 
    while (currentEvent != nullptr) 
    {
        EventNode* nextEvent = currentEvent->next; 
        delete currentEvent;
        currentEvent = nextEvent; 
    }
}











