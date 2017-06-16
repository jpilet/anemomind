	.section	__TEXT,__text,regular,pure_instructions
	.macosx_version_min 10, 11
	.section	__TEXT,__literal16,16byte_literals
	.align	4
LCPI0_0:
	.long	0                       ## 0x0
	.long	1                       ## 0x1
	.long	2                       ## 0x2
	.long	3                       ## 0x3
LCPI0_1:
	.long	4                       ## 0x4
	.long	5                       ## 0x5
	.long	6                       ## 0x6
	.long	7                       ## 0x7
	.section	__TEXT,__literal8,8byte_literals
	.align	3
LCPI0_2:
	.quad	4696837146684686336     ## double 1.0E+6
	.section	__TEXT,__text,regular,pure_instructions
	.globl	__Z10sumOddDiv3v
	.align	4, 0x90
__Z10sumOddDiv3v:                       ## @_Z10sumOddDiv3v
Lfunc_begin0:
	.cfi_startproc
	.cfi_personality 155, ___gxx_personality_v0
	.cfi_lsda 16, Lexception0
## BB#0:                                ## %_ZNSt3__16vectorIiNS_9allocatorIiEEEC1Em.exit
	pushq	%rbp
Ltmp16:
	.cfi_def_cfa_offset 16
Ltmp17:
	.cfi_offset %rbp, -16
	movq	%rsp, %rbp
Ltmp18:
	.cfi_def_cfa_register %rbp
	pushq	%r15
	pushq	%r14
	pushq	%r13
	pushq	%r12
	pushq	%rbx
	pushq	%rax
Ltmp19:
	.cfi_offset %rbx, -56
Ltmp20:
	.cfi_offset %r12, -48
Ltmp21:
	.cfi_offset %r13, -40
Ltmp22:
	.cfi_offset %r14, -32
Ltmp23:
	.cfi_offset %r15, -24
	movl	$120000, %edi           ## imm = 0x1D4C0
	callq	__Znwm
	movq	%rax, %r14
	leaq	120000(%r14), %rbx
	movl	$120000, %esi           ## imm = 0x1D4C0
	movq	%r14, %rdi
	callq	___bzero
	xorl	%eax, %eax
	movdqa	LCPI0_0(%rip), %xmm0    ## xmm0 = [0,1,2,3]
	movdqa	LCPI0_1(%rip), %xmm1    ## xmm1 = [4,5,6,7]
	.align	4, 0x90
LBB0_1:                                 ## %vector.body
                                        ## =>This Inner Loop Header: Depth=1
	movd	%eax, %xmm2
	pshufd	$0, %xmm2, %xmm2        ## xmm2 = xmm2[0,0,0,0]
	movdqa	%xmm2, %xmm3
	paddd	%xmm0, %xmm3
	paddd	%xmm1, %xmm2
	movdqu	%xmm3, (%r14,%rax,4)
	movdqu	%xmm2, 16(%r14,%rax,4)
	leal	8(%rax), %ecx
	movd	%ecx, %xmm2
	pshufd	$0, %xmm2, %xmm2        ## xmm2 = xmm2[0,0,0,0]
	movdqa	%xmm2, %xmm3
	paddd	%xmm0, %xmm3
	paddd	%xmm1, %xmm2
	movdqu	%xmm3, 32(%r14,%rax,4)
	movdqu	%xmm2, 48(%r14,%rax,4)
	leal	16(%rax), %ecx
	movd	%ecx, %xmm2
	pshufd	$0, %xmm2, %xmm2        ## xmm2 = xmm2[0,0,0,0]
	movdqa	%xmm2, %xmm3
	paddd	%xmm0, %xmm3
	paddd	%xmm1, %xmm2
	movdqu	%xmm3, 64(%r14,%rax,4)
	movdqu	%xmm2, 80(%r14,%rax,4)
	addq	$24, %rax
	cmpq	$30000, %rax            ## imm = 0x7530
	jne	LBB0_1
## BB#2:                                ## %middle.block
	xorl	%r12d, %r12d
	callq	__ZNSt3__16chrono12system_clock3nowEv
	movq	%rax, %r15
	xorl	%r13d, %r13d
	.align	4, 0x90
LBB0_3:                                 ## %.lr.ph.i.i.preheader
                                        ## =>This Loop Header: Depth=1
                                        ##     Child Loop BB0_4 Depth 2
	xorl	%eax, %eax
	movq	%r14, %rcx
	.align	4, 0x90
