__asm("jmp kmain");

#define IDT_TYPE_INTR (0x0E)
#define IDT_TYPE_TRAP (0x0F)
// Селектор секции кода, установленный загрузчиком ОС
#define GDT_CS (0x8)

#define VIDEO_BUF_PTR (0xb8000)
#define PIC1_PORT (0x20)

#define CURSOR_PORT (0x3D4)
#define VIDEO_WIDTH (80) 
#define VIDEO_HEIGHT (25)

#define COLOR_ADDRESS 0x9000

#define NULL 0

unsigned char lower_case_codes[58] = 
{
    0, '`', '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', 0, 0,
    'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', 0, 0,
    'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`', 0,
    '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0, 0, 0, ' '
};

unsigned char upper_case_codes[58] = 
{
    0, '~', '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+', 0, 0,
    'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', '|', 0,
    'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', '\"', '~', 0,
    '|', 'Z', 'X', 'C', 'V', 'B', 'N', 'M', '<', '>', '?', 0, 0, 0, ' '
};

enum Colors {
	green = '1',
	blue,
	red,
	yellow,
	gray,
	white
};

typedef void (*intr_handler)();

static inline void outb (unsigned short port, unsigned char data);
static inline unsigned char inb (unsigned short port);

void keyb_init(void);
void keyb_handler(void);
void keyb_process_keys(void);
void default_intr_handler(void);
void intr_reg_handler(int num, unsigned short segm_sel, unsigned short
flags, intr_handler hndlr);
void clean(void);
void out_chr(unsigned char c);
void on_key(unsigned char scan_code);
void read_and_exec_comand(void);
void help(void);
void info(void);
int strcmp (const char *a_, const char *b_);
char *strchr (const char *string, int c_);
char *strtok_r (char *s, const char *delimiters, char **save_ptr);
char* strtok(char *s, const char *delimiters);
void backspace_pressed(void);
void reverse(char str[], int length);
char* citoa(int num, char* str, int base);


//global vars
const char* inp_err = "Error: Wrong input!";
char input_string[80];
int input_len = 0;
bool shift_pressed = 0;
int cursor_line = 0;
int cursor_position = 0;
int color = 7;

void reverse(char str[], int length)
{
    int start = 0;
    int end = length - 1;
    while (start < end) {
        char temp = str[start];
        str[start] = str[end];
        str[end] = temp;
        end--;
        start++;
    }
}
// Implementation of citoa()
char* citoa(int num, char* str, int base)
{
    int i = 0;
    bool isNegative = false;
 
    /* Handle 0 explicitly, otherwise empty string is
     * printed for 0 */
    if (num == 0) {
        str[i++] = '0';
        str[i] = '\0';
        return str;
    }
 
    // In standard itoa(), negative numbers are handled
    // only with base 10. Otherwise numbers are
    // considered unsigned.
    if (num < 0 && base == 10) {
        isNegative = true;
        num = -num;
    }
 
    // Process individual digits
    while (num != 0) {
        int rem = num % base;
        str[i++] = (rem > 9) ? (rem - 10) + 'a' : rem + '0';
        num = num / base;
    }
 
    // If number is negative, append '-'
    if (isNegative)
        str[i++] = '-';
 
    str[i] = '\0'; // Append string terminator
 
    // Reverse the string
    reverse(str, i);
 
    return str;
}

char *
strchr (const char *string, int c_) 
{
  char c = c_;

  // ASSERT (string != NULL);

  for (;;) 
    if (*string == c)
      return (char *) string;
    else if (*string == '\0')
      return NULL;
    else
      string++;
}

char *
strtok_r (char *s, const char *delimiters, char **save_ptr) 
{
  char *token;
  
  // ASSERT (delimiters != NULL);
  // ASSERT (save_ptr != NULL);

  /* If S is nonnull, start from it.
     If S is null, start from saved position. */
  if (s == NULL)
    s = *save_ptr;
  // ASSERT (s != NULL);

  /* Skip any DELIMITERS at our current position. */
  while (strchr (delimiters, *s) != NULL) 
    {
      /* strchr() will always return nonnull if we're searching
         for a null byte, because every string contains a null
         byte (at the end). */
      if (*s == '\0')
        {
          *save_ptr = s;
          return NULL;
        }

      s++;
    }

  /* Skip any non-DELIMITERS up to the end of the string. */
  token = s;
  while (strchr (delimiters, *s) == NULL)
    s++;
  if (*s != '\0') 
    {
      *s = '\0';
      *save_ptr = s + 1;
    }
  else 
    *save_ptr = s;
  return token;
}

