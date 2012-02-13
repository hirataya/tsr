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

#include "yasu.h"

void gr_init(void);
void gr_set_pal(BYTE pal, BYTE r, BYTE g, BYTE b);

/* gr_init
 * 
 * 解説: グラフィック画面を初期化します
 * 宣言: void gr_init(void);
 * 引数: void --- なし
 * 戻値: void --- なし
 * 備考: アナログ16色、表示・書込バンク0、400ラインモードで初期化します
 */
void gr_init(void)
{
	asm MOV AH,42H;
	asm MOV CH,0C0H;
	asm INT 18H;
	
	asm MOV AL,00H;
	asm OUT 0A4H,AL;
	asm OUT 0A6H,AL;
	
	asm INC AL;		/* MOV AH,01H */
	asm OUT 6AH,AL;
	
	return;
}

/* gr_set_pal
 * 
 * 解説: アナログパレットを設定します
 * 宣言: void gr_set_pal(BYTE pal, BYTE r, BYTE g, BYTE b)
 * 引数: BYTE pal --- パレット番号(0--15)
 *       BYTE r --- 赤の輝度
 *       BYTE g --- 緑の輝度
 *       BYTE b --- 青の輝度
 * 戻値: void
 * 備考: 
 */
void gr_set_pal(BYTE pal, BYTE r, BYTE g, BYTE b)
{
	asm MOV AL,pal;
	asm OUT 0A8H,AL;
	asm MOV AL,r;
	asm OUT 0ACH,AL;
	asm MOV AL,g;
	asm OUT 0AAH,AL;
	asm MOV AL,b;
	asm OUT 0AEH,AL;
	
	return;
}
