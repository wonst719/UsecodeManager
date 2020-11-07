/* UsecodeManager
 * Copyright (C) 2005,2006,2010,2020 Won Star
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 */

#include "file.h"
#include "usecode.h"

// 텍스트 파일에 쓰고, 라인 피드를 추가.
#define WRITE_LINE do { file.write(writeBuf, strlen(writeBuf)); \
	file.writeByte(0xd); \
	file.writeByte(0xa); \
} while(0)

// 텍스트 파일에 쓰지만, 라인피드를 추가하지 않는다. 연속된 문장일 경우 사용
#define WRITE do { file.write(writeBuf, strlen(writeBuf)); } while(0)

// 알 수 없는 opcode들을 파일에 쓴다.
#define FLUSH_UNK_BUF do {\
		if(strlen(unkBuf)) {\
			sprintf(writeBuf, "unk_op ");\
			WRITE;\
			file.write(unkBuf, strlen(unkBuf));\
			file.writeByte(0xd);\
			file.writeByte(0xa);\
			unkBuf[0] = 0;\
		}\
} while(0)

// 2바이트의 LE형 워드를 읽는다. 단 포인터의 위치는 증가되지 않는다.
inline int READ_UINT16_LE(byte *ptr)
{
	return ptr[0] + (ptr[1] * 256);
}

bool UseCodeManager::exportFuncTextOldType(int num)
{
	File file;
	File msgFile;
	char fn[256];
	sprintf(fn, "SCRIPTS/USE_%04d.DEC", num);
	if(!file.open(fn, workDir, File::kFileWriteMode)) {
		printf("%s 파일에 쓸 수 없습니다!\n", fn);
		return 0;
	}
	sprintf(fn, "MESSAGES/USE_%04d.MSG", num);

	bool msgAlreadyExists = false;

	if (msgFile.open(fn, workDir, File::kFileReadMode))
	{
		msgFile.close();
		msgAlreadyExists = true;
	}

	if (!msgAlreadyExists)
	{
		if(!msgFile.open(fn, workDir, File::kFileWriteMode)) {
			printf("%s 파일에 쓸 수 없습니다!\n", fn);
			return 0;
		}
	}

	STable *t = &useTable[num];
	byte *srcPtr = t->data;

	char writeBuf[1024] = {0,};

	int strSize = READ_UINT16_LE(srcPtr);
	srcPtr += 2;

	//Message Buffer를 추출한다.
	byte *stringPtr = srcPtr;
	int messageNum = 0;
	int messagePosTable[4000] = {0,};
	while(stringPtr < srcPtr + strSize) {
		char stringBuf[1024];
		strcpy(stringBuf, (char *)stringPtr);
		messagePosTable[messageNum] = stringPtr - srcPtr;

		int strBufSize = strlen(stringBuf);
		for(int i = 0; i < strBufSize - 1; i++) {
			if(stringBuf[i] == 0x0d && stringBuf[i + 1] == 0x0a) {
				stringBuf[i] = '\\';
				stringBuf[i + 1] = 'n';
			}
		}
		sprintf(writeBuf, "[%04d]%s", messageNum, stringBuf);

		if (!msgAlreadyExists)
		{
			msgFile.write(writeBuf, strlen(writeBuf));
			msgFile.writeByte(0xd);
			msgFile.writeByte(0xa);
		}

		stringPtr += strlen((char *)stringPtr) + 1;
		messageNum++;
	}
	if (!msgAlreadyExists)
	{
		msgFile.close();
	}

	sprintf(writeBuf, "msg_nr %d", messageNum);
	WRITE_LINE;

	//현재 Function의 ID를 기록한다.
	sprintf(writeBuf, "func_s %d", t->id);
	WRITE_LINE;

	// 문자열이 끝난 다음으로 포인터를 돌린다.
	srcPtr = stringPtr;

	char *unkBuf;
	unkBuf = new char[t->size + 10000];
	unkBuf[0] = 0;

	// 디컴파일 시작.
	while(srcPtr < t->data + t->size) {
		byte opcode = *srcPtr++;
		if(opcode == 0x1c || opcode == 0x1d) {
			FLUSH_UNK_BUF;
			int posMessage = READ_UINT16_LE(srcPtr);
			switch(opcode) {
			case 0x1c:
				sprintf(writeBuf, "add_si ");
				break;
			case 0x1d:
				sprintf(writeBuf, "push_s ");
				break;
			}
			int j;
			for(j = 0; j < messageNum; j++) {
				if(posMessage == messagePosTable[j]) {
					break;
				}
			}
			//실제로 존재하는 Message 위치일때
			if(j != messageNum) {
				WRITE;
				sprintf(writeBuf, "%d", j);
				WRITE_LINE;
				srcPtr += 2;
			} else { //unk 0x1c / 0x1d 일때
				sprintf(writeBuf, "%2x", opcode);
				strcat(unkBuf, writeBuf);
				if(*srcPtr == 0x25) {
					FLUSH_UNK_BUF;
					sprintf(writeBuf, "func_e");
					WRITE_LINE;
				} else if(*srcPtr == 0x32) {
					FLUSH_UNK_BUF;
					sprintf(writeBuf, "?unc_e");
					WRITE_LINE;
				} else {
					sprintf(writeBuf, "%2x", *srcPtr);
					strcat(unkBuf, writeBuf);
				}
				srcPtr++;
			}
		} else if(opcode == 0x25) {
			FLUSH_UNK_BUF;
			sprintf(writeBuf, "func_e");
			WRITE_LINE;
		} else if(opcode == 0x32) {
			FLUSH_UNK_BUF;
			sprintf(writeBuf, "?unc_e");
			WRITE_LINE;
		} else {
			sprintf(writeBuf, "%2x", opcode);
			strcat(unkBuf, writeBuf);
			//printf("%2x", opcode);
		}
	}
	if(strlen(unkBuf)) {
		printf("에러? 남는다! \"%s\"\n", unkBuf);
	}
	FLUSH_UNK_BUF;
	delete []unkBuf;

	sprintf(fn, "SCRIPTS/USE_%04d.DEC", num);
	printf("%s 파일에 기록했습니다.\n", fn);
	file.close();
	return 1;
}
