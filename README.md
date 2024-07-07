# OS-Design-principles
Лабораторная работа 1
ПРИНЦИПЫ РАЗРАБОТКИ ОПЕРАЦИОННЫХ СИСТЕМ
Цель работы — изучение основ разработки ОС, принципов
низкоуровневого взаимодействия с аппаратным обеспечением,
программирования системной функциональности и процесса загрузки
системы.

Порядок выполнения работы
1. Получить у преподавателя номер задания (Приложение).
2. Скомпилировать и запустить на эмуляторе пример загрузочного
сектора start.asm, описанного в Теоретических сведениях, указанным в
вашем варианте транслятором.
3. Пользуясь примерами из Теоретических сведений, разработать
простой загрузчик для загрузки минимального ядра. Скомпилировать
загрузчик и минимальное ядро указанными в вашем варианте задания
транслятором и компилятором языка Си. Проверить работоспособность
загрузчика и ядра на эмуляторе.
4. Разработать указанные в вашем варианте задания функции
загрузчика.
5. Расширить реализацию минимального ядра, добавив в него
функции, перечисленные в задании.
6. Предложить не менее 15 тестов для проверки работоспособности
ОС (команд или иных входных данных, поступающих от пользователя).
Протестировать вашу реализацию на предложенных тестах.
7. Ответить на контрольные вопросы.

DictOS
Реализация простого словаря-переводчика с английского языка на указанный язык.
Загрузчик: взаимодействует с пользователем, позволяя пользователю указать — слова на
какие буквы английского языка его будут интересовать после запуска ОС. Загрузчик
предлагает пользователю разрешить использовать слова на указанную букву или
запретить использовать их путем однократного нажатия на соответствующую клавишу.
При этом на экране указывается какие буквы отмечены, а какие нет:
abcde_____kl_nop____uv_x_z
При нажатии на клавишу, например 'w' экран обновляется:
abcde_____kl_nop____uvwx_z
Для старта ОС пользователь нажимает Enter.
Словарь может храниться в секции данных ядра (можно объявить его прямо в коде как
статические данные). Словарь должен быть отсортирован. Допускается хранение словаря
в неотсортированном виде, но при этом сортировка выполняется при запуске ОС.
Словарь должен содержать не менее 50 общеупотребительных слов. Артикли в словах
указывать не требуется.
**Доп. задание: Словарь должен содержать не менее 500 общеупотребительных пар слов.
Для поиска в словаре обязательно должен применяться бинарный поиск.
Поддерживаемые ОС команды:
info
Выводит информацию об авторе и средствах разработки ОС (транслятор ассемблера,
компилятор), заданные в загрузчике параметры — перечень букв, слова на которые
должны обрабатываться при запросе перевода.
dictinfo
выводит информацию о загруженном словаре (язык, общее кол-во слов, количество
доступных для перевода слова — вычислено на основании заданных в загрузчике
данных). Пример:
# dictinfo
Dictionary: en -> es
Number of words: 1121
Number of loaded words: 780
translate слово
Переводит слово с английского языка на другой. Если слово не найдено или не загружено
— выводит ошибку. При поиске слова выполняется бинарный поиск в массиве слов
словаря.
**Доп. задание: При выводе перевода некоторые специфические буквы языка должны
выводится корректно (для испанского языка — буква ñ; для финского — буквы ä, ö). В
словаре обязательно должны быть слова с такими буквами.
Пример:
# translate cat
gato
# translate airport
aeropuerto
# translate moonlight
Error: A word 'moonlight' is unknown
wordstat буква
Выводит количество загруженных слов в словаре на указанную букву.
# wordstat z
Letter 'z': 57 words loaded.
# wordstat y
Letter 'z': 1 word loaded.
# wordstat d
Letter 'd': 0 words loaded.
shutdown
Выключение компьютера.
**Доп задание:
anyword буква
Выводит случайное слово на указанную букву и его перевод. Буква может быть не указана
– тогда выводится случайное слово на любую букву. Алгоритм генерации случайных или
псевдослучайных чисел определяется студентом и должен быть описан в отчете.
Примеры:
# anyword n
night: noche
# anyword z
Error: no words
# anyword n
nice: bonito
# anyword
boy: muchacho