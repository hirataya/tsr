/* -*- coding: shift_jis -*-
 *
 * Copyright (C) 1994 HIRATA Yasuyuki <yasu@asuka.net>,
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR (HIRATA Yasuyuki) ``AS IS''
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
 * TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
 * PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF
 * USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#if !defined __DOS_H__
	#define __DOS_H__
	
	#include <dos.h>
	#include "yasu.h"
	
	#define dos_get_next_mcb(pmcb)		\
		(MK_FP(FP_SEG((pmcb))+(pmcb)->size+1, 0x0000))
	
	struct dos_date
	{
		int year;
		char month;
		char day;
		char yobi;
	};
	
	struct dos_time
	{
		char hour;
		char min;
		char sec;
	};
	
	struct mcb
	{
		BYTE id;
		WORD owner;
		WORD size;
		BYTE reserve[3];
		char name[8];
	};
	
	extern void dos_puts_fast(const char *pstr);
	extern void dos_putchar_fast(char c);
	extern void dos_get_date(struct dos_date *pdate);
	extern void dos_get_time(struct dos_time *ptime);
	extern struct mcb far *dos_get_top_mcb(void);
	extern struct mcb far *(dos_get_next_mcb)(struct mcb far *pmcb);
	extern BOOL dos_remove_env(void);
	extern BOOL dos_remove_mem(WORD seg);
	extern BOOL dos_keep(BYTE retcode, WORD size);
	extern void interrupt (*dos_get_vect(BYTE intno))();
	extern void dos_set_vect(BYTE intno, void interrupt (*handler)());
	extern BYTE dos_get_umb_link(void);
	extern BOOL dos_set_umb_link(WORD cmd);
#endif
