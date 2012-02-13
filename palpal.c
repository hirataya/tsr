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

#include <stdlib.h>
#include <dos.h>
#include "gr.h"
#include "dos.h"
#include "timer.h"
#include "tsr.h"
#include "yasu.h"

#define PALPALVER "0.01"
#define IDOFS 0x0080
#define IDSTR "$$PALPAL$$"
#define INTERVAL 10

void interrupt palpal(UINT bp, UINT di, UINT si, UINT ds, UINT es, 
UINT dx, UINT cx, UINT bx, UINT ax);
void palpal_stay(void);
void palpal_remove(void);
void usage(void);
void error_exit(const char *msg);

extern UINT _heaplen=1;					/* 最低値(0だと64kB指定) */
extern UINT _stklen=0x100;				/* 少なくしておく */
extern WORD __heapbase;					/* HEAPの開始オフセット */

int main(int argc, char **argv)
{
	const static char msg[]=
	"This is palpal, TSR Version " PALPALVER "\n"
	"Copr. 1993-1994, YASU/Project-NCDW/"
	"TOKAI Univ. DAIYON HIGHSCHOOL SUKEN\n"
	"All rights reserved.\n";
	struct dos_time time;
	
	dos_puts_fast(msg);
	
	if((argc==2)&&(argv[1][0]=='-'))
	{
		dos_get_time(&time);
		srand(time.sec|1);
		
		switch(argv[1][1])
		{
			case 's':					/* 常駐 */
			case 'S':
				palpal_stay();
				break;
			case 'r':					/* 解除 */
			case 'R':
				palpal_remove();
				break;
			default:					/* 使用法表示 */
				usage();
				break;
		}
		
		return EXIT_SUCCESS;
	}
	
	usage();
	return EXIT_SUCCESS;
}

/* 割り込みルーチン */
#pragma warn -par					/* 使用していない引数にたいする警告抑止 */
void interrupt palpal(UINT bp, UINT di, UINT si, UINT ds, UINT es, 
UINT dx, UINT cx, UINT bx, UINT ax)
{
	register int i;
	static int col[3]={0, 0, 0};		/* アナログrgbの値 */
	static signed int pl[3]={0, 0, 0};	/* 毎回この値をcol[n]に足す */
	static int counter=0;				/* ﾊﾟﾚｯﾄの値を0--10にするためのｶｳﾝﾀｰ */
	
	counter++;
	counter%=11;
	if(counter==0)
		do
		{
			for(i=0; i<3; i++)
				if(rand()%2)
					pl[i]=(col[i]==0)?(+1):(-1);
				else
					pl[i]=0;
		}
		while(pl[0]==pl[1]==pl[2]==0);
	
	for(i=0; i<3; i++)
		col[i]+=pl[i];
	
	do									/* V-SYNC待ち */
		asm IN AL,0A0H;
	while(!(_AL&0x20));
	
	gr_set_pal(0, col[0], col[1], col[2]);
	
	timer_set(INTERVAL, palpal);
										/* ｲﾝﾀｰﾊﾞﾙﾀｲﾏｰはﾜﾝｼｮｯﾄなので再設定 */
	return;
}
#pragma warn .par

/* 常駐 */
void palpal_stay(void)
{
	if(tsr_is_stayed(IDOFS, IDSTR))
		error_exit("already stayed.\n");
	
	if(!dos_remove_env())
		error_exit("unable to remove env. area.\n");
	
	gr_init();
	
	*((DWORD far *)MK_FP(_psp, IDOFS+sizeof(IDSTR)))=(DWORD)dos_get_vect(0x07);
	
	dos_puts_fast("stayed on memory.\n");
	
	timer_set(INTERVAL, palpal);
	
	tsr_stay(EXIT_SUCCESS, IDOFS, IDSTR, _DS-_psp+(__heapbase/16+16));
	
	return;								/* dummy */
}

/* 解除 */
void palpal_remove(void)
{
	WORD seg;
	
	if((seg=tsr_get_stayed_seg(IDOFS, IDSTR))==0x0000)
		error_exit("not stayed.\n");
	
	dos_set_vect(0x07,
	(void interrupt (*)())(*((DWORD far *)MK_FP(seg, IDOFS+sizeof(IDSTR)))));
	
	if(!tsr_remove(IDOFS, IDSTR))
		error_exit("unable to remove from memory.\n");
	
	dos_puts_fast("removed from memory.\n");
	
	gr_set_pal(0, 0, 0, 0);
	
	exit(EXIT_SUCCESS);
	
	return;
}

/* 仕様法表示 */
void usage(void)
{
	const char msg[]=
	"Usage: palpal <option>\n"
	"    option: -s stay on memory\n"
	"            -r remove from memory\n"
	"            -? print help message";
	
	dos_puts_fast(msg);
	
	return;
}

/* エラー終了 */
void error_exit(const char *msg)
{
	dos_puts_fast("error: ");
	dos_puts_fast(msg);
	
	exit(EXIT_FAILURE);
}
