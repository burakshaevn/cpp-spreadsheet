# Электронна таблица

## Описание
Реализована электронная таблица с поддержкой текстовых и формульных ячеек. Таблица позволяет выполнять вычисления на основе формул, поддерживает ссылки на другие ячейки и автоматическое обновление зависимостей.

Основные возможности:
- Поддержка текстовых и формульных ячеек.
- Вычисление выражений и использование ссылок на другие ячейки.
- Обработка ошибок вычислений, таких как:
- `#VALUE!` — возникает, если неверный тип данных используется в арифметических операциях.
- `#REF!` — если ячейка ссылается на несуществующую ячейку.
- `#ARITHM!` — ошибки, связанные с арифметикой, например, деление на ноль.
- `Circular Dependency Error` — если возникает циклическая зависимость между ячейками.
- Оптимизация через кэширование вычислений.
- Проверка циклических ссылок между ячейками и предупреждение о них.
- Поддержка базовых арифметических операций и функций.

Из интересного в коде:
- **Использование идиомы Pimpl**: используется для сокрытия деталей реализации, улучшения инкапсуляции и уменьшения времени компиляции. В проекте это реализовано в классе `Cell`.
- **Умные указатели**: использование идиомы Pimpl не может обойтись без умных указателей: умный указатель `std::unique_ptr<Impl>` используется в классе `Cell` используется для автоматического удаления объекта Impl, когда ячейка уничтожается.
- **Граф зависимостей ячеек**: таблица поддерживает сложные зависимости между ячейками. При изменении значения одной ячейки автоматически обновляются все зависящие от неё ячейки. Для этого используется система отслеживания зависимостей, построенная на графах. Например, граф зависимостей управляется через методы:
   `AddDependency()` — добавляет зависимость ячейки от другой.
  `RemoveDependency()` — удаляет зависимость.
  Это позволяет оптимизировать пересчёт значений и гарантировать корректную работу с зависимостями.
- **Проверка циклических зависимостей**: как и MS Excel, мой проект реализует проверку на наличие циклических зависимостей между ячейками. Это необходимо, чтобы избежать бесконечного пересчёта значений в случае, если одна ячейка напрямую или косвенно ссылается сама на себя через другие ячейки. Например, метод `HasCircularDependency()` помогает обнаруживать такие циклы.
- **Кэширование вычисленных значений**: для повышения производительности используются механизмы кэширования вычисленных значений ячеек. При изменении значения ячейки её кэш сбрасывается, и только необходимые ячейки пересчитываются, что позволяет избежать лишних вычислений.

## Примеры
<details>

<summary>Пример 1. Простое использование текста в ячейках</summary>

```cpp
auto sheet = CreateSheet();

sheet->SetCell("A1"_pos, "Hello");
sheet->SetCell("A2"_pos, "World");
sheet->SetCell("B1"_pos, "123");

std::cout << sheet->GetCell("A1"_pos)->GetText();  
std::cout << sheet->GetCell("A2"_pos)->GetText();  
std::cout << sheet->GetCell("B1"_pos)->GetValue();  
```
Выходной поток:
```
>> HelloWorld123
```
</details>

<details>

<summary>Пример 2. Простая арифметическая формула</summary>
Можно задать формулы в ячейках, которые будут ссылаться на другие ячейки и выполнять арифметические операции.

```cpp
auto sheet = CreateSheet();

sheet->SetCell("A1"_pos, "2");
sheet->SetCell("A2"_pos, "3");
sheet->SetCell("A3"_pos, "=A1 + A2");

std::cout << std::get<double>(sheet->GetCell("A3"_pos)->GetValue());
```
Выходной поток:
```
>> 5
```
</details>

<details>

<summary>Пример 3. Форматирование и ссылки на ячейки</summary>

Формулы автоматически форматируются при вводе, а ссылки на ячейки могут повторяться.

```cpp
auto sheet = CreateSheet();

sheet->SetCell("A1"_pos, "= 2 + 2 ");
std::cout << sheet->GetCell("A1"_pos)->GetText() << std::endl;

sheet->SetCell("B1"_pos, "=A1 + A1");
std::cout << std::get<double>(sheet->GetCell("B1"_pos)->GetValue()) << std::endl;
```
Выходной поток:
```
>> =2+2
>> 8
```
</details>
<details>
<summary>Пример 4. Более сложный пример</summary> 
   
```cpp
 auto sheet = CreateSheet();

 sheet->SetCell("A1"_pos, "5");
 sheet->SetCell("A2"_pos, "3");
 sheet->SetCell("A3"_pos, "7");

 sheet->SetCell("B1"_pos, "=A1 * A2 + A3");       
 sheet->SetCell("B2"_pos, "=(A1 + A2) / A3");     
 sheet->SetCell("B3"_pos, "=B1 * B2 - A1");          
 sheet->SetCell("B4"_pos, "=B3 + (A2 - A3) * 2");    

 // комбинируем несколько ячеек в одну формулу
 sheet->SetCell("C1"_pos, "=A1 + B1 + B2 + B3 + B4");  
 
 // выводим её значение
 std::cout << std::get<double>(sheet->GetCell("C1"_pos)->GetValue()) << std::endl;
```
Выходной поток. Результат может отличаться из за особенностей округления. 
```
>> 60.4286
```
</details>

## Сборка и запуск
..

## Структура
- sheet.h / sheet.cpp — реализация таблицы и управления ячейками.
- cell.h / cell.cpp — класс ячейки, включая различные типы ячеек: текстовые, формульные и пустые.
- formula.h / formula.cpp — парсинг и вычисление формул.
- common.h — общие типы и утилиты, используемые в проекте.
- CMakeLists.txt — конфигурационный файл для сборки проекта.
