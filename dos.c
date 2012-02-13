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

void dos_puts_fast(const char *pstr);
void dos_putchar_fast(char c);
void dos_get_date(struct dos_date *pdate);
void dos_get_time(struct dos_time *ptime);
struct mcb far *dos_get_top_mcb(void);
struct mcb far *dos_get_next_mcb(struct mcb far *pmcb);
BOOL dos_remove_env(void);
BOOL dos_remove_mem(WORD seg);
BOOL dos_keep(BYTE retcode, WORD size);
void interrupt (*dos_get_vect(BYTE intno))();
void dos_set_vect(BYTE intno, void interrupt (*handler)());
BYTE dos_get_umb_link(void);
BOOL dos_set_umb_link(WORD cmd);

/* dos_puts_fast
 * 
 * 解説: asciz文字列を高速にコンソール出力します
 * 宣言: void dos_puts_fast(const char *pstr);
 * 引数: const char *pstr --- 表示するasciz文字列
 * 戻値: void --- なし
 * 備考: リダイレクト・パイプは使用不可能です
 *       '\n'に'\r'を付加します
 *       putsと違い、末尾に'\n'(+'\r')は付加されません
 *       非公開割り込み29Hを使用しています。
 */
void dos_puts_fast(const char *pstr)
{
	for(; *pstr; pstr++)
	{
		_AL=*pstr;
		asm INT 29H;
		if(_AL=='\n')
		{
			asm MOV AL,0DH;
			asm INT 29H;
		}
	}
	
	return;
}

/* dos_putchar_fast
 * 
 * 解説: 1文字を高速にコンソール出力します
 * 宣言: void dos_putchar_fast(char c);
 * 引数: char c --- 表示する文字
 * 戻値: void --- なし
 * 備考: リダイレクト・パイプは使用不可能です
 *       '\n'に'\r'を付加します
 *       非公開割り込み29Hを使用しています。
 */
void dos_putchar_fast(char c)
{
	asm mov AL,c;
	asm INT 29H;
	
	if(_AL=='\n')
	{
		asm MOV AL,0DH;
		asm INT 29H;
	}
	
	return;
}

/* dos_get_date
 * 
 * 解説: 日付を取得します
 * 宣言: void dos_get_date(struct dos_date *pdate);
 * 引数: struct dos_date *pdate --- 日付情報を格納するアドレス
 * 戻値: void --- なし
 * 備考: MS-DOSシステムコール2AHを使用しています
 */
void dos_get_date(struct dos_date *pdate)
{
	asm MOV AH,2AH;
	asm INT 21H;
	
	pdate->year=_CX;
	pdate->month=_DH;
	pdate->day=_DL;
	pdate->yobi=_AL;
	
	return;
}

/* dos_get_time
 * 
 * 解説: 時刻を取得します
 * 宣言: void dos_get_time(struct dos_time *ptime);
 * 引数: struct dos_time *ptime --- 時刻情報を格納するアドレス
 * 戻値: void --- なし
 * 備考: MS-DOSシステムコール2CHを使用しています
 */
void dos_get_time(struct dos_time *ptime)
{
	asm MOV AH,2CH;
	asm INT 21H;
	
	ptime->hour=_CH;
	ptime->min=_CL;
	ptime->sec=_DH;
	
	return;
}

/* dos_get_top_mcb
 * 
 * 解説: 先頭のMCBを指すfarポインターを取得します
 * 宣言: struct mcb far *dos_get_top_mcb(void);
 * 引数: void --- なし
 * 戻値: struct mcb far * --- 先頭のMCBを指すfarポインター
 * 備考: MS-DOS非公開システムコール52Hを使用しています
 */
struct mcb far *dos_get_top_mcb(void)
{
	asm MOV AH,52H;
	asm INT 21H;
	
	return MK_FP(*((WORD far *)MK_FP(_ES, _BX-2)), 0x0000);
}

/* dos_get_next_mcb
 * 
 * 解説: 次のMCBを指すfarポインターを取得します
 * 宣言: struct mcb far *dos_get_next_mcb(struct mcb far *pmcb);
 * 引数: struct mcb far *pmcb --- MCBを指すfarポインター
 * 戻値: struct mcb far * --- 次のMCBを指すfarポインター
 * 備考: 通常はマクロ展開されます
 */
struct mcb far *dos_get_next_mcb(struct mcb far *pmcb)
{
	return MK_FP(FP_SEG(pmcb)+pmcb->size+1, 0x0000);
}

/* dos_remove_env
 * 
 * 解説: 現在実行中のプログラムの環境変数領域を開放します
 * 宣言: BOOL dos_remove_env(void);
 * 引数: void --- なし
 * 戻値: BOOL --- TRUE: 成功  FALSE: 失敗
 * 備考: 2回目以降の呼び出しでは1回目の成功・失敗にかかわらず常にFALSEを
 *       返します
 */
BOOL dos_remove_env(void)
{
	static BOOL first=TRUE;
	
	if(!first)
		return FALSE;
	else
		first=FALSE;
	
	return dos_remove_mem(*((WORD far *)MK_FP(_psp, 0x002C)));
}

/* dos_remove_mem
 * 
 * 解説: メモリーブロックを開放します
 * 宣言: BOOL dos_remove_mem(WORD seg);
 * 引数: WORD seg --- 開放するメモリーのセグメント(オフセットは0000H)
 * 戻値: BOOL --- TRUE: 成功  FALSE: 失敗
 * 備考: MS-DOSシステムコール49Hを使用しています
 */
BOOL dos_remove_mem(WORD seg)
{
	asm MOV ES,seg;
	asm MOV AH,49H;
	asm INT 21H;
	asm JC error;

no_error:
	return TRUE;

error:
	return FALSE;
}

/* dos_keep
 * 
 * 解説: プログラムを常駐終了します
 * 宣言: BOOL dos_keep(BYTE retcode, WORD size);
 * 引数: BYTE retcode --- プログラムの終了コード(errorlevelに返される値)
 *       WORD size --- パラグラフ単位の常駐サイズ
 * 戻値: BOOL --- TRUE: (成功)  FALSE: 失敗
 * 備考: 常駐終了時にはプログラムに制御が戻ってこないので返り値は常にFALSEと
 *       なります
 *       実際の使用ではtsr_stayを使用する方が酔いでしょう。
 */
BOOL dos_keep(BYTE retcode, WORD size)
{
	asm MOV DX,size;
	asm MOV AL,retcode;
	asm MOV AH,31H;
	asm INT 21H;
	
	return FALSE;
}

/* dos_get_vect
 * 
 * 解説: 割り込みベクターを取得します
 * 宣言: void interrupt (*dos_get_vect(BYTE intno))();
 * 引数: BYTE intno --- 割り込み番号
 * 戻値: void interrupt (*)() --- 割り込みベクターを指す(far)ポインター
 * 備考: MS-DOSシステムコール35Hを使用しています
 */
void interrupt (*dos_get_vect(BYTE intno))()
{
	asm MOV AH,35H;
	asm MOV AL,intno;
	asm INT 21H;
	
	return MK_FP(_ES, _BX);
}

/* dos_set_vect
 * 
 * 解説: 割り込みベクターを設定します
 * 宣言: void dos_set_vect(BYTE intno, void interrupt (*handler)());
 * 引数: BYTE intno --- 割り込み番号
 *       void interrupt (*handler)() --- 割り込み先を指す(far)ポインター
 * 戻値: void --- なし
 * 備考: MS-DOSシステムコール25Hを使用しています
 */
void dos_set_vect(BYTE intno, void interrupt (*handler)())
{
	asm PUSH DS;
	_DS=FP_SEG(handler);
	_DX=FP_OFF(handler);
	asm MOV AH,25H;
	asm MOV AL,intno;
	asm INT 21H;
	asm POP DS;
	
	return;
}

/* dos_get_umb_link
 * 
 * 解説: UMBリンク状態を取得します
 * 宣言: BYTE dos_get_umb_link(void);
 * 引数: void --- なし
 * 戻値: BYTE --- 0x00: リンクされていない  0x01:リンクされている
 * 備考: MS-DOSシステムコール5802Hを使用しています
 */
BYTE dos_get_umb_link(void)
{
	asm MOV AX,5802H;
	asm INT 21H;
	
	return _AL;
}

/* dos_set_umb_link
 * 
 * 解説: UMBリンク状態を設定します
 * 宣言: BOOL dos_set_umb_link(WORD cmd)
 * 引数: WORD cmd --- 0x0000: リンクしない  0x0001: リンクする
 * 戻値: BOOL --- TRUE: 成功  FALSE: 失敗
 * 備考: MS-DOSバージョン5未満またはUMBが無効である場合、FALSEが返ります
 */
BOOL dos_set_umb_link(WORD cmd)
{
	asm MOV AX,5803H;
	asm MOV BX,cmd;
	asm INT 21H;
	asm JC error;
	
no_error:
	return TRUE;

error:
	return FALSE;
}