LBB0_4:                                 ## %.lr.ph.i.i
                                        ##   Parent Loop BB0_3 Depth=1
                                        ## =>  This Inner Loop Header: Depth=2
	movl	(%rcx), %edx
	movl	%edx, %esi
	shrl	$31, %esi
	addl	%edx, %esi
	andl	$-2, %esi
	movl	%edx, %edi
	subl	%esi, %edi
	cmpl	$1, %edi
	jne	LBB0_6
## BB#5:                                ##   in Loop: Header=BB0_4 Depth=2
	movslq	%edx, %rdx
	imulq	$1431655766, %rdx, %rdx ## imm = 0x55555556
	movq	%rdx, %rsi
	shrq	$63, %rsi
	shrq	$32, %rdx
	addl	%esi, %edx
	addl	%eax, %edx
	movl	%edx, %eax
LBB0_6:                                 ## %_ZN4sail6FilterIZN10SumOddDiv3ILb1EE5applyERKNSt3__16vectorIiNS3_9allocatorIiEEEEEUliE_NS_3MapIZNS2_5applyES9_EUliE0_NS_7AddStepIiEEEEE4stepEii.exit.i.i
                                        ##   in Loop: Header=BB0_4 Depth=2
	movl	4(%rcx), %edx
	movl	%edx, %esi
	shrl	$31, %esi
	addl	%edx, %esi
	andl	$-2, %esi
	movl	%edx, %edi
	subl	%esi, %edi
	cmpl	$1, %edi
	jne	LBB0_8
## BB#7:                                ##   in Loop: Header=BB0_4 Depth=2
	movslq	%edx, %rdx
	imulq	$1431655766, %rdx, %rdx ## imm = 0x55555556
	movq	%rdx, %rsi
	shrq	$63, %rsi
	shrq	$32, %rdx
	addl	%esi, %edx
	addl	%eax, %edx
	movl	%edx, %eax
LBB0_8:                                 ## %_ZN4sail6FilterIZN10SumOddDiv3ILb1EE5applyERKNSt3__16vectorIiNS3_9allocatorIiEEEEEUliE_NS_3MapIZNS2_5applyES9_EUliE0_NS_7AddStepIiEEEEE4stepEii.exit.i.i.1
                                        ##   in Loop: Header=BB0_4 Depth=2
	addq	$8, %rcx
	cmpq	%rbx, %rcx
	jne	LBB0_4
## BB#9:                                ## %_ZN10SumOddDiv3ILb1EE5applyERKNSt3__16vectorIiNS1_9allocatorIiEEEE.exit
                                        ##   in Loop: Header=BB0_3 Depth=1
	cltq
	imulq	$1431655766, %rax, %rcx ## imm = 0x55555556
	movq	%rcx, %rdx
	shrq	$63, %rdx
	shrq	$32, %rcx
	addl	%edx, %ecx
	leal	(%rcx,%rcx,2), %ecx
	subl	%ecx, %eax
	addl	%eax, %r13d
	incl	%r12d
	cmpl	$30000, %r12d           ## imm = 0x7530
	jne	LBB0_3
## BB#10:
	callq	__ZNSt3__16chrono12system_clock3nowEv
	movq	%rax, %r12
Ltmp0:
	movq	__ZNSt3__14coutE@GOTPCREL(%rip), %rdi
	leaq	L_.str(%rip), %rsi
	movl	$17, %edx
	callq	__ZNSt3__124__put_character_sequenceIcNS_11char_traitsIcEEEERNS_13basic_ostreamIT_T0_EES7_PKS4_m
Ltmp1:
## BB#11:                               ## %_ZNSt3__1lsINS_11char_traitsIcEEEERNS_13basic_ostreamIcT_EES6_PKc.exit
	subq	%r15, %r12
	cvtsi2sdq	%r12, %xmm0
	divsd	LCPI0_2(%rip), %xmm0
Ltmp2:
	movq	%rax, %rdi
	callq	__ZNSt3__113basic_ostreamIcNS_11char_traitsIcEEElsEd
	movq	%rax, %r15
Ltmp3:
## BB#12:
	movq	(%r15), %rax
	movq	-24(%rax), %rsi
	addq	%r15, %rsi
Ltmp4:
	leaq	-48(%rbp), %rdi
	callq	__ZNKSt3__18ios_base6getlocEv
Ltmp5:
## BB#13:                               ## %.noexc14
Ltmp6:
	movq	__ZNSt3__15ctypeIcE2idE@GOTPCREL(%rip), %rsi
	leaq	-48(%rbp), %rdi
	callq	__ZNKSt3__16locale9use_facetERNS0_2idE
