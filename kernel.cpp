__asm("jmp kmain");

#define IDT_TYPE_INTR (0x0E)
#define IDT_TYPE_TRAP (0x0F)
#define GDT_CS (0x8)
#define VIDEO_BUF_PTR (0xb8000)
#define PIC1_PORT (0x20)
#define CURSOR_PORT (0x3D4)
#define VIDEO_WIDTH (80) 
#define VIDEO_HEIGHT (25)
#define NULL 0

typedef void (*intr_handler)();

void keyb_init(void);
void keyb_handler(void);
void keyb_process_keys(void);
void default_intr_handler(void);
void intr_reg_handler(int num, unsigned short segm_sel, unsigned short
flags, intr_handler hndlr);

static inline void outb (unsigned short port, unsigned char data);
static inline void outw(unsigned int port, unsigned int data);
static inline unsigned char inb (unsigned short port);

void on_key(unsigned char scan_code);
void command_handler(void);
void backspace();

const char* inp_err = "Something's wrong.";
int sizeString = 0;
char textFromTerm[64];

int cursor_line = 0;
int cursor_position = 0;

bool is_delim(char c, char *delim)
{
    while(*delim != '\0')
    {
        if(c == *delim)
            return true;
        delim++;
    }
    return false;
}

char *strtok(char *s, char *delim)
{
    static char *p; // start of the next search
    if(!s)
    {
        s = p;
    }
    if(!s)
    {
        // user is bad user
        return NULL;
    }

    // handle beginning of the string containing delims
    while(1)
    {
        if(is_delim(*s, delim))
        {
            s++;
            continue;
        }
        if(*s == '\0')
        {
            return NULL; // we've reached the end of the string
        }
        // now, we've hit a regular character. Let's exit the
        // loop, and we'd need to give the caller a string
        // that starts here.
        break;
    }

    char *ret = s;
    while(1)
    {
        if(*s == '\0')
        {
            p = s; // next exec will return NULL
            return ret;
        }
        if(is_delim(*s, delim))
        {
            *s = '\0';
            p = s + 1;
            return ret;
        }
        s++;
    }
}

int strcmp( const char *s1, const char *s2 )
{
    const unsigned char *p1 = ( const unsigned char * )s1;
    const unsigned char *p2 = ( const unsigned char * )s2;

    while ( *p1 && *p1 == *p2 ) ++p1, ++p2;

    return ( *p1 > *p2 ) - ( *p2  > *p1 );
}

char *strchr(const char *s, int c) {
    while (*s) {
        if (*s == (char)c) {
            return (char *)s; // Возвращаем указатель на найденный символ
        }
        s++;
    }
    // Проверяем также на случай, если искомый символ - это нуль-терминатор
    if (c == '0') {
        return (char *)s;
    }
    return NULL; // Символ не найден
}

void swap(char *a, char *b)
{
    if(!a || !b)
        return;

    char temp = *(a);
    *(a) = *(b);
    *(b) = temp;
}

void reverse(char *str, int length)
{
    int start = 0;
    int end = length -1;
    while (start < end)
    {
        swap((str+start), (str+end));
        start++;
        end--;
    }
}

char* itoa(int num, char* str, int base)
{
    int i = 0;
    bool isNegative = false;

    if (num == 0)
    {
        str[i++] = '0';
        str[i] = '\0';
        return str;
    }

    if (num < 0 && base == 10)
    {
        isNegative = true;
        num = -num;
    }

    while (num != 0)
    {
        int rem = num % base;
        str[i++] = (rem > 9)? (rem-10) + 'a' : rem + '0';
        num = num/base;
    }

    if (isNegative == true)
        str[i++] = '-';

    str[i] = '\0';
    reverse(str, i);
    return str;
}

bool shift_on = 0;
int color = 7;

// Функция переводит курсор на строку strnum (0 – самая верхняя) в позицию pos на этой строке (0 – самое левое положение).
void cursor_moveto(unsigned int strnum, unsigned int pos) {
	unsigned short new_pos = (strnum * VIDEO_WIDTH) + pos;
	outb(CURSOR_PORT, 0x0F);
	outb(CURSOR_PORT + 1, (unsigned char)(new_pos & 0xFF));
	outb(CURSOR_PORT, 0x0E);
	outb(CURSOR_PORT + 1, (unsigned char)( (new_pos >> 8) & 0xFF));
}

