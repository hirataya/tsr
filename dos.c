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
 * ���: asciz������������ɃR���\�[���o�͂��܂�
 * �錾: void dos_puts_fast(const char *pstr);
 * ����: const char *pstr --- �\������asciz������
 * �ߒl: void --- �Ȃ�
 * ���l: ���_�C���N�g�E�p�C�v�͎g�p�s�\�ł�
 *       '\n'��'\r'��t�����܂�
 *       puts�ƈႢ�A������'\n'(+'\r')�͕t������܂���
 *       ����J���荞��29H���g�p���Ă��܂��B
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
 * ���: 1�����������ɃR���\�[���o�͂��܂�
 * �錾: void dos_putchar_fast(char c);
 * ����: char c --- �\�����镶��
 * �ߒl: void --- �Ȃ�
 * ���l: ���_�C���N�g�E�p�C�v�͎g�p�s�\�ł�
 *       '\n'��'\r'��t�����܂�
 *       ����J���荞��29H���g�p���Ă��܂��B
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
 * ���: ���t���擾���܂�
 * �錾: void dos_get_date(struct dos_date *pdate);
 * ����: struct dos_date *pdate --- ���t�����i�[����A�h���X
 * �ߒl: void --- �Ȃ�
 * ���l: MS-DOS�V�X�e���R�[��2AH���g�p���Ă��܂�
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
 * ���: �������擾���܂�
 * �錾: void dos_get_time(struct dos_time *ptime);
 * ����: struct dos_time *ptime --- ���������i�[����A�h���X
 * �ߒl: void --- �Ȃ�
 * ���l: MS-DOS�V�X�e���R�[��2CH���g�p���Ă��܂�
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
 * ���: �擪��MCB���w��far�|�C���^�[���擾���܂�
 * �錾: struct mcb far *dos_get_top_mcb(void);
 * ����: void --- �Ȃ�
 * �ߒl: struct mcb far * --- �擪��MCB���w��far�|�C���^�[
 * ���l: MS-DOS����J�V�X�e���R�[��52H���g�p���Ă��܂�
 */
struct mcb far *dos_get_top_mcb(void)
{
	asm MOV AH,52H;
	asm INT 21H;
	
	return MK_FP(*((WORD far *)MK_FP(_ES, _BX-2)), 0x0000);
}

/* dos_get_next_mcb
 * 
 * ���: ����MCB���w��far�|�C���^�[���擾���܂�
 * �錾: struct mcb far *dos_get_next_mcb(struct mcb far *pmcb);
 * ����: struct mcb far *pmcb --- MCB���w��far�|�C���^�[
 * �ߒl: struct mcb far * --- ����MCB���w��far�|�C���^�[
 * ���l: �ʏ�̓}�N���W�J����܂�
 */
struct mcb far *dos_get_next_mcb(struct mcb far *pmcb)
{
	return MK_FP(FP_SEG(pmcb)+pmcb->size+1, 0x0000);
}

/* dos_remove_env
 * 
 * ���: ���ݎ��s���̃v���O�����̊��ϐ��̈���J�����܂�
 * �錾: BOOL dos_remove_env(void);
 * ����: void --- �Ȃ�
 * �ߒl: BOOL --- TRUE: ����  FALSE: ���s
 * ���l: 2��ڈȍ~�̌Ăяo���ł�1��ڂ̐����E���s�ɂ�����炸���FALSE��
 *       �Ԃ��܂�
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
 * ���: �������[�u���b�N���J�����܂�
 * �錾: BOOL dos_remove_mem(WORD seg);
 * ����: WORD seg --- �J�����郁�����[�̃Z�O�����g(�I�t�Z�b�g��0000H)
 * �ߒl: BOOL --- TRUE: ����  FALSE: ���s
 * ���l: MS-DOS�V�X�e���R�[��49H���g�p���Ă��܂�
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
 * ���: �v���O�������풓�I�����܂�
 * �錾: BOOL dos_keep(BYTE retcode, WORD size);
 * ����: BYTE retcode --- �v���O�����̏I���R�[�h(errorlevel�ɕԂ����l)
 *       WORD size --- �p���O���t�P�ʂ̏풓�T�C�Y
 * �ߒl: BOOL --- TRUE: (����)  FALSE: ���s
 * ���l: �풓�I�����ɂ̓v���O�����ɐ��䂪�߂��Ă��Ȃ��̂ŕԂ�l�͏��FALSE��
 *       �Ȃ�܂�
 *       ���ۂ̎g�p�ł�tsr_stay���g�p������������ł��傤�B
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
 * ���: ���荞�݃x�N�^�[���擾���܂�
 * �錾: void interrupt (*dos_get_vect(BYTE intno))();
 * ����: BYTE intno --- ���荞�ݔԍ�
 * �ߒl: void interrupt (*)() --- ���荞�݃x�N�^�[���w��(far)�|�C���^�[
 * ���l: MS-DOS�V�X�e���R�[��35H���g�p���Ă��܂�
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
 * ���: ���荞�݃x�N�^�[��ݒ肵�܂�
 * �錾: void dos_set_vect(BYTE intno, void interrupt (*handler)());
 * ����: BYTE intno --- ���荞�ݔԍ�
 *       void interrupt (*handler)() --- ���荞�ݐ���w��(far)�|�C���^�[
 * �ߒl: void --- �Ȃ�
 * ���l: MS-DOS�V�X�e���R�[��25H���g�p���Ă��܂�
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
 * ���: UMB�����N��Ԃ��擾���܂�
 * �錾: BYTE dos_get_umb_link(void);
 * ����: void --- �Ȃ�
 * �ߒl: BYTE --- 0x00: �����N����Ă��Ȃ�  0x01:�����N����Ă���
 * ���l: MS-DOS�V�X�e���R�[��5802H���g�p���Ă��܂�
 */
BYTE dos_get_umb_link(void)
{
	asm MOV AX,5802H;
	asm INT 21H;
	
	return _AL;
}

/* dos_set_umb_link
 * 
 * ���: UMB�����N��Ԃ�ݒ肵�܂�
 * �錾: BOOL dos_set_umb_link(WORD cmd)
 * ����: WORD cmd --- 0x0000: �����N���Ȃ�  0x0001: �����N����
 * �ߒl: BOOL --- TRUE: ����  FALSE: ���s
 * ���l: MS-DOS�o�[�W����5�����܂���UMB�������ł���ꍇ�AFALSE���Ԃ�܂�
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