Ltmp7:
## BB#14:
	movq	(%rax), %rcx
	movq	56(%rcx), %rcx
Ltmp8:
	movl	$10, %esi
	movq	%rax, %rdi
	callq	*%rcx
	movb	%al, %r12b
Ltmp9:
## BB#15:                               ## %.noexc
	leaq	-48(%rbp), %rdi
	callq	__ZNSt3__16localeD1Ev
Ltmp11:
	movsbl	%r12b, %esi
	movq	%r15, %rdi
	callq	__ZNSt3__113basic_ostreamIcNS_11char_traitsIcEEE3putEc
Ltmp12:
## BB#16:                               ## %.noexc11
Ltmp13:
	movq	%r15, %rdi
	callq	__ZNSt3__113basic_ostreamIcNS_11char_traitsIcEEE5flushEv
Ltmp14:
## BB#17:                               ## %_ZNSt3__16vectorIiNS_9allocatorIiEEED1Ev.exit10
	movq	%r14, %rdi
	callq	__ZdlPv
	movl	%r13d, %eax
	addq	$8, %rsp
	popq	%rbx
	popq	%r12
	popq	%r13
	popq	%r14
	popq	%r15
	popq	%rbp
	retq
LBB0_18:
Ltmp15:
	movq	%rax, %rbx
	jmp	LBB0_19
LBB0_20:
Ltmp10:
	movq	%rax, %rbx
	leaq	-48(%rbp), %rdi
	callq	__ZNSt3__16localeD1Ev
LBB0_19:                                ## %_ZNSt3__16vectorIiNS_9allocatorIiEEED1Ev.exit
	movq	%r14, %rdi
	callq	__ZdlPv
	movq	%rbx, %rdi
	callq	__Unwind_Resume
Lfunc_end0:
	.cfi_endproc
	.section	__TEXT,__gcc_except_tab
	.align	2
GCC_except_table0:
Lexception0:
	.byte	255                     ## @LPStart Encoding = omit
	.byte	155                     ## @TType Encoding = indirect pcrel sdata4
	.asciz	"\303\200"              ## @TType base offset
	.byte	3                       ## Call site Encoding = udata4
	.byte	65                      ## Call site table length
Lset0 = Lfunc_begin0-Lfunc_begin0       ## >> Call Site 1 <<
	.long	Lset0
Lset1 = Ltmp0-Lfunc_begin0              ##   Call between Lfunc_begin0 and Ltmp0
	.long	Lset1
	.long	0                       ##     has no landing pad
	.byte	0                       ##   On action: cleanup
Lset2 = Ltmp0-Lfunc_begin0              ## >> Call Site 2 <<
	.long	Lset2
Lset3 = Ltmp5-Ltmp0                     ##   Call between Ltmp0 and Ltmp5
	.long	Lset3
Lset4 = Ltmp15-Lfunc_begin0             ##     jumps to Ltmp15
	.long	Lset4
	.byte	0                       ##   On action: cleanup
Lset5 = Ltmp6-Lfunc_begin0              ## >> Call Site 3 <<
	.long	Lset5
Lset6 = Ltmp9-Ltmp6                     ##   Call between Ltmp6 and Ltmp9
	.long	Lset6
Lset7 = Ltmp10-Lfunc_begin0             ##     jumps to Ltmp10
	.long	Lset7
	.byte	0                       ##   On action: cleanup
Lset8 = Ltmp11-Lfunc_begin0             ## >> Call Site 4 <<
	.long	Lset8
Lset9 = Ltmp14-Ltmp11                   ##   Call between Ltmp11 and Ltmp14
	.long	Lset9
Lset10 = Ltmp15-Lfunc_begin0            ##     jumps to Ltmp15
	.long	Lset10
	.byte	0                       ##   On action: cleanup
Lset11 = Ltmp14-Lfunc_begin0            ## >> Call Site 5 <<
	.long	Lset11
Lset12 = Lfunc_end0-Ltmp14              ##   Call between Ltmp14 and Lfunc_end0
	.long	Lset12
	.long	0                       ##     has no landing pad
	.byte	0                       ##   On action: cleanup
	.align	2

	.section	__TEXT,__text,regular,pure_instructions
	.globl	_main
	.align	4, 0x90
_main:                                  ## @main
Lfunc_begin1:
	.cfi_startproc
	.cfi_personality 155, ___gxx_personality_v0
	.cfi_lsda 16, Lexception1
## BB#0:
	pushq	%rbp
Ltmp29:
	.cfi_def_cfa_offset 16
Ltmp30:
	.cfi_offset %rbp, -16
	movq	%rsp, %rbp
