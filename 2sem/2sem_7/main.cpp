#include <vector>
#include "Student.hpp"
#include "Teacher.hpp"

int main()
{
    setlocale(LC_ALL, "Russian");
    Student* s1 = new Student("Иванов");
    Student* s2 = new Student("Петров");

    s1->addGrade(4.78);
    s1->addGrade(5.0);

    s2->addGrade(3.67);
    s2->addGrade(3.75);

    Teacher* t1 = new Teacher("Смирнов", "Математика");

    std::vector<Person*> people;

    people.push_back(s1);
    people.push_back(s2);
    people.push_back(t1);

    for (const auto& person : people)
        person->print();

    for (auto person : people)
        delete person;

    return 0;
}