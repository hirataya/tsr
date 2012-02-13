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

#include <dos.h>
#include "yasu.h"

void timer_set(WORD time, void interrupt (*handler)());

/* timer_set
 * 
 * 解説: インターバルタイマーをbiosを使用して設定します
 * 宣言: void timer_set(WORD time, void interrupt (*handler)());
 * 引数: WORD time --- 10ms単位の時間
 *       void interrupt (*handler)() --- 割り込みハンドラー
 * 戻値: void --- なし
 * 備考: 
 */
void timer_set(WORD time, void interrupt (*handler)())
{
	_ES=FP_SEG(handler);
	_BX=FP_OFF(handler);
	
	asm	MOV AH,02H;
	asm MOV CX,time;
	asm INT 1CH;
	
	return;
}