Ltmp31:
	.cfi_def_cfa_register %rbp
	pushq	%r14
	pushq	%rbx
	subq	$16, %rsp
Ltmp32:
	.cfi_offset %rbx, -32
Ltmp33:
	.cfi_offset %r14, -24
	movq	__ZNSt3__14coutE@GOTPCREL(%rip), %rdi
	leaq	L_.str.1(%rip), %rsi
	movl	$7, %edx
	callq	__ZNSt3__124__put_character_sequenceIcNS_11char_traitsIcEEEERNS_13basic_ostreamIT_T0_EES7_PKS4_m
	movq	%rax, %rbx
	callq	__Z10sumOddDiv3v
	movq	%rbx, %rdi
	movl	%eax, %esi
	callq	__ZNSt3__113basic_ostreamIcNS_11char_traitsIcEEElsEi
	movq	%rax, %rbx
	movq	(%rbx), %rax
	movq	-24(%rax), %rsi
	addq	%rbx, %rsi
	leaq	-24(%rbp), %r14
	movq	%r14, %rdi
	callq	__ZNKSt3__18ios_base6getlocEv
Ltmp24:
	movq	__ZNSt3__15ctypeIcE2idE@GOTPCREL(%rip), %rsi
	movq	%r14, %rdi
	callq	__ZNKSt3__16locale9use_facetERNS0_2idE
Ltmp25:
## BB#1:
	movq	(%rax), %rcx
	movq	56(%rcx), %rcx
Ltmp26:
	movl	$10, %esi
	movq	%rax, %rdi
	callq	*%rcx
	movb	%al, %r14b
Ltmp27:
## BB#2:                                ## %_ZNSt3__14endlIcNS_11char_traitsIcEEEERNS_13basic_ostreamIT_T0_EES7_.exit
	leaq	-24(%rbp), %rdi
	callq	__ZNSt3__16localeD1Ev
	movsbl	%r14b, %esi
	movq	%rbx, %rdi
	callq	__ZNSt3__113basic_ostreamIcNS_11char_traitsIcEEE3putEc
	movq	%rbx, %rdi
	callq	__ZNSt3__113basic_ostreamIcNS_11char_traitsIcEEE5flushEv
	xorl	%eax, %eax
	addq	$16, %rsp
	popq	%rbx
	popq	%r14
	popq	%rbp
	retq
LBB1_3:
Ltmp28:
	movq	%rax, %rbx
	leaq	-24(%rbp), %rdi
	callq	__ZNSt3__16localeD1Ev
	movq	%rbx, %rdi
	callq	__Unwind_Resume
Lfunc_end1:
	.cfi_endproc
	.section	__TEXT,__gcc_except_tab
	.align	2
GCC_except_table1:
Lexception1:
	.byte	255                     ## @LPStart Encoding = omit
	.byte	155                     ## @TType Encoding = indirect pcrel sdata4
	.byte	41                      ## @TType base offset
	.byte	3                       ## Call site Encoding = udata4
	.byte	39                      ## Call site table length
Lset13 = Lfunc_begin1-Lfunc_begin1      ## >> Call Site 1 <<
	.long	Lset13
Lset14 = Ltmp24-Lfunc_begin1            ##   Call between Lfunc_begin1 and Ltmp24
	.long	Lset14
	.long	0                       ##     has no landing pad
	.byte	0                       ##   On action: cleanup
Lset15 = Ltmp24-Lfunc_begin1            ## >> Call Site 2 <<
	.long	Lset15
Lset16 = Ltmp27-Ltmp24                  ##   Call between Ltmp24 and Ltmp27
	.long	Lset16
Lset17 = Ltmp28-Lfunc_begin1            ##     jumps to Ltmp28
	.long	Lset17
	.byte	0                       ##   On action: cleanup
Lset18 = Ltmp27-Lfunc_begin1            ## >> Call Site 3 <<
	.long	Lset18
Lset19 = Lfunc_end1-Ltmp27              ##   Call between Ltmp27 and Lfunc_end1
	.long	Lset19
	.long	0                       ##     has no landing pad
	.byte	0                       ##   On action: cleanup
	.align	2

	.section	__TEXT,__textcoal_nt,coalesced,pure_instructions
	.private_extern	___clang_call_terminate
	.globl	___clang_call_terminate
	.weak_def_can_be_hidden	___clang_call_terminate
	.align	4, 0x90
