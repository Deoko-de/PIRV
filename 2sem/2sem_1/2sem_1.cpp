#include <iostream>

using namespace std;

void enterScores(double* scoresArray, int studentCount)
{
    for (int i = 0; i < studentCount; i++)
    {
        double currentScore;
        while (true)
        {
            cout << "Укажите успеваемость учащегося " << i + 1 << " (от 0 до 5): ";
            cin >> currentScore;

            if (currentScore < 0.0 || currentScore > 5.0)
            {
                cout << "Некорректное значение. Попробуйте снова.\n";
            }
            else
            {
                scoresArray[i] = currentScore;
                break;
            }
        }
    }
}

double calculateAverage(const double* scoresArray, int studentCount)
{
    double total = 0.0;
    for (int i = 0; i < studentCount; i++)
        total += scoresArray[i];

    return total / studentCount;
}

double findHighest(const double* scoresArray, int studentCount)
{
    double highest = scoresArray[0];
    for (int i = 1; i < studentCount; i++)
        if (scoresArray[i] > highest)
            highest = scoresArray[i];

    return highest;
}

double findLowest(const double* scoresArray, int studentCount)
{
    double lowest = scoresArray[0];
    for (int i = 1; i < studentCount; i++)
        if (scoresArray[i] < lowest)
            lowest = scoresArray[i];

    return lowest;
}

int countAboveLimit(const double* scoresArray, int studentCount, double limitValue)
{
    int counter = 0;
    for (int i = 0; i < studentCount; i++)
        if (scoresArray[i] > limitValue)
            counter++;
    return counter;
}

int main()
{

    setlocale(LC_ALL, "Russian");
    int studentCount;

    cout << "Введите число учащихся: ";
    cin >> studentCount;

    if (studentCount <= 0)
    {
        cout << "Ошибка: количество учащихся должно быть положительным\n";
        return 1;
    }

    double* scoresArray = new double[studentCount];

    enterScores(scoresArray, studentCount);

    cout << "\nОбщий средний показатель: "
        << calculateAverage(scoresArray, studentCount) << endl;

    cout << "Наивысший балл: "
        << findHighest(scoresArray, studentCount) << endl;

    cout << "Наименьший балл: "
        << findLowest(scoresArray, studentCount) << endl;

    double limitValue;
    cout << "Задайте пороговое значение: ";
    cin >> limitValue;

    cout << "Число учащихся, превысивших порог: "
        << countAboveLimit(scoresArray, studentCount, limitValue) << endl;

    delete[] scoresArray;

    return 0;
}