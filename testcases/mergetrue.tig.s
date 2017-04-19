.section .rodata
.L43:
.string "\00\00\00 "
.L42:
.string "\00\00\00\n"
.L35:
.string "\00\00\000"
.L34:
.string "\00\00\00-"
.L31:
.string "\00\00\000"
.L16:
.string "\00\00\000"
.L9:
.string "\00\00\00\n"
.L8:
.string "\00\00\00 "
.L1:
.string "\00\00\009"
.L0:
.string "\00\00\000"
.text
.globl tigermain
.type tigermain, @function
tigermain:
pushl %ebp
movl %esp,%ebp
subl $12,%esp
L48:
movl %ebp,%eax
addl $-4,%eax
movl %eax,%ebx
call getchar
movl %eax,%eax
movl %eax,(%ebx)
movl %ebp,%eax
addl $-8,%eax
movl %eax,%ebx
pushl %ebp
call FUN3
movl %eax,%eax
movl %eax,(%ebx)
movl %ebp,%eax
addl $-12,%eax
movl %eax,%ebx
movl %ebp,%eax
addl $-4,%eax
movl %eax,%esi
call getchar
movl %eax,%eax
movl %eax,(%esi)
pushl %ebp
call FUN3
movl %eax,(%ebx)
movl %ebp,%ebx
movl -8(%ebp),%eax
movl -12(%ebp),%ecx
pushl %ecx
pushl %eax
pushl %ebp
call FUN4
movl %eax,%eax
pushl %eax
pushl %ebx
call FUN6
jmp L47
L47:

leave
ret
.text
.globl FUN6
.type FUN6, @function
FUN6:
pushl %ebp
movl %esp,%ebp
subl $12,%esp
movl %edi, -4(%ebp)
movl %esi, -8(%ebp)
movl %ebx, -12(%ebp)
L50:
movl 12(%ebp),%eax
movl $0,%ebx
cmp %ebx,%eax
je L44
L45:
movl 8(%ebp),%eax
movl 12(%ebp),%ebx
movl 0(%ebx),%ebx
pushl %ebx
pushl %eax
call FUN5
movl $.L43,%eax
pushl %eax
call print
movl 8(%ebp),%eax
movl 12(%ebp),%ebx
movl 4(%ebx),%ebx
pushl %ebx
pushl %eax
call FUN6
movl %eax,%eax
L46:
jmp L49
L44:
movl $.L42,%eax
pushl %eax
call print
movl %eax,%eax
jmp L46
L49:
movl  -4(%ebp),%edi
movl  -8(%ebp),%esi
movl  -12(%ebp),%ebx

leave
ret
.text
.globl FUN5
.type FUN5, @function
FUN5:
pushl %ebp
movl %esp,%ebp
subl $12,%esp
movl %edi, -4(%ebp)
movl %esi, -8(%ebp)
movl %ebx, -12(%ebp)
L52:
movl 12(%ebp),%eax
movl $0,%ebx
cmp %ebx,%eax
jl L39
L40:
movl 12(%ebp),%eax
movl $0,%ebx
cmp %ebx,%eax
jg L36
L37:
movl $.L35,%eax
pushl %eax
call print
movl %eax,%eax
L38:
movl %eax,%eax
L41:
jmp L51
L39:
movl $.L34,%eax
pushl %eax
call print
movl $0,%eax
movl 12(%ebp),%ebx
subl %ebx,%eax
pushl %eax
pushl %ebp
call FUN7
movl %eax,%eax
jmp L41
L36:
movl 12(%ebp),%eax
pushl %eax
pushl %ebp
call FUN7
movl %eax,%eax
jmp L38
L51:
movl  -4(%ebp),%edi
movl  -8(%ebp),%esi
movl  -12(%ebp),%ebx

leave
ret
.text
.globl FUN7
.type FUN7, @function
FUN7:
pushl %ebp
movl %esp,%ebp
subl $12,%esp
movl %edi, -4(%ebp)
movl %esi, -8(%ebp)
movl %ebx, -12(%ebp)
L54:
movl 12(%ebp),%eax
movl $0,%ebx
cmp %ebx,%eax
jg L32
L33:
jmp L53
L32:
movl 8(%ebp),%ebx
movl 12(%ebp),%eax
mov $10, %ecx
mov $0, %edx
mov %eax, %eax
idiv %ecx
mov %eax, %eax
pushl %eax
pushl %ebx
call FUN7
movl 8(%ebp),%eax
movl 8(%eax),%eax
movl 8(%eax),%eax
movl %eax,%eax
movl 8(%ebp),%eax
movl 8(%eax),%eax
movl 8(%eax),%eax
movl %eax,%eax
movl 12(%ebp),%eax
movl %eax,%ebx
movl 12(%ebp),%eax
mov $10, %ecx
mov $0, %edx
mov %eax, %eax
idiv %ecx
mov %eax, %eax
movl %eax,%eax
imull $10,%eax
subl %eax,%ebx
movl %ebx,%esi
movl $.L31,%eax
pushl %eax
call ord
movl %eax,%ebx
movl %esi,%eax
addl %ebx,%eax
pushl %eax
call chr
movl %eax,%eax
pushl %eax
call print
jmp L33
L53:
movl  -4(%ebp),%edi
movl  -8(%ebp),%esi
movl  -12(%ebp),%ebx