___clang_call_terminate:                ## @__clang_call_terminate
## BB#0:
	pushq	%rax
	callq	___cxa_begin_catch
	callq	__ZSt9terminatev

	.globl	__ZNSt3__124__put_character_sequenceIcNS_11char_traitsIcEEEERNS_13basic_ostreamIT_T0_EES7_PKS4_m
	.weak_def_can_be_hidden	__ZNSt3__124__put_character_sequenceIcNS_11char_traitsIcEEEERNS_13basic_ostreamIT_T0_EES7_PKS4_m
	.align	4, 0x90
__ZNSt3__124__put_character_sequenceIcNS_11char_traitsIcEEEERNS_13basic_ostreamIT_T0_EES7_PKS4_m: ## @_ZNSt3__124__put_character_sequenceIcNS_11char_traitsIcEEEERNS_13basic_ostreamIT_T0_EES7_PKS4_m
Lfunc_begin2:
	.cfi_startproc
	.cfi_personality 155, ___gxx_personality_v0
	.cfi_lsda 16, Lexception2
## BB#0:
	pushq	%rbp
Ltmp55:
	.cfi_def_cfa_offset 16
Ltmp56:
	.cfi_offset %rbp, -16
	movq	%rsp, %rbp
Ltmp57:
	.cfi_def_cfa_register %rbp
	pushq	%r15
	pushq	%r14
	pushq	%r13
	pushq	%r12
	pushq	%rbx
	subq	$40, %rsp
Ltmp58:
	.cfi_offset %rbx, -56
Ltmp59:
	.cfi_offset %r12, -48
Ltmp60:
	.cfi_offset %r13, -40
Ltmp61:
	.cfi_offset %r14, -32
Ltmp62:
	.cfi_offset %r15, -24
	movq	%rdx, %r14
	movq	%rsi, %r15
	movq	%rdi, %rbx
Ltmp34:
	leaq	-64(%rbp), %rdi
	movq	%rbx, %rsi
	callq	__ZNSt3__113basic_ostreamIcNS_11char_traitsIcEEE6sentryC1ERS3_
Ltmp35:
## BB#1:
	cmpb	$0, -64(%rbp)
	je	LBB3_10
## BB#2:
	movq	(%rbx), %rax
	movq	-24(%rax), %rax
	leaq	(%rbx,%rax), %r12
	movq	40(%rbx,%rax), %rdi
	movl	8(%rbx,%rax), %r13d
	movl	144(%rbx,%rax), %eax
	cmpl	$-1, %eax
	jne	LBB3_7
## BB#3:
Ltmp37:
	movq	%rdi, -72(%rbp)         ## 8-byte Spill
	leaq	-48(%rbp), %rdi
	movq	%r12, %rsi
	callq	__ZNKSt3__18ios_base6getlocEv
Ltmp38:
## BB#4:                                ## %.noexc
Ltmp39:
	movq	__ZNSt3__15ctypeIcE2idE@GOTPCREL(%rip), %rsi
	leaq	-48(%rbp), %rdi
	callq	__ZNKSt3__16locale9use_facetERNS0_2idE
Ltmp40:
## BB#5:
	movq	(%rax), %rcx
	movq	56(%rcx), %rcx
Ltmp41:
	movl	$32, %esi
	movq	%rax, %rdi
	callq	*%rcx
	movb	%al, -73(%rbp)          ## 1-byte Spill
Ltmp42:
## BB#6:                                ## %_ZNKSt3__19basic_iosIcNS_11char_traitsIcEEE5widenEc.exit.i
	leaq	-48(%rbp), %rdi
	callq	__ZNSt3__16localeD1Ev
	movsbl	-73(%rbp), %eax         ## 1-byte Folded Reload
	movl	%eax, 144(%r12)
	movq	-72(%rbp), %rdi         ## 8-byte Reload
LBB3_7:
	addq	%r15, %r14
	andl	$176, %r13d
	cmpl	$32, %r13d
	movq	%r15, %rdx
	cmoveq	%r14, %rdx
Ltmp44:
	movsbl	%al, %r9d
	movq	%r15, %rsi
	movq	%r14, %rcx
	movq	%r12, %r8
	callq	__ZNSt3__116__pad_and_outputIcNS_11char_traitsIcEEEENS_19ostreambuf_iteratorIT_T0_EES6_PKS4_S8_S8_RNS_8ios_baseES4_
Ltmp45:
## BB#8:
	testq	%rax, %rax
	jne	LBB3_10
## BB#9:
	movq	(%rbx), %rax
	movq	-24(%rax), %rax
	leaq	(%rbx,%rax), %rdi
	movl	32(%rbx,%rax), %esi
	orl	$5, %esi
Ltmp46:
	callq	__ZNSt3__18ios_base5clearEj