void keyb_init() {
	// Регистрация обработчика прерывания
	intr_reg_handler(0x09, GDT_CS, 0x80 | IDT_TYPE_INTR, keyb_handler);
	// segm_sel=0x8, P=1, DPL=0, Type=Intr
	// Разрешение только прерываний клавиатуры от контроллера 8259
	outb(PIC1_PORT + 1, 0xFF ^ 0x02); // 0xFF - все прерывания, 0x02 - бит IRQ1 (клавиатура).
	// Разрешены будут только прерывания, чьи биты установлены в 0
}

void keyb_handler() {
	asm("pusha");
	// Обработка поступивших данных
	keyb_process_keys();
	// Отправка контроллеру 8259 нотификации о том, что прерывание обработано
	outb(PIC1_PORT, 0x20);
	asm("popa; leave; iret");
}

void keyb_process_keys() {
// Проверка что буфер PS/2 клавиатуры не пуст (младший бит присутствует)
	if (inb(0x64) & 0x01)
	{
		unsigned char scan_code;
		unsigned char state;
		scan_code = inb(0x60); // Считывание символа с PS/2 клавиатуры
		int scan_codes[7] = {14, 28, 42, 54, 170, 182, 128};
		if (scan_code < scan_codes[7]) {
			if (scan_code == scan_codes[0]) {
				backspace();
			} else if (scan_code == scan_codes[1]) {
				command_handler();
			} else if (scan_code == scan_codes[2] || scan_code == scan_codes[3]) {
				shift_on= 1;
			} else {
				on_key(scan_code);
			}
		}  else {
			if (scan_code == scan_codes[4] || scan_code == scan_codes[5]) {
				shift_on = 0;
			}
		}
	}
}

struct idt_entry {
	unsigned short base_lo; // Младшие биты адреса обработчика
	unsigned short segm_sel; // Селектор сегмента кода
	unsigned char always0; // Этот байт всегда 0
	unsigned char flags; // Флаги тип. Флаги: P, DPL, Типы - это константы - IDT_TYPE...
	unsigned short base_hi; // Старшие биты адреса обработчика
} __attribute__((packed)); // Выравнивание запрещено

// Структура, адрес которой передается как аргумент команды lidt
struct idt_ptr {
	unsigned short limit;
	unsigned int base;
} __attribute__((packed)); // Выравнивание запрещено

struct idt_entry g_idt[256]; // Реальная таблица IDT
struct idt_ptr g_idtp; // Описатель таблицы для команды lidt

void default_intr_handler() {
	asm("pusha");
	// ... (реализация обработки)
	asm("popa; leave; iret");
}


void intr_reg_handler(int num, unsigned short segm_sel, unsigned short flags, intr_handler hndlr) {
	unsigned int hndlr_addr = (unsigned int) hndlr;
	g_idt[num].base_lo = (unsigned short) (hndlr_addr & 0xFFFF);
	g_idt[num].segm_sel = segm_sel;
	g_idt[num].always0 = 0;
	g_idt[num].flags = flags;
	g_idt[num].base_hi = (unsigned short) (hndlr_addr >> 16);
}

void intr_init() {
	int i;
	int idt_count = sizeof(g_idt) / sizeof(g_idt[0]);
	for(i = 0; i < idt_count; i++)
		intr_reg_handler(i, GDT_CS, 0x80 | IDT_TYPE_INTR, default_intr_handler); 
}

void intr_start() {
	int idt_count = sizeof(g_idt) / sizeof(g_idt[0]);
	g_idtp.base = (unsigned int) (&g_idt[0]);
	g_idtp.limit = (sizeof (struct idt_entry) * idt_count) - 1;
	asm("lidt %0" : : "m" (g_idtp) );
}

void intr_enable() {
	asm("sti");
}

void intr_disable() {
	asm("cli");
}


static inline unsigned char inb (unsigned short port) // Чтение из порта
{
	unsigned char data;
	asm volatile ("inb %w1, %b0" : "=a" (data) : "Nd" (port));
	return data;
}