int
strcmp (const char *a_, const char *b_) 
{
  const unsigned char *a = (const unsigned char *) a_;
  const unsigned char *b = (const unsigned char *) b_;

  // ASSERT (a != NULL);
  // ASSERT (b != NULL);

  while (*a != '\0' && *a == *b) 
    {
      a++;
      b++;
    }

  return *a < *b ? -1 : *a > *b;
}

char* strtok(char *s, const char *delimiters)
{
	static char *save_ptr;
	return strtok_r(s, delimiters, &save_ptr);
}

// Функция переводит курсор на строку strnum (0 – самая верхняя) в позицию pos на этой строке (0 – самое левое положение).
void cursor_moveto(unsigned int strnum, unsigned int pos)
{
	unsigned short new_pos = (strnum * VIDEO_WIDTH) + pos;
	outb(CURSOR_PORT, 0x0F);
	outb(CURSOR_PORT + 1, (unsigned char)(new_pos & 0xFF));
	outb(CURSOR_PORT, 0x0E);
	outb(CURSOR_PORT + 1, (unsigned char)( (new_pos >> 8) & 0xFF));
}

void keyb_init()
{
	// Регистрация обработчика прерывания
	intr_reg_handler(0x09, GDT_CS, 0x80 | IDT_TYPE_INTR, keyb_handler);
	// segm_sel=0x8, P=1, DPL=0, Type=Intr
	// Разрешение только прерываний клавиатуры от контроллера 8259
	outb(PIC1_PORT + 1, 0xFF ^ 0x02); // 0xFF - все прерывания, 0x02 - бит IRQ1 (клавиатура).
	// Разрешены будут только прерывания, чьи биты установлены в 0
}

void keyb_handler()
{
	asm("pusha");
	// Обработка поступивших данных
	keyb_process_keys();
	// Отправка контроллеру 8259 нотификации о том, что прерывание обработано
	outb(PIC1_PORT, 0x20);
	asm("popa; leave; iret");
}

void keyb_process_keys()
{
// Проверка что буфер PS/2 клавиатуры не пуст (младший бит присутствует)
	if (inb(0x64) & 0x01)
	{
		unsigned char scan_code;
		unsigned char state;
		scan_code = inb(0x60); // Считывание символа с PS/2 клавиатуры
		if (scan_code < 128) // Скан-коды выше 128 - это отпускание клавиши
		{
			switch (scan_code){
			case 0x0e: //backspace
				backspace_pressed();
				return;
			case 0x1c: //enter
				read_and_exec_comand();
				return;
			case 0x2a: //lshift
				shift_pressed = 1;
			case 0x36: //rshift
				shift_pressed = 1;
				return;
			}
			on_key(scan_code);
		}
		else
		{
			if (scan_code == 0xAA || scan_code == 0xb6)
				shift_pressed = 0;
		}	
	}
}

struct idt_entry
{
	unsigned short base_lo; // Младшие биты адреса обработчика
	unsigned short segm_sel; // Селектор сегмента кода
	unsigned char always0; // Этот байт всегда 0
	unsigned char flags; // Флаги тип. Флаги: P, DPL, Типы - это константы - IDT_TYPE...
	unsigned short base_hi; // Старшие биты адреса обработчика
} __attribute__((packed)); // Выравнивание запрещено

// Структура, адрес которой передается как аргумент команды lidt
struct idt_ptr
{
	unsigned short limit;
	unsigned int base;
} __attribute__((packed)); // Выравнивание запрещено

struct idt_entry g_idt[256]; // Реальная таблица IDT
struct idt_ptr g_idtp; // Описатель таблицы для команды lidt

// Пустой обработчик прерываний. Другие обработчики могут быть реализованы по этому шаблону
void default_intr_handler()
{
	asm("pusha");
	// ... (реализация обработки)
	asm("popa; leave; iret");
}


void intr_reg_handler(int num, unsigned short segm_sel, unsigned short
flags, intr_handler hndlr)
{
	unsigned int hndlr_addr = (unsigned int) hndlr;
	g_idt[num].base_lo = (unsigned short) (hndlr_addr & 0xFFFF);
	g_idt[num].segm_sel = segm_sel;
	g_idt[num].always0 = 0;
	g_idt[num].flags = flags;
	g_idt[num].base_hi = (unsigned short) (hndlr_addr >> 16);
}