Ltmp47:
LBB3_10:                                ## %_ZNSt3__19basic_iosIcNS_11char_traitsIcEEE8setstateEj.exit
	leaq	-64(%rbp), %rdi
	callq	__ZNSt3__113basic_ostreamIcNS_11char_traitsIcEEE6sentryD1Ev
LBB3_15:
	movq	%rbx, %rax
	addq	$40, %rsp
	popq	%rbx
	popq	%r12
	popq	%r13
	popq	%r14
	popq	%r15
	popq	%rbp
	retq
LBB3_11:
Ltmp48:
	movq	%rax, %r14
	jmp	LBB3_12
LBB3_20:
Ltmp36:
	movq	%rax, %r14
	jmp	LBB3_13
LBB3_19:
Ltmp43:
	movq	%rax, %r14
	leaq	-48(%rbp), %rdi
	callq	__ZNSt3__16localeD1Ev
LBB3_12:                                ## %.body
	leaq	-64(%rbp), %rdi
	callq	__ZNSt3__113basic_ostreamIcNS_11char_traitsIcEEE6sentryD1Ev
LBB3_13:
	movq	%rbx, %r15
	movq	%r14, %rdi
	callq	___cxa_begin_catch
	movq	(%rbx), %rax
	addq	-24(%rax), %r15
Ltmp49:
	movq	%r15, %rdi
	callq	__ZNSt3__18ios_base33__set_badbit_and_consider_rethrowEv
Ltmp50:
## BB#14:
	callq	___cxa_end_catch
	jmp	LBB3_15
LBB3_16:
Ltmp51:
	movq	%rax, %rbx
Ltmp52:
	callq	___cxa_end_catch
Ltmp53:
## BB#17:
	movq	%rbx, %rdi
	callq	__Unwind_Resume
LBB3_18:
Ltmp54:
	movq	%rax, %rdi
	callq	___clang_call_terminate
Lfunc_end2:
	.cfi_endproc
	.section	__TEXT,__gcc_except_tab
	.align	2
GCC_except_table3:
Lexception2:
	.byte	255                     ## @LPStart Encoding = omit
	.byte	155                     ## @TType Encoding = indirect pcrel sdata4
	.byte	125                     ## @TType base offset
	.byte	3                       ## Call site Encoding = udata4
	.byte	117                     ## Call site table length
Lset20 = Ltmp34-Lfunc_begin2            ## >> Call Site 1 <<
	.long	Lset20
Lset21 = Ltmp35-Ltmp34                  ##   Call between Ltmp34 and Ltmp35
	.long	Lset21
Lset22 = Ltmp36-Lfunc_begin2            ##     jumps to Ltmp36
	.long	Lset22
	.byte	1                       ##   On action: 1
Lset23 = Ltmp37-Lfunc_begin2            ## >> Call Site 2 <<
	.long	Lset23
Lset24 = Ltmp38-Ltmp37                  ##   Call between Ltmp37 and Ltmp38
	.long	Lset24
Lset25 = Ltmp48-Lfunc_begin2            ##     jumps to Ltmp48
	.long	Lset25
	.byte	1                       ##   On action: 1
Lset26 = Ltmp39-Lfunc_begin2            ## >> Call Site 3 <<
	.long	Lset26
Lset27 = Ltmp42-Ltmp39                  ##   Call between Ltmp39 and Ltmp42
	.long	Lset27
Lset28 = Ltmp43-Lfunc_begin2            ##     jumps to Ltmp43
	.long	Lset28
	.byte	1                       ##   On action: 1
Lset29 = Ltmp44-Lfunc_begin2            ## >> Call Site 4 <<
	.long	Lset29
Lset30 = Ltmp47-Ltmp44                  ##   Call between Ltmp44 and Ltmp47
	.long	Lset30
Lset31 = Ltmp48-Lfunc_begin2            ##     jumps to Ltmp48
	.long	Lset31
	.byte	1                       ##   On action: 1
Lset32 = Ltmp47-Lfunc_begin2            ## >> Call Site 5 <<
	.long	Lset32
Lset33 = Ltmp49-Ltmp47                  ##   Call between Ltmp47 and Ltmp49
	.long	Lset33
	.long	0                       ##     has no landing pad
	.byte	0                       ##   On action: cleanup
Lset34 = Ltmp49-Lfunc_begin2            ## >> Call Site 6 <<
	.long	Lset34
Lset35 = Ltmp50-Ltmp49                  ##   Call between Ltmp49 and Ltmp50
	.long	Lset35