static inline void outb (unsigned short port, unsigned char data) // Запись
{
	asm volatile ("outb %b0, %w1" : : "a" (data), "Nd" (port));
}

static inline void outw(unsigned int port, unsigned int data) {
	asm volatile ("outw %w0, %w1" : : "a" (data), "Nd" (port));
}

int unix_scroll(int count) {
	unsigned char *video_buf = (unsigned char *)VIDEO_BUF_PTR;
	int lines = cursor_line + count - VIDEO_HEIGHT + 1;
	for (int i = 0; i < lines; i++) {
		for (int j = 0; j < VIDEO_WIDTH * 2 * (VIDEO_HEIGHT - 1); j++) {
			video_buf[j] = video_buf[j + VIDEO_WIDTH * 2];
		}
		for (int j = 0; j < VIDEO_WIDTH * 2; j++) {
			video_buf[VIDEO_WIDTH * 2 * (VIDEO_HEIGHT - 1) + i] = 0;
		}
	}
	return lines;
}


void out_str(int color, const char* ptr, unsigned int strnum) {
	unsigned char* video_buf = (unsigned char*) VIDEO_BUF_PTR;
	video_buf = video_buf + VIDEO_WIDTH * 2 * strnum;
	while (*ptr)
	{
		video_buf[0] = (unsigned char) *ptr; // Символ (код)
		video_buf[1] = color; // Цвет символа и фона
		video_buf = video_buf + 2;
		ptr++;
	}
}

void out_chr(unsigned char symbol) {
	unsigned char* video_buf = (unsigned char*) VIDEO_BUF_PTR;
	video_buf =  video_buf + 2 * VIDEO_WIDTH * cursor_line + cursor_position * 2;
	video_buf[0] = symbol;
	video_buf[1] = color;
	cursor_moveto(cursor_line, ++cursor_position);
}

unsigned char symbol_on_key(unsigned char scan_code, int shift_on) {
	if (shift_on == 1) {
		switch(scan_code) {
			case 1:
				return '~';
			case 2:
				return '!';
			case 3:
				return '@';
			case 4:
				return '#';
			case 5:
				return '$';
			case 6:
				return '%';
			case 7:
				return '^';
			case 8:
				return '&';
			case 9:
				return '*';
			case 10:
				return '(';
			case 11:
				return ')';
			case 12:
				return '_';
			case 13:
				return '+';
			case 16:
				return 'Q';
			case 17:
				return 'W';
			case 18:
				return 'E';
			case 19:
				return 'R';
			case 20:
				return 'T';
			case 21:
				return 'Y';
			case 22:
				return 'U';
			case 23:
				return 'I';
			case 24:
				return 'O';
			case 25:
				return 'P';
			case 26:
				return '{';
			case 27:
				return '}';
			case 28:
				return '|';
			case 30:
				return 'A';
			case 31:
				return 'S';
			case 32:
				return 'D';
			case 33:
				return 'F';
			case 34:
				return 'G';
			case 35:
				return 'H';
			case 36:
				return 'J';
			case 37:
				return 'K';
			case 38:
				return 'L';
			case 39:
				return ':';
			case 40:
				return '\"';
			case 41:
				return '~';
			case 43:
				return '|';
			case 44:
				return 'Z';
			case 45:
				return 'X';
			case 46:
				return 'C';
			case 47:
				return 'V';
			case 48:
				return 'B';
			case 49:
				return 'N';
			case 50:
				return 'M';
			case 51:
				return '<';
			case 52:
				return '>';
			case 53:
				return '?';
			case 57:
				return ' ';
			default:
				return 0;
		}
	} else if (shift_on == 0) {
		switch(scan_code) {
			case 1:
				return '`';
			case 2:
				return '1';
			case 3:
				return '2';
			case 4:
				return '3';
			case 5:
				return '4';
			case 6:
				return '5';
			case 7:
				return '6';
			case 8:
				return '7';
			case 9:
				return '8';
			case 10:
				return '9';
			case 11:
				return '0';
			case 12:
				return '-';
			case 13:
				return '-';
			case 16:
				return '1';
			case 17:
				return 'w';
			case 18:
				return 'e';
			case 19:
				return 'r';
			case 20:
				return 't';
			case 21:
				return 'y';
			case 22:
				return 'u';
			case 23:
				return 'i';
			case 24:
				return 'o';
			case 25:
				return 'p';
			case 26:
				return '[';
			case 27:
				return ']';
			case 30:
				return 'a';
			case 31:
				return 's';
			case 32:
				return 'd';
			case 33:
				return 'f';
			case 34:
				return 'g';
			case 35:
				return 'h';
			case 36:
				return 'j';
			case 37:
				return 'k';
			case 38:
				return 'l';
			case 39:
				return ';';
			case 40:
				return '\'';
			case 41:
				return '`';
			case 43:
				return '\\';
			case 44:
				return 'z';
			case 45:
				return 'x';
			case 46:
				return 'c';
			case 47:
				return 'v';
			case 48:
				return 'b';
			case 49:
				return 'n';
			case 50:
				return 'm';
			case 51:
				return ',';
			case 52:
				return '.';
			case 53:
				return '/';
			case 57:
				return ' ';
			default:
				return 0;
		}
	}
}

