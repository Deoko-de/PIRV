#include <iostream>
#include <vector>
#include <algorithm>
#include <cmath>

using namespace std;


void enterGrades(vector<vector<double>>& gradesGrid, int studentsNum, int subjectsNum)
{
    for (int i = 0; i < studentsNum; i++)
    {
        cout << "\nОбучающийся " << i + 1 << ":\n";
        for (int j = 0; j < subjectsNum; j++)
        {
            double currentMark;
            while (true)
            {
                cout << "Отметка по дисциплине " << j + 1 << " (пятибалльная шкала):\n";
                cin >> currentMark;

                if (currentMark < 0.0 || currentMark > 5.0)
                {
                    cout << "Некорректное значение, повторите ввод.\n";
                }
                else
                {
                    gradesGrid[i][j] = currentMark;
                    break;
                }
            }
        }
    }
}

double calculatePersonAverage(const vector<double>& personMarks)
{
    double total = 0.0;
    for (double mark : personMarks)
        total += mark;

    return total / personMarks.size();
}

vector<double> calculateAllPersonsAverage(const vector<vector<double>>& gradesGrid)
{
    vector<double> averageValues;

    for (const auto& individual : gradesGrid)
        averageValues.push_back(calculatePersonAverage(individual));

    return averageValues;
}

vector<double> calculateSubjectAverages(const vector<vector<double>>& gradesGrid, int subjectsNum)
{
    vector<double> subjectAverages(subjectsNum, 0.0);

    for (int j = 0; j < subjectsNum; j++)
    {
        for (size_t i = 0; i < gradesGrid.size(); i++)
            subjectAverages[j] += gradesGrid[i][j];

        subjectAverages[j] /= gradesGrid.size();
    }

    return subjectAverages;
}

int findTopPerformer(const vector<double>& averageValues)
{
    int topIndex = 0;

    for (size_t i = 1; i < averageValues.size(); i++)
        if (averageValues[i] > averageValues[topIndex])
            topIndex = i;

    return topIndex;
}

int main()
{
    setlocale(LC_ALL, "Russian");
    int studentsNum, subjectsNum;

    cout << "Укажите число обучающихся: ";
    cin >> studentsNum;

    cout << "Укажите количество дисциплин: ";
    cin >> subjectsNum;

    if (studentsNum <= 0 || subjectsNum <= 0)
    {
        cout << "Ошибка - значения должны быть положительными\n";
        return 1;
    }

    vector<vector<double>> gradesGrid(studentsNum, vector<double>(subjectsNum));

    enterGrades(gradesGrid, studentsNum, subjectsNum);

    vector<double> averageValues = calculateAllPersonsAverage(gradesGrid);
    vector<pair<int, double>> learners;

    for (size_t i = 0; i < averageValues.size(); i++)
    {
        learners.push_back({ static_cast<int>(i), averageValues[i] });
    }

    double epsilon = 1e-9;

    sort(learners.begin(), learners.end(), [epsilon](const pair<int, double>& first,
        const pair<int, double>& second)
        {
            if (fabs(first.second - second.second) > epsilon)
                return first.second > second.second;

            return first.first < second.first;
        });

    cout << "\nРейтинговый список обучающихся:\n";

    for (const auto& learner : learners)
    {
        cout << "Обучающийся " << learner.first + 1
            << " - Средний показатель: "
            << learner.second << endl;
    }

    vector<double> subjectAverages = calculateSubjectAverages(gradesGrid, subjectsNum);

    cout << "\nПерсональные средние показатели:\n";
    for (size_t i = 0; i < averageValues.size(); i++)
        cout << "Обучающийся " << i + 1 << ": "
        << averageValues[i] << endl;

    cout << "\nСредние показатели по дисциплинам:\n";
    for (size_t j = 0; j < subjectAverages.size(); j++)
        cout << "Дисциплина " << j + 1 << ": "
        << subjectAverages[j] << endl;

    int topPerformer = findTopPerformer(averageValues);
    cout << "\nЛучший обучающийся по среднему баллу: "
        << topPerformer + 1
        << " ("
        << averageValues[topPerformer] << ")\n";

    double limitValue;
    cout << "\nЗадайте пороговое значение среднего балла: ";
    cin >> limitValue;

    int honorsCount = 0;
    int dangerCount = 0;

    for (const auto& learner : learners)
    {
        if (learner.second >= 4.5)
            honorsCount++;

        if (learner.second < 3.0)
            dangerCount++;
    }

    cout << "\nЧисло отличников: " << honorsCount << endl;

    cout << "Количество обучающихся в зоне риска: " << dangerCount << endl;

    learners.erase(
        remove_if(learners.begin(), learners.end(),
            [limitValue](const pair<int, double>& learner)
            {
                return learner.second < limitValue;
            }),
        learners.end()
    );

    cout << "\nПеречень после отсеивания:\n";

    for (const auto& learner : learners)
    {
        cout << "Обучающийся " << learner.first + 1
            << " - Средний показатель: "
            << learner.second << endl;
    }

    return 0;
}