Lset36 = Ltmp51-Lfunc_begin2            ##     jumps to Ltmp51
	.long	Lset36
	.byte	0                       ##   On action: cleanup
Lset37 = Ltmp50-Lfunc_begin2            ## >> Call Site 7 <<
	.long	Lset37
Lset38 = Ltmp52-Ltmp50                  ##   Call between Ltmp50 and Ltmp52
	.long	Lset38
	.long	0                       ##     has no landing pad
	.byte	0                       ##   On action: cleanup
Lset39 = Ltmp52-Lfunc_begin2            ## >> Call Site 8 <<
	.long	Lset39
Lset40 = Ltmp53-Ltmp52                  ##   Call between Ltmp52 and Ltmp53
	.long	Lset40
Lset41 = Ltmp54-Lfunc_begin2            ##     jumps to Ltmp54
	.long	Lset41
	.byte	1                       ##   On action: 1
Lset42 = Ltmp53-Lfunc_begin2            ## >> Call Site 9 <<
	.long	Lset42
Lset43 = Lfunc_end2-Ltmp53              ##   Call between Ltmp53 and Lfunc_end2
	.long	Lset43
	.long	0                       ##     has no landing pad
	.byte	0                       ##   On action: cleanup
	.byte	1                       ## >> Action Record 1 <<
                                        ##   Catch TypeInfo 1
	.byte	0                       ##   No further actions
                                        ## >> Catch TypeInfos <<
	.long	0                       ## TypeInfo 1
	.align	2

	.section	__TEXT,__textcoal_nt,coalesced,pure_instructions
	.private_extern	__ZNSt3__116__pad_and_outputIcNS_11char_traitsIcEEEENS_19ostreambuf_iteratorIT_T0_EES6_PKS4_S8_S8_RNS_8ios_baseES4_
	.globl	__ZNSt3__116__pad_and_outputIcNS_11char_traitsIcEEEENS_19ostreambuf_iteratorIT_T0_EES6_PKS4_S8_S8_RNS_8ios_baseES4_
	.weak_def_can_be_hidden	__ZNSt3__116__pad_and_outputIcNS_11char_traitsIcEEEENS_19ostreambuf_iteratorIT_T0_EES6_PKS4_S8_S8_RNS_8ios_baseES4_
	.align	4, 0x90
__ZNSt3__116__pad_and_outputIcNS_11char_traitsIcEEEENS_19ostreambuf_iteratorIT_T0_EES6_PKS4_S8_S8_RNS_8ios_baseES4_: ## @_ZNSt3__116__pad_and_outputIcNS_11char_traitsIcEEEENS_19ostreambuf_iteratorIT_T0_EES6_PKS4_S8_S8_RNS_8ios_baseES4_
Lfunc_begin3:
	.cfi_startproc
	.cfi_personality 155, ___gxx_personality_v0
	.cfi_lsda 16, Lexception3
## BB#0:
	pushq	%rbp
Ltmp66:
	.cfi_def_cfa_offset 16
Ltmp67:
	.cfi_offset %rbp, -16
	movq	%rsp, %rbp
Ltmp68:
	.cfi_def_cfa_register %rbp
	pushq	%r15
	pushq	%r14
	pushq	%r13
	pushq	%r12
	pushq	%rbx
	subq	$40, %rsp
Ltmp69:
	.cfi_offset %rbx, -56
Ltmp70:
	.cfi_offset %r12, -48
Ltmp71:
	.cfi_offset %r13, -40
Ltmp72:
	.cfi_offset %r14, -32
Ltmp73:
	.cfi_offset %r15, -24
	movq	%rcx, %r15
	movq	%rdx, %r12
	movq	%rdi, %r13
	xorl	%eax, %eax
	testq	%r13, %r13
	je	LBB4_9
## BB#1:
	movq	%r15, %rax
	subq	%rsi, %rax
	movq	24(%r8), %rcx
	xorl	%ebx, %ebx
	subq	%rax, %rcx
	cmovgq	%rcx, %rbx
	movq	%r12, %r14
	subq	%rsi, %r14
	testq	%r14, %r14
	jle	LBB4_3
## BB#2:
	movq	(%r13), %rax
	movq	%r13, %rdi
	movq	%r14, %rdx
	movq	%r15, -72(%rbp)         ## 8-byte Spill
	movq	%r12, -80(%rbp)         ## 8-byte Spill
	movq	%r8, %r12
	movl	%r9d, %r15d
	callq	*96(%rax)
	movl	%r15d, %r9d
	movq	%r12, %r8
	movq	-80(%rbp), %r12         ## 8-byte Reload
	movq	-72(%rbp), %r15         ## 8-byte Reload
	movq	%rax, %rcx
	xorl	%eax, %eax
	cmpq	%r14, %rcx
	jne	LBB4_9