void on_key(unsigned char scan_code) {
	intr_disable();
	out_chr(symbol_on_key(scan_code, shift_on));
	intr_enable();
	sizeString += 1;
}

void info(void) {
	out_str(color, "CalcOS: v.01. Developer: Sitnikov Maxim, 5131001/20503, SpbPU, 2024", cursor_line++);
	out_str(color, "Compilers: bootloader: fasm, kernel: gcc", cursor_line++);
	char numsInChar[6] = {1, 2, 3, 4, 5, 6};
	char colors[6][7] = {"green", "blue", "red", "yellow", "gray", "white"};
	unsigned int color_info = *(unsigned int *) 0x9000;
	for (int i = 1; i <= 6; i++) {
		if (color_info == i + 48) {
			out_str(color, "Bootloader parameters: ", cursor_line++);
			out_str(color, colors[i - 1], cursor_line++);
			break;
		}
	}	
}

void clean(void) {
    unsigned char *video_buf = (unsigned char *)VIDEO_BUF_PTR;
    for (int i = 0; i <= VIDEO_WIDTH * VIDEO_HEIGHT * 2; i++)
        video_buf[i] = 0;
    cursor_line = 0;
    cursor_position = 0;
    sizeString = 0;
}

int pos;
#define INT_MAX 2147483647
#define INT_MIN -2147483648

int parse_expression(char *expr);
int flag_parser = 0;

int parse_number(char *expr) {
    int num = 0;
    int sign = 1;

    while (expr[pos] == '-') {
        sign = -sign;
        pos++;
    }

    while (expr[pos] - '0' >= 0 && expr[pos] - '0' <= 9) {//isdigit(expr[pos])) {
        int digit = expr[pos] - '0';
        if (num > (INT_MAX - digit) / 10) {
            flag_parser = 1;
            return 0;
        }
        num = num * 10 + digit;
        pos++;
    }

    return sign * num;
}

int parse_factor(char *expr) {
    if (expr[pos] == '(') {
        pos++; // пропускаем '('
        int result = parse_expression(expr);
        if (expr[pos] != ')') {
            flag_parser = 2;
            return 0;
        }
        pos++; // пропускаем ')'
        return result;
    } else {
        return parse_number(expr);
    }
}

int parse_term(char *expr) {
    int result = parse_factor(expr);

    while (expr[pos] == '*' || expr[pos] == '/') {
        char op = expr[pos++];
        int next_factor = parse_factor(expr);
        if (next_factor == 0) {
        	return 0;
        }
        if (op == '*') {
            if (result > (INT_MAX / next_factor)) {
                flag_parser = 1;
                return 0;
            }
            result *= next_factor;
        } else { // op == '/'
            if (next_factor == 0) {
                flag_parser = 3;
                return 0;
            }
            result /= next_factor;
        }
    }

    return result;
}

