#include "Student.hpp"

int main()
{
    setlocale(LC_ALL, "Russian");
    Student s1("Иванов");
    Student s2("Петров");

    s1.addGrade(4.5);
    s1.addGrade(5.0);
    s1.addGrade(4.0);

    s2.addGrade(3.0);
    s2.addGrade(3.5);
    s2.addGrade(4.0);

    s1.print();
    s2.print();

    return 0;
}