LBB4_3:
	testq	%rbx, %rbx
	jle	LBB4_6
## BB#4:                                ## %_ZNKSt3__112basic_stringIcNS_11char_traitsIcEENS_9allocatorIcEEE4dataEv.exit
	movq	%r8, -72(%rbp)          ## 8-byte Spill
	xorps	%xmm0, %xmm0
	movaps	%xmm0, -64(%rbp)
	movq	$0, -48(%rbp)
	movsbl	%r9b, %edx
	leaq	-64(%rbp), %rdi
	movq	%rbx, %rsi
	callq	__ZNSt3__112basic_stringIcNS_11char_traitsIcEENS_9allocatorIcEEE6__initEmc
	testb	$1, -64(%rbp)
	leaq	-63(%rbp), %rsi
	cmovneq	-48(%rbp), %rsi
	movq	(%r13), %rax
	movq	96(%rax), %rax
Ltmp63:
	movq	%r13, %rdi
	movq	%rbx, %rdx
	callq	*%rax
	movq	%rax, %r14
Ltmp64:
## BB#5:                                ## %_ZNSt3__115basic_streambufIcNS_11char_traitsIcEEE5sputnEPKcl.exit
	leaq	-64(%rbp), %rdi
	callq	__ZNSt3__112basic_stringIcNS_11char_traitsIcEENS_9allocatorIcEEED1Ev
	xorl	%eax, %eax
	cmpq	%rbx, %r14
	movq	-72(%rbp), %r8          ## 8-byte Reload
	jne	LBB4_9
LBB4_6:
	subq	%r12, %r15
	testq	%r15, %r15
	jle	LBB4_8
## BB#7:
	movq	(%r13), %rax
	movq	%r13, %rdi
	movq	%r12, %rsi
	movq	%r15, %rdx
	movq	%r8, %rbx
	callq	*96(%rax)
	movq	%rbx, %r8
	movq	%rax, %rcx
	xorl	%eax, %eax
	cmpq	%r15, %rcx
	jne	LBB4_9
LBB4_8:
	movq	$0, 24(%r8)
	movq	%r13, %rax
LBB4_9:
	addq	$40, %rsp
	popq	%rbx
	popq	%r12
	popq	%r13
	popq	%r14
	popq	%r15
	popq	%rbp
	retq
LBB4_10:
Ltmp65:
	movq	%rax, %rbx
	leaq	-64(%rbp), %rdi
	callq	__ZNSt3__112basic_stringIcNS_11char_traitsIcEENS_9allocatorIcEEED1Ev
	movq	%rbx, %rdi
	callq	__Unwind_Resume
Lfunc_end3:
	.cfi_endproc
	.section	__TEXT,__gcc_except_tab
	.align	2
GCC_except_table4:
Lexception3:
	.byte	255                     ## @LPStart Encoding = omit
	.byte	155                     ## @TType Encoding = indirect pcrel sdata4
	.byte	41                      ## @TType base offset
	.byte	3                       ## Call site Encoding = udata4
	.byte	39                      ## Call site table length
Lset44 = Lfunc_begin3-Lfunc_begin3      ## >> Call Site 1 <<
	.long	Lset44
Lset45 = Ltmp63-Lfunc_begin3            ##   Call between Lfunc_begin3 and Ltmp63
	.long	Lset45
	.long	0                       ##     has no landing pad
	.byte	0                       ##   On action: cleanup
Lset46 = Ltmp63-Lfunc_begin3            ## >> Call Site 2 <<
	.long	Lset46
Lset47 = Ltmp64-Ltmp63                  ##   Call between Ltmp63 and Ltmp64
	.long	Lset47
Lset48 = Ltmp65-Lfunc_begin3            ##     jumps to Ltmp65
	.long	Lset48
	.byte	0                       ##   On action: cleanup
Lset49 = Ltmp64-Lfunc_begin3            ## >> Call Site 3 <<
	.long	Lset49
Lset50 = Lfunc_end3-Ltmp64              ##   Call between Ltmp64 and Lfunc_end3
	.long	Lset50
	.long	0                       ##     has no landing pad
	.byte	0                       ##   On action: cleanup
	.align	2

	.section	__TEXT,__cstring,cstring_literals
L_.str:                                 ## @.str
	.asciz	"Elapsed seconds: "

L_.str.1:                               ## @.str.1
	.asciz	"Sum is "


.subsections_via_symbols
