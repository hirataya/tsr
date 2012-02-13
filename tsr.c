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

extern void _restorezero(void);			/* ����J���C�u�����[�֐� */

/* tsr_get_stayed_seg
 * 
 * ���: �v���O�����̏풓�Z�O�����g���擾���܂�
 * �錾: WORD tsr_get_stayed_seg(WORD idofs, const char *idstr);
 * ����: WORD idofs --- �풓id������̊J�n�I�t�Z�b�g
 *       const char *idstr --- �풓id������
 * �ߒl: WORD --- 0x0000: �풓���Ă��Ȃ�  ��0x0000: �풓�Z�O�����g
 * ���l: 
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
 * ���: �v���O�����̏풓�𔻒肵�܂�
 * �錾: BOOL tsr_is_stayed(WORD idofs, const char *idstr);
 * ����: WORD idofs --- �풓id������̊J�n�I�t�Z�b�g
 *       const char *idstr --- �풓id������
 * �ߒl: WORD --- TRUE: �풓���Ă���  FALSE: �풓���Ă��Ȃ�
 * ���l: �ʏ�̓}�N���W�J����܂�
 */
BOOL tsr_is_stayed(WORD idofs, const char *idstr)
{
	return tsr_get_stayed_seg(idofs, idstr)? TRUE: FALSE;
}

/* tsr_stay
 * 
 * ���: �v���O�������풓���܂�
 * �錾: BOOL tsr_stay(BYTE retcode, WORD idofs, const char *idstr, WORD size);
 * ����: BYTE retcode --- �v���O�����I���R�[�h
 *       WORD idofs --- �풓id������̊J�n�I�t�Z�b�g
 *       const char *idstr --- �풓id������
 *       WORD size --- �풓�T�C�Y(�p���O���t�P��)
 * �ߒl: BOOL --- TRUE: (����)  FALSE: ���s
 * ���l: �풓�ɐ�������ƃv���O�����ɐ��䂪�߂��Ă��Ȃ��̂ŏ��FALSE��Ԃ��܂�
 *       idofs+strlen(idstr)<0xFF�̏������v���O���}�[�̐ӔC�Ŗ���������K�v
 *       ������܂�
 *       �����Ŕ���J���C�u�����[�֐�_restorezero���Ăяo���Ă��܂�
 *       idofs�̐����l��0x0080�܂���0x0081�ł�
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
 * ���: �v���O�����̏풓���������܂�
 * �錾: BOOL tsr_remove(WORD idofs, const char *idstr);
 * ����: WORD idofs --- �풓id������̊J�n�I�t�Z�b�g
 *       const char *idstr --- �풓id������
 * �ߒl: BOOL --- TRUE: ����  FALSE: ���s
 * ���l: 
 */
BOOL tsr_remove(WORD idofs, const char *idstr)
{
	WORD seg;
	
	if((seg=tsr_get_stayed_seg(idofs, idstr))==0x0000)
		return FALSE;
	
	return dos_remove_mem(seg);
}
