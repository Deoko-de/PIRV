#include <iostream>
#include <vector>
#include <fstream>

using namespace std;

#pragma pack(push, 1)
struct FileHeader
{
    char signature[4];
    int version;
    int studentCount;
};
#pragma pack(pop)

const char Expected[4] = { 'G','R','D','1' };

void enterMarks(vector<vector<double>>& marksGrid)
{
    for (int i = 0; i < marksGrid.size(); i++)
    {
        cout << "\nОбучающийся " << i + 1 << ":\n";
        for (int j = 0; j < marksGrid[i].size(); j++)
        {
            double currentValue;

            while (true)
            {
                cout << "  Отметка по дисциплине " << j + 1 << " (0-5): ";
                cin >> currentValue;

                if (currentValue < 0.0 || currentValue > 5.0)
                {
                    cout << "Некорректное значение. Попробуйте снова.\n";
                }
                else
                {
                    marksGrid[i][j] = currentValue;
                    break;
                }
            }
        }
    }
}

void storeToFile(const string& filePath,
    const vector<vector<double>>& marksGrid)
{
    ofstream outputStream(filePath, ios::binary);

    if (!outputStream)
    {
        cout << "Не удалось открыть файл для записи\n";
        return;
    }

    FileHeader header;
    for (int i = 0; i < 4; i++) {
        header.signature[i] = Expected[i];
    }
    header.version = 1;
    header.studentCount = marksGrid.size();

    cout << "\nРазмер структуры заголовка: "
        << sizeof(FileHeader) << " байт\n";

    outputStream.write(reinterpret_cast<char*>(&header), sizeof(header));

    int subjectCount = marksGrid[0].size();
    outputStream.write(reinterpret_cast<char*>(&subjectCount), sizeof(subjectCount));

    for (const auto& individual : marksGrid)
    {
        outputStream.write(reinterpret_cast<const char*>(individual.data()),
            subjectCount * sizeof(double));
    }

    outputStream.close();
    cout << "Информация успешно сохранена в файл\n";
}

void retrieveFromFile(const string& filePath)
{
    ifstream inputStream(filePath, ios::binary);

    if (!inputStream)
    {
        cout << "Не удалось открыть файл для чтения\n";
        return;
    }

    FileHeader header;
    inputStream.read(reinterpret_cast<char*>(&header), sizeof(header));

    if (!equal(header.signature,
        header.signature + 4,
        Expected))
    {
        cout << "Ошибка: неверная сигнатура файла\n";
        return;
    }

    int subjectCount;
    inputStream.read(reinterpret_cast<char*>(&subjectCount), sizeof(subjectCount));

    vector<vector<double>> marksGrid(header.studentCount,
        vector<double>(subjectCount));

    for (int i = 0; i < header.studentCount; i++)
    {
        inputStream.read(reinterpret_cast<char*>(marksGrid[i].data()),
            subjectCount * sizeof(double));
    }

    cout << "\nИнформация успешно загружена из файла\n";
    cout << "Версия файла: " << header.version << endl;
    cout << "Число обучающихся: " << header.studentCount << endl;
    cout << "Количество дисциплин: " << subjectCount << endl;

    cout << "\nОтметки:\n";
    for (int i = 0; i < marksGrid.size(); i++)
    {
        cout << "Обучающийся " << i + 1 << ": ";
        for (double mark : marksGrid[i])
            cout << mark << " ";
        cout << endl;
    }

    inputStream.close();
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
        cout << "Ошибка: значения должны быть положительными\n";
        return 1;
    }

    vector<vector<double>> marksGrid(studentsNum, vector<double>(subjectsNum));

    enterMarks(marksGrid);

    storeToFile("C:\\Users\\Deoko\\source\\repos\\PiRV\\Dz_2sem\\2sem_5\\grades.bin", marksGrid);

    retrieveFromFile("grades.bin");

    return 0;
}