int parse_expression(char *expr) {
    int result = parse_term(expr);
    if (flag_parser == 1) {
		out_str(color, "Error!", cursor_line++);
	} else if (flag_parser == 2) {
		out_str(color, "Error!", cursor_line++);
	} else if (flag_parser == 3) {
		out_str(color, "Error!", cursor_line++);
	}
    while (expr[pos] == '+' || expr[pos] == '-') {
    	if (flag_parser == 1) {
    		out_str(color, "Error!", cursor_line++);
    	} else if (flag_parser == 2) {
    		out_str(color, "Error!", cursor_line++);
    	} else if (flag_parser == 3) {
    		out_str(color, "Error!", cursor_line++);
    	}
        char op = expr[pos++];
        int next_term = parse_term(expr);
        char out[10];
        if (op == '+') {
            if (result > INT_MAX - next_term) {
                flag_parser = 1;
            }
            result += next_term;
        } else { // op == '-'
            if (result < INT_MIN + next_term) {
                flag_parser = 1;
            }
            result -= next_term;
        }
    }
    return result;
}

void command_handler(void) {
	intr_disable();
	unsigned char *video_buf = (unsigned char *)VIDEO_BUF_PTR;
    video_buf = video_buf + 2 * VIDEO_WIDTH * (cursor_line++) + 2;
    int i;
    for(int i = 0; i < sizeString; i++)
    {
        textFromTerm[i] = video_buf[0];
        video_buf += 2;
        if (i == sizeString - 1) {
        	textFromTerm[i + 1] = '\0';
        }
    }

	cursor_position = 0;

    char *token = strtok(textFromTerm, " ");
    if(!strcmp(token, "shutdown") || !strcmp(token, "SHUTDOWN")) {
    	if (cursor_line + 1 >= VIDEO_HEIGHT) {
    		int str_scrolled = unix_scroll(1);
    		cursor_line -= str_scrolled;
    	}
    	outw(0x604, 0x2000);
    }
    else if(!strcmp(token, "clear") || !strcmp(token, "CLEAR")) {
    	if (cursor_line + 1 >= VIDEO_HEIGHT) {
    		int str_scrolled = unix_scroll(1);
    		cursor_line -= str_scrolled;
    	}
    	clean();
    }
    else if(!strcmp(token, "info") || !strcmp(token, "INFO")) {
    	if (cursor_line + 5 >= VIDEO_HEIGHT) {
    		int str_scrolled = unix_scroll(5);
    		cursor_line -= str_scrolled;
    	}
    	info();
    }
    else if(!strcmp(token, "expr") || !strcmp(token, "EXPR")) {
    	if (cursor_line + 2 >= VIDEO_HEIGHT) {
    		int str_scrolled = unix_scroll(2);
    		cursor_line -= str_scrolled;
    	}
    	pos = 0;
    	flag_parser = 0;
    	token = strtok(NULL, " ");
    	int result = parse_expression(token);
    	//intr_enable();
    	char out[10];
    	out_str(color, itoa(result, out, 10), cursor_line++);
	}
    else {
        out_str(color, "Command incorrect!", cursor_line++);
    }
    out_chr('#');
	cursor_moveto(cursor_line, cursor_position);

	intr_enable();
}

void backspace() {
    unsigned char *video_buf = (unsigned char *)VIDEO_BUF_PTR;
    video_buf = video_buf + (VIDEO_WIDTH * 2 * cursor_line) + (2 * (--cursor_position));
    video_buf[0] = 0;
    video_buf[1] = color;
    cursor_moveto(cursor_line, cursor_position);
    sizeString -= 1;
}

void choose_color() {
	unsigned int* colorNum = (unsigned int *) 0x9000;
	char colors[6] = {'1', '2', '3', '4', '5', '6'};
	if (*colorNum == colors[0]) {
		color = 2;
	} else if (*colorNum == colors[1]) {
		color = 1;
	} else if (*colorNum == colors[2]) {
		color = 4;
	} else if (*colorNum == colors[3]) {
		color = 6;
	} else if (*colorNum == colors[4]) {
		color = 8;
	} else if (*colorNum == colors[5]) {
		color = 7;
	} else {
		outw(0x604, 0x2000);
	}
}

extern "C" int kmain() {

	// Cleaning previous mess and changing color based on input from loader
	clean();
	choose_color();

	intr_disable();
	intr_init();
    keyb_init();
    intr_start();
    intr_enable();

	out_chr('#');
	cursor_moveto(cursor_line, cursor_position);

	while(1) {
		asm("hlt");
	}
}