#include "Group.hpp"
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

    Group group("ИУ1-43B");

    group.addStudent(s1);
    group.addStudent(s2);

    group.printAll();


    delete s1;
    delete s2;

    return 0;
}