leave
ret
.text
.globl FUN4
.type FUN4, @function
FUN4:
pushl %ebp
movl %esp,%ebp
subl $12,%esp
movl %edi, -4(%ebp)
movl %esi, -8(%ebp)
movl %ebx, -12(%ebp)
L56:
movl 12(%ebp),%eax
movl $0,%ebx
cmp %ebx,%eax
je L28
L29:
movl 16(%ebp),%eax
movl $0,%ebx
cmp %ebx,%eax
je L25
L26:
movl 12(%ebp),%eax
movl 0(%eax),%eax
movl 16(%ebp),%ebx
movl 0(%ebx),%ebx
cmp %ebx,%eax
jl L22
L23:
movl $8,%eax
pushl %eax
call malloc
movl %eax,%ebx
movl 16(%ebp),%eax
movl 0(%eax),%eax
movl %eax,0(%ebx)
movl %ebx,%eax
addl $4,%eax
movl %eax,%esi
movl 8(%ebp),%eax
movl 12(%ebp),%ecx
movl 16(%ebp),%edx
movl 4(%edx),%edx
pushl %edx
pushl %ecx
pushl %eax
call FUN4
movl %eax,%eax
movl %eax,(%esi)
movl %ebx,%eax
L24:
movl %eax,%eax
L27:
movl %eax,%eax
L30:
movl %eax,%eax
jmp L55
L28:
movl 16(%ebp),%eax
movl %eax,%eax
jmp L30
L25:
movl 12(%ebp),%eax
movl %eax,%eax
jmp L27
L22:
movl $8,%eax
pushl %eax
call malloc
movl %eax,%ebx
movl 12(%ebp),%eax
movl 0(%eax),%eax
movl %eax,0(%ebx)
movl %ebx,%eax
addl $4,%eax
movl %eax,%esi
movl 8(%ebp),%eax
movl 12(%ebp),%ecx
movl 4(%ecx),%ecx
movl 16(%ebp),%edx
pushl %edx
pushl %ecx
pushl %eax
call FUN4
movl %eax,%eax
movl %eax,(%esi)
movl %ebx,%eax
jmp L24
L55:
movl  -4(%ebp),%edi
movl  -8(%ebp),%esi
movl  -12(%ebp),%ebx

leave
ret
.text
.globl FUN3
.type FUN3, @function
FUN3:
pushl %ebp
movl %esp,%ebp
subl $20,%esp
movl %edi, -12(%ebp)
movl %esi, -16(%ebp)
movl %ebx, -20(%ebp)
L58:
movl %ebp,%eax
addl $-4,%eax
movl %eax,%ebx
movl $4,%eax
pushl %eax
call malloc
movl %eax,%eax
movl $0,%ecx
movl %ecx,0(%eax)
movl %eax,(%ebx)
movl %ebp,%eax
addl $-8,%eax
movl %eax,%ebx
movl 8(%ebp),%eax
movl -4(%ebp),%ecx
pushl %ecx
pushl %eax
call FUN0
movl %eax,%eax
movl %eax,(%ebx)
movl -4(%ebp),%eax
movl 0(%eax),%eax
movl $0,%ebx
cmp %ebx,%eax
je L20
L19:
movl $8,%eax
pushl %eax
call malloc
movl %eax,%esi
movl -8(%ebp),%eax
movl %eax,0(%esi)
movl %esi,%eax
addl $4,%eax
movl %eax,%ebx
movl 8(%ebp),%eax
pushl %eax
call FUN3
movl %eax,%eax
movl %eax,(%ebx)
movl %esi,%eax
L21:
movl %eax,%eax
jmp L57
L20:
movl $0,%eax
movl %eax,%eax
jmp L21
L57:
movl  -12(%ebp),%edi
movl  -16(%ebp),%esi
movl  -20(%ebp),%ebx

