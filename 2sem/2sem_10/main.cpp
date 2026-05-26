#include <vector>
#include <iostream>
#include "Group.hpp"
#include "Student.hpp"
#include "Teacher.hpp"

int main()
{
    setlocale(LC_ALL, "Russian");
    Student* s1 = new Student("Иванов", "26У123");
    Student* s2 = new Student("Петров", "26У456");

    s1->addGrade(5);
    s1->addGrade(4.67);

    s2->addGrade(3.15);
    s2->addGrade(3.89);

    Teacher* t1 = new Teacher("Васильев", "Математика");
    std::vector<Person*> people;
    people.push_back(s1);
    people.push_back(s2);
    people.push_back(t1);

    for (const auto& person : people)
        person->print();

    for (auto person : people)
        delete person;

    Group group("ИУ1-43B");

    group.addStudent(s1);
    group.addStudent(s2);

    group.printAll();


    delete s1;
    delete s2;

    return 0;
}