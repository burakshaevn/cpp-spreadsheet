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

#### 1. Использование идиомы Pimpl 
Pointer to IMPLementation используется для сокрытия деталей реализации, улучшения инкапсуляции и уменьшения времени компиляции. В проекте это реализовано в классе `Cell`.
#### 2. Умные указатели 
Использование идиомы Pimpl не может обойтись без умных указателей: умный указатель `std::unique_ptr<Impl>` используется в классе `Cell` используется для автоматического удаления объекта Impl, когда ячейка уничтожается.
#### 3. Граф зависимостей ячеек
Таблица поддерживает сложные зависимости между ячейками. При изменении значения одной ячейки автоматически обновляются все зависящие от неё ячейки. Для этого используется система отслеживания зависимостей, построенная на графах. Например, граф зависимостей управляется через методы:
- `AddDependency()` — добавляет зависимость ячейки от другой.
- `RemoveDependency()` — удаляет зависимость.
Это позволяет оптимизировать пересчёт значений и гарантировать корректную работу с зависимостями.
#### 4. Проверка циклических зависимостей
Как и MS Excel, мой проект реализует проверку на наличие циклических зависимостей между ячейками. Это необходимо, чтобы избежать бесконечного пересчёта значений в случае, если одна ячейка напрямую или косвенно ссылается сама на себя через другие ячейки. Например, метод `HasCircularDependency()` помогает обнаруживать такие циклы.
#### 5. Кэширование вычисленных значений
Для повышения производительности используются механизмы кэширования вычисленных значений ячеек. При изменении значения ячейки её кэш сбрасывается, и только необходимые ячейки пересчитываются, что позволяет избежать лишних вычислений.

## Пример
..
## Сборка и запуск
..

## Структура
- sheet.h / sheet.cpp — реализация таблицы и управления ячейками.
- cell.h / cell.cpp — класс ячейки, включая различные типы ячеек: текстовые, формульные и пустые.
- formula.h / formula.cpp — парсинг и вычисление формул.
- common.h — общие типы и утилиты, используемые в проекте.
- CMakeLists.txt — конфигурационный файл для сборки проекта.
