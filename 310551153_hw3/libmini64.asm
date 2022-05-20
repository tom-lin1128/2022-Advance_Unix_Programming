
section .data

section .text

global sys_write:function
sys_write:
    mov	rax, 1
	syscall
    ret

global sys_exit:function
sys_exit:
    mov rax, 60
    syscall
    ret

global sys_rt_sigpending:function
sys_rt_sigpending:
    mov rax, 127
    syscall
    ret

global sys_rt_sigprocmask:function
sys_rt_sigprocmask:
    push r10
	mov	r10, rcx
    mov rax, 14
    syscall
    pop r10
    ret

global  sys_rt_sigaction:function
sys_rt_sigaction:
    push r10
	mov	r10, rcx
    mov rax, 13
    syscall
    pop r10
    ret

global __myrt:function
__myrt:
    mov rax, 15
    syscall
    ret

global sys_alarm:function
sys_alarm:
    mov rax, 37
    syscall
    ret

global sys_pause:function
sys_pause:
    mov rax, 34
    syscall
    ret

global sys_nanosleep:function
sys_nanosleep:
    mov rax, 35
    syscall
    ret

global setjmp:function
setjmp:
    mov qword [rdi + 0], rbx
    mov qword [rdi + 8], rsp
    mov qword [rdi + 16], rbp
    mov qword [rdi + 24], r12
    mov qword [rdi + 32], r13
    mov qword [rdi + 40], r14
    mov qword [rdi + 48], r15
    mov rax, qword [rsp] 
    mov qword [rdi + 56], rax

    push rdi
    push rsi
    push rdx
    push rcx

    mov rdi, 0
    mov rsi, 0
    lea rdx, [rdi + 64] 
    mov rcx, 8
    call sys_rt_sigprocmask
    
    pop rcx     
    pop rdx
    pop rsi
    pop rdi
    mov rax, 0
    ret

global longjmp:function
longjmp:
    mov rbx, qword [rdi + 0]
    mov rsp, qword [rdi + 8]
    mov rbp, qword [rdi + 16]
    mov r12, qword [rdi + 24]
    mov r13, qword [rdi + 32]
    mov r14, qword [rdi + 40]
    mov r15, qword [rdi + 48]
    mov rax, qword [rdi + 56]
    mov qword [rsp], rax

    push rdi
    push rsi
    push rdx
    push rcx

    lea rsi, [rdi + 64] 
    mov rdi, 2
    mov rdx, 0
    mov rcx, 8
    call sys_rt_sigprocmask

    pop rcx     
    pop rdx
    pop rsi
    pop rdi
    mov rax, rsi
    ret