leave
ret
.text
.globl FUN0
.type FUN0, @function
FUN0:
pushl %ebp
movl %esp,%ebp
subl $16,%esp
movl %edi, -8(%ebp)
movl %esi, -12(%ebp)
movl %ebx, -16(%ebp)
L60:
movl $0,%eax
movl %eax,-4(%ebp)
pushl %ebp
call FUN2
movl 12(%ebp),%eax
movl %eax,%eax
addl $0,%eax
movl %eax,%ebx
movl 8(%ebp),%eax
movl -4(%eax),%eax
pushl %eax
pushl %ebp
call FUN1
movl %eax,%eax
movl %eax,(%ebx)
L17:
movl 8(%ebp),%eax
movl -4(%eax),%eax
pushl %eax
pushl %ebp
call FUN1
movl %eax,%eax
movl $0,%ebx
cmp %ebx,%eax
je L15
L18:
movl %ebp,%eax
addl $-4,%eax
movl %eax,%edi
movl -4(%ebp),%eax
movl %eax,%eax
imull $10,%eax
movl %eax,%esi
movl 8(%ebp),%eax
movl -4(%eax),%eax
pushl %eax
call ord
movl %eax,%ebx
movl %esi,%eax
addl %ebx,%eax
movl %eax,%esi
movl $.L16,%eax
pushl %eax
call ord
movl %eax,%ebx
movl %esi,%eax
subl %ebx,%eax
movl %eax,(%edi)
movl 8(%ebp),%eax
movl %eax,%eax
addl $-4,%eax
movl %eax,%ebx
call getchar
movl %eax,%eax
movl %eax,(%ebx)
jmp L17
L15:
movl -4(%ebp),%eax
movl %eax,%eax
jmp L59
L59:
movl  -8(%ebp),%edi
movl  -12(%ebp),%esi
movl  -16(%ebp),%ebx

leave
ret
.text
.globl FUN2
.type FUN2, @function
FUN2:
pushl %ebp
movl %esp,%ebp
subl $12,%esp
movl %edi, -4(%ebp)
movl %esi, -8(%ebp)
movl %ebx, -12(%ebp)
L62:
L13:
movl 8(%ebp),%eax
movl 8(%eax),%eax
movl -4(%eax),%eax
movl $.L8,%ebx
pushl %ebx
pushl %eax
call stringEqual
movl %eax,%eax
movl $0,%ebx
cmp %ebx,%eax
je L11
L10:
movl $1,%eax
movl %eax,%eax
L12:
movl $0,%ebx
cmp %ebx,%eax
je L7
L14:
movl 8(%ebp),%eax
movl 8(%eax),%eax
movl %eax,%eax
addl $-4,%eax
movl %eax,%ebx
call getchar
movl %eax,%eax
movl %eax,(%ebx)
jmp L13
L11:
movl 8(%ebp),%eax
movl 8(%eax),%eax
movl -4(%eax),%eax
movl $.L9,%ebx
pushl %ebx
pushl %eax
call stringEqual
movl %eax,%eax
jmp L12
L7:
jmp L61
L61:
movl  -4(%ebp),%edi
movl  -8(%ebp),%esi
movl  -12(%ebp),%ebx

leave
ret
.text
.globl FUN1
.type FUN1, @function
FUN1:
pushl %ebp
movl %esp,%ebp
subl $12,%esp
movl %edi, -4(%ebp)
movl %esi, -8(%ebp)
movl %ebx, -12(%ebp)
L64:
movl 8(%ebp),%eax
movl 8(%eax),%eax
movl -4(%eax),%eax
pushl %eax
call ord
movl %eax,%eax
movl %eax,%ebx
movl $.L0,%eax
pushl %eax
call ord
movl %eax,%eax
cmp %eax,%ebx
jge L2
L3:
movl $0,%eax
movl %eax,%eax
L4:
movl %eax,%eax
jmp L63
L2:
movl $1,%eax
movl %eax,%esi
movl 8(%ebp),%eax
movl 8(%eax),%eax
movl -4(%eax),%eax
pushl %eax
call ord
movl %eax,%eax
movl %eax,%ebx
movl $.L1,%eax
pushl %eax
call ord
movl %eax,%eax
cmp %eax,%ebx
jle L5
L6:
movl $0,%eax
movl %eax,%esi
L5:
movl %esi,%eax
jmp L4
L63:
movl  -4(%ebp),%edi
movl  -8(%ebp),%esi
movl  -12(%ebp),%ebx

leave
ret
