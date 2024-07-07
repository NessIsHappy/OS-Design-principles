use16
org 0x7C00
start:
    mov ax, cs ; Сохранение адреса сегмента кода в ax
    mov ds, ax ; Сохранение этого адреса как начало сегмента данных
    mov ss, ax ; И сегмента стека
    mov sp, start ; Сохранение адреса стека как адрес первой инструкции этого кода. Стек будет расти вверх и не перекроет код.
    mov ah, 0x0e ; В ah номер функции BIOS: 0x0e - вывод символа на активную видео страницу (эмуляция телетайпа)
    mov al, 'H' ; В al помещается код символа
    int 0x10 ; Вызывается прерывание. Обработчиком является код BIOS. Символ будет выведен на экран.
    mov al, 'e'
    int 0x10
    mov al, 'l'
    int 0x10
    int 0x10
    mov al, 'o'
    int 0x10

inf_loop:
    jmp inf_loop ; Бесконечный цикл
    ; Внимание! Сектор будет считаться загрузочным, если содержит в конце своих 512 байтов два следующих байта: 0x55 и 0xAA
    times (512 - ($ - start) - 2) db 0 ; Заполнение нулями до границы 512 - 2 текущей точки
    db 0x55, 0xAA ; 2 необходимых байта чтобы сектор считался загрузочным