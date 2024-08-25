.code16
.org 0x7c00

go_to_start:
  jmp start

puts:
  movb 0(%bx), %al
  test %al, %al
  jz end_puts

  movb $0x0e, %ah
  int $0x10

  addw $1, %bx
  jmp puts

  end_puts:
  ret

clear:
  mov $3, %ax
  int $0x10
  ret

start:
  mov %cs, %ax # Сохранение адреса сегмента кода в ax
  mov %ax, %ds # Сохранение этого адреса как начало сегмента данных
  mov %ax, %ss # И сегмента стека
  mov start, %sp
  movw $loading_str, %bx
  call puts


load_kernel:
  mov $0x1, %dl
  xor %dh, %dh
  mov $0x1, %cl
  xor %ch, %ch
  mov $24, %al     # size of kernel comand 'du' in term
  mov $0x1000, %bx
  mov %bx, %es
  xor %bx, %bx
  mov $0x02, %ah
  int $0x13

read_color:
  call clear

  movw $color_choose, %bx
  call puts

  mov $0x00, %ah
  int $0x16          # => %al

  mov $0x0e, %ah
  int $0x10

  #put arg to memory
  mov $0x9000, %edi
  movb %al, 0(%edi) 

  call clear

  #mov $0x00, %ah
  #int $0x16


turn_on_protected_mode:
  # Отключение прерываний
  cli

  # Загрузка размера и адреса таблицы дескрипторов
  lgdt gdt_info

  # Включение адресной линии А20
  inb $0x92, %al
  orb $2, %al
  outb %al, $0x92

  # Установка бита PE регистра CR0 - процессор перейдет в защищенный режим
  movl %cr0, %eax
  orb $1, %al
  movl %eax, %cr0
  ljmp $0x8, $protected_mode # "Дальний" переход для загрузки 

.code32
protected_mode:
  movw $0x10, %ax
  movw %ax, %es
  movw %ax, %ds
  movw %ax, %ss
  call 0x10000
  

loading_str:
  .asciz "Loading...\n\r"

color_choose:
  .asciz "To choosse color press:\n\r1-green\n\r2-blue\n\r3-red\n\r4-yellow\n\r5-gray\n\r6-white\n\r"


gdt:
  .byte 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
  .byte 0xff, 0xff, 0x00, 0x00, 0x00, 0x9A, 0xCF, 0x00
  .byte 0xff, 0xff, 0x00, 0x00, 0x00, 0x92, 0xCF, 0x00

gdt_info:
  .word gdt_info - gdt
  .word gdt, 0

.zero (512 - (. - go_to_start) - 2)
.byte 0x55, 0xAA