// Функция инициализации системы прерываний: заполнение массива с адресами обработчиков
void intr_init()
{
	int i;
	int idt_count = sizeof(g_idt) / sizeof(g_idt[0]);
	for(i = 0; i < idt_count; i++)
		intr_reg_handler(i, GDT_CS, 0x80 | IDT_TYPE_INTR, default_intr_handler); 
		//segm_sel=0x8, P=1, DPL=0, Type=Intr
}

void intr_start()
{
	int idt_count = sizeof(g_idt) / sizeof(g_idt[0]);
	g_idtp.base = (unsigned int) (&g_idt[0]);
	g_idtp.limit = (sizeof (struct idt_entry) * idt_count) - 1;
	asm("lidt %0" : : "m" (g_idtp) );
}

void intr_enable()
{
	asm("sti");
}

void intr_disable()
{
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

void out_str(int color, const char* ptr, unsigned int strnum)
{

	if (cursor_line >= 25)
		clean();
	unsigned char* video_buf = (unsigned char*) VIDEO_BUF_PTR;
	video_buf += 80*2 * strnum;
	while (*ptr)
	{
		video_buf[0] = (unsigned char) *ptr; // Символ (код)
		video_buf[1] = color; // Цвет символа и фона
		video_buf += 2;
		ptr++;
	}
}


void out_chr(unsigned char c)
{
	unsigned char* video_buf = (unsigned char*) VIDEO_BUF_PTR;
	video_buf += 80 * 2 * cursor_line + cursor_position * 2;
	video_buf[0] = c;
	video_buf[1] = color;
	cursor_moveto(cursor_line, ++cursor_position);
}

void on_key(unsigned char scan_code)
{
	if (scan_code > 58 || cursor_position > 75)
		return;

	if(!shift_pressed && lower_case_codes[scan_code])
		out_chr(lower_case_codes[scan_code]);
	else if (shift_pressed && upper_case_codes[scan_code])
		out_chr(upper_case_codes[scan_code]);
	input_len++;
}

void shutdown(void){
  	const char *p_off = "Powering off...";
	out_str (color, p_off, cursor_line++);
    asm volatile("outw %0, %1": : "a"((unsigned short) 0x2000), "d"((unsigned short)0x604)); // отправляем команду ACPI на порт
}


void info(void)
{
	out_str(color, "Calc OS: v.01. Developer: Shabanov Ilya, 5131001/20003, SpbPU, 2024", cursor_line++);
	out_str(color, "Compilers: bootloader: yasm (at&t), kernel: gcc", cursor_line++);
	switch (*(unsigned int *) COLOR_ADDRESS){
	case green:
		out_str(color, "Bootloader parameters: green color.", cursor_line++);
		break;
	case blue:
		out_str(color, "Bootloader parameters: blue color.", cursor_line++);
		break;
	case red:
		out_str(color, "Bootloader parameters: red color.", cursor_line++);
		break;
	case yellow:
		out_str(color, "Bootloader parameters: yellow color.", cursor_line++);
		break;
	case gray:
		out_str(color, "Bootloader parameters: gray color.", cursor_line++);
		break;	
	case white:
		out_str(color, "Bootloader parameters: white color.", cursor_line++);
	}
}

void help(void){
	out_str(color, "help     - see command list ", cursor_line++);
    out_str(color, "info     - get info about developer and OS", cursor_line++);
    out_str(color, "shutdown - turn off", cursor_line++);
    out_str(color, "cls      - clean all strings", cursor_line++);
    out_str(color, "expr ___ - expresion to calculate ", cursor_line++);
}

void clean(void){
	// asm volatile("mov $3, %ah\n" "int $0x10\n");
    unsigned char *video_buf = (unsigned char *)VIDEO_BUF_PTR;
    for (int i = 0; i <= VIDEO_WIDTH * VIDEO_HEIGHT; i++)
        *(video_buf + i * 2) = '\0';
    cursor_line = 0;
    cursor_position = 0;
    input_len = 0;
}

void read_and_exec_comand(void)
{
	intr_disable();

	//read
	unsigned char *video_buf = (unsigned char *)VIDEO_BUF_PTR;
    video_buf += 80 * 2 * cursor_line + 2;
    int i;
    for(int i = 0; i < input_len; i++)
    {
        input_string[i] = video_buf[0];
        video_buf += 2;
    }
    input_string[i] = '\0';

	cursor_line++;
	cursor_position = 0;

    //choose command 
    char *token = strtok(input_string, " ");
    if(strcmp(token, "help") == 0)
    	help();
    else if(strcmp(token, "info") == 0)
    	info();
    else if(strcmp(token, "cls") == 0)
    	clean();
    else if(strcmp(token, "shutdown") == 0)
    	shutdown();
    else if(strcmp(token, "expr") == 0)
    {
    	token = strtok(NULL, " ");
	  	int result = 0;
	    int current_number = 0;
	    int prev_number = 0;
	    char operation = '+';
	    char prev_operation = '+';
	    int i = 0;

	    while (token[i] != '\0') {
	        if (token[i] == '+' || token[i] == '-' || token[i] == '*' || token[i] == '/') {
	        	if(token[i+1] && 
	        		((token[i] == '*' && token[i + 1] == '*') || 
	        		(token[i] == '/' && token[i + 1] == '/'))){
	        		out_str(color, "Error: incorrect expression", cursor_line++);
	        		goto end;
	        	}
	            operation = token[i];
	            i++;
	        } else if (token[i] == '-' || token[i] == '+') {
	            int sign_count = 0;
	            while (token[i] == '-' || token[i] == '+') {
	                if (token[i] == '-') {
	                    sign_count++;
	                }
	                i++;
	            }
	            if (sign_count % 2 == 1) {
	                prev_number = -prev_number;
	            }
	        } else if (token[i] >= '0' && token[i] <= '9') {
	            current_number = 0;
	            while (token[i] >= '0' && token[i] <= '9') {
	                current_number = current_number * 10 + (token[i] - '0');
	                if (current_number < 0) {
	                    out_str(color, "Error: integer overflow", cursor_line++);
        	            goto end;		
	                }
	                i++;
	            }
	            switch (operation) {
	                case '+':
	                    result += prev_number;
	                    prev_number = current_number;
	                    break;
	                case '-':
	                    result += prev_number;
	                    prev_number = -current_number;
	                    break;
	                case '*':
	                    prev_number *= current_number;
	                    break;
	                case '/':
	                    if (current_number == 0) {
	                        out_str(color, "Error: division by 0", cursor_line++);
            	            goto end;	
	                    }
	                    prev_number /= current_number;
	                    break;
	                default:
	                    break;
	            }
	        } else {
	            out_str(color, "Error: expression consists letters", cursor_line++);
	            goto end;
	        }
	    }

	    switch (operation) {
	        case '+':
	            result += prev_number;
	            break;
	        case '-':
	            result += prev_number;
	            break;
	        case '*':
	            result += prev_number;
	            break;
	        case '/':
	            if (prev_number == 0) {
	                out_str(color, "Error: division by 0", cursor_line++);
    	            goto end;
	            }
	            result += prev_number;
	            break;
	        default:
	            break;
	    }
	    char out[10];
	    out_str(color, citoa(result, out, 10), cursor_line++);
	}
    else{
        out_str(color, "Error: Unknown command", cursor_line++);
    }
    end:
    out_chr('#');
	cursor_moveto(cursor_line, cursor_position);

	intr_enable();
}

void backspace_pressed(void)
{
    if (cursor_position == 1)
        return;

    unsigned char *video_buf = (unsigned char *)VIDEO_BUF_PTR;
    video_buf += 80 * 2 * cursor_line + 2 * (--cursor_position);
    video_buf[0] = 0;
    cursor_moveto(cursor_line, cursor_position);
    input_len--;
}

extern "C" int kmain()
{
	//set up color from boot
	unsigned int* color_num = (unsigned int *) COLOR_ADDRESS;

	switch (*color_num){
	case '1':
		color = 2;
		break;
	case '2':
		color = 1;
		break;
	case '3':
		color = 4;
		break;
	case '4':
		color = 6;
		break;
	case '5':
		color = 8;
		break;
	case '6':
		color = 7;
		break;
	default:
		out_str(4, inp_err, cursor_line++);
		shutdown();
	}

	intr_init();
    keyb_init();
    intr_start();
    intr_enable();

	const char* hello = "Welcome to CalcOS!";
	out_str(color, hello, cursor_line++);

	help();
	out_chr('#');
	cursor_moveto(cursor_line, cursor_position);
	// Бесконечный цикл
	while(1)
	{
		asm("hlt");
	}
	return 0;
}