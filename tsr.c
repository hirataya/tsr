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
#include <string.h>
#include "dos.h"
#include "yasu.h"

WORD tsr_get_stayed_seg(WORD idofs, const char *idstr);
BOOL tsr_is_stayed(WORD idofs, const char *idstr);
BOOL tsr_stay(BYTE retcode, WORD idofs, const char *idstr, WORD size);
BOOL tsr_remove(WORD idofs, const char *idstr);

extern void _restorezero(void);			/* 非公開ライブラリー関数 */

/* tsr_get_stayed_seg
 * 
 * 解説: プログラムの常駐セグメントを取得します
 * 宣言: WORD tsr_get_stayed_seg(WORD idofs, const char *idstr);
 * 引数: WORD idofs --- 常駐id文字列の開始オフセット
 *       const char *idstr --- 常駐id文字列
 * 戻値: WORD --- 0x0000: 常駐していない  ≠0x0000: 常駐セグメント
 * 備考: 
 */
WORD tsr_get_stayed_seg(WORD idofs, const char *idstr)
{
	BYTE by;
	struct mcb far *pmcb;
	
	if(_osmajor>=5)
	{
		by=dos_get_umb_link();
		dos_set_umb_link(0x0000);
	}
	
	pmcb=dos_get_top_mcb();
	for(;;)
	{
		if((pmcb->owner!=0x0000)&&
		(_fstrcmp(MK_FP(FP_SEG(pmcb)+1, idofs), idstr)==0))
		{
			if(_osmajor>=5)
				dos_set_umb_link(by? 0x0000: 0x0001);
			return FP_SEG(pmcb)+1;
		}
		if(pmcb->id=='Z')
			break;
		pmcb=dos_get_next_mcb(pmcb);
	}
	
	if(_osmajor>=5)
		dos_set_umb_link(by? 0x0000: 0x0001);
	
	return 0x0000;
}

/* tsr_is_stayed
 * 
 * 解説: プログラムの常駐を判定します
 * 宣言: BOOL tsr_is_stayed(WORD idofs, const char *idstr);
 * 引数: WORD idofs --- 常駐id文字列の開始オフセット
 *       const char *idstr --- 常駐id文字列
 * 戻値: WORD --- TRUE: 常駐している  FALSE: 常駐していない
 * 備考: 通常はマクロ展開されます
 */
BOOL tsr_is_stayed(WORD idofs, const char *idstr)
{
	return tsr_get_stayed_seg(idofs, idstr)? TRUE: FALSE;
}

/* tsr_stay
 * 
 * 解説: プログラムを常駐します
 * 宣言: BOOL tsr_stay(BYTE retcode, WORD idofs, const char *idstr, WORD size);
 * 引数: BYTE retcode --- プログラム終了コード
 *       WORD idofs --- 常駐id文字列の開始オフセット
 *       const char *idstr --- 常駐id文字列
 *       WORD size --- 常駐サイズ(パラグラフ単位)
 * 戻値: BOOL --- TRUE: (成功)  FALSE: 失敗
 * 備考: 常駐に成功するとプログラムに制御が戻ってこないので常にFALSEを返します
 *       idofs+strlen(idstr)<0xFFの条件をプログラマーの責任で満足させる必要
 *       があります
 *       内部で非公開ライブラリー関数_restorezeroを呼び出しています
 *       idofsの推奨値は0x0080または0x0081です
 */
BOOL tsr_stay(BYTE retcode, WORD idofs, const char *idstr, WORD size)
{
	_restorezero();
	
	if(tsr_is_stayed(idofs, idstr))
		return FALSE;
	
	_fstrcpy(MK_FP(_psp, idofs), idstr);
	
	dos_keep(retcode, size);
	
	return FALSE;
}

/* tsr_remove
 * 
 * 解説: プログラムの常駐を解除します
 * 宣言: BOOL tsr_remove(WORD idofs, const char *idstr);
 * 引数: WORD idofs --- 常駐id文字列の開始オフセット
 *       const char *idstr --- 常駐id文字列
 * 戻値: BOOL --- TRUE: 成功  FALSE: 失敗
 * 備考: 
 */
BOOL tsr_remove(WORD idofs, const char *idstr)
{
	WORD seg;
	
	if((seg=tsr_get_stayed_seg(idofs, idstr))==0x0000)
		return FALSE;
	
	return dos_remove_mem(seg);
}
