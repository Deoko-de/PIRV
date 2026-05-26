#include "Person.hpp"

Person::Person(const std::string& name)
    : name(name)
{
}


void Person::print() const
{
    std::cout << "Èìÿ: " << name << std::endl;
}