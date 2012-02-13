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

#if !defined __YASU_H__
	#define __YASU_H__
	
	typedef unsigned char BYTE;
	typedef unsigned short int WORD;
	typedef unsigned int UINT;
	typedef unsigned long int DWORD;
	typedef enum{FALSE,TRUE} BOOL;
	
	/* SWAP
	 * 
	 * 解説: 2つの変数を入れ替えます
	 * 引数: a, b --- 入れ替える変数
	 * 備考: 複数回引数が評価される
	 *       ポインターには対応していない
	 */
	#define SWAP(a,b)					\
	do									\
	{									\
		(a)^=(b);						\
		(b)^=(a);						\
		(a)^=(b);						\
	}									\
	while(0)
	
#endif
