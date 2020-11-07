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

int htoi(char *pt)
{
	int a = 0, b = 0;
	if(*pt >= '0' && *pt <= '9')
		a = (*pt - '0');
	if(*pt >= 'a' && *pt <= 'f')
		a = (*pt - 'a') + 10;

	if(*(pt + 1) >= '0' && *(pt + 1) <= '9')
		b = (*(pt + 1) - '0');
	if(*(pt + 1) >= 'a' && *(pt + 1) <= 'f')
		b = (*(pt + 1) - 'a') + 10;

	return a * 16 + b;
}

// 디컴파일된 스크립트를 컴파일한다.
bool UseCodeManager::exportFuncTextCompiledOldType(int num)
{
	File src, msgSrc, dst;

	char fn[256];

	char messageBuf[768][1024] = {0,};
	int messageOffset[768] = {0,};
	int messageNum = 0;
	int messageLen = 0;
	char writeBuf[16384] = {0,};
	char readBuf[16384] = {0,};
	int totalBytes = 0;
	char *pt = readBuf;
	bool msgFileExists = 1;

	sprintf(fn, "SCRIPTS/USE_%04d.DEC", num);
	if(!src.open(fn, workDir, File::kFileReadMode)) {
		printf("%s 파일을 읽을 수 없습니다!\n", fn);
		return 0;
	}

	sprintf(fn, "MESSAGES/USE_%04dK.MSG", num);
	if(!msgSrc.open(fn, workDir, File::kFileReadMode)) {
		sprintf(fn, "MESSAGES/USE_%04d.MSG", num);
		if(!msgSrc.open(fn, workDir, File::kFileReadMode)) {
			msgFileExists = 0;
		}
	}

	sprintf(fn, "COMPILED/USE_%04d.FNC", num);
	if(!dst.open(fn, workDir, File::kFileWriteMode)) {
		printf("%s 파일에 쓸 수 없습니다!\n", fn);
		return 0;
	}

	// 함수 정보를 읽는다.
	src.readLine(readBuf);
	pt = readBuf + 7;
	//	printf("function num: %x\n", atoi(pt));
	printf("message %d\r", num, atoi(pt));
	int msgNr = atoi(pt);

	src.readLine(readBuf);
	pt = readBuf + 7;
	//	printf("function num: %x\n", atoi(pt));
	printf("function %d - num: 0x%03X\r", num, atoi(pt));
	dst.writeUint16LE(atoi(pt));
	dst.writeUint16LE(0); //바이트는 나중에 넣어 준다.
	dst.writeUint16LE(0); //문자열 총 바이트도 나중에 넣어 준다.

	char *p;

	if(msgFileExists) {
		for(int i = 0; i < msgNr; i++) {
			readBuf[0] = readBuf[1] = 0;
			msgSrc.readLine(readBuf);
			if(readBuf[0] != '[' && readBuf[5] != ']') {
				i--;
				continue;
			}
			p = &readBuf[6];

			if(p[strlen(p) - 1] == 0x0a || p[strlen(p) - 1] == 0x0d)
				p[strlen(p) - 1] = 0;
			if(p[strlen(p) - 1] == 0x0a || p[strlen(p) - 1] == 0x0d)
				p[strlen(p) - 1] = 0;

			//printf("#%s#\n", p);
			for(int k = 0; k < strlen(p); k++) {
				if(p[k] == '\\' && p[k + 1] == 'n') {
					p[k] = 0xd;
					p[k + 1] = 0xa;
				}
			}
			dst.write(p, strlen(p));
			dst.writeByte(0);
			messageOffset[messageNum++] = messageLen;
			messageLen += strlen(p) + 1;
		}
	}
	if(msgNr != messageNum) {
		printf("에러: Message String의 개수가 맞지 않습니다! %d != %d\n", msgNr, messageNum);
		exit(1);
	}

	while(!src.eof()) {
		readBuf[0] = 0;
		src.readLine(readBuf);

		if(!strnicmp(readBuf, "unk_op", 6)) {					
			p = &readBuf[7];
	
			if(p[strlen(p) - 1] == 0x0a || p[strlen(p) - 1] == 0x0d)
				p[strlen(p) - 1] = 0;
			if(p[strlen(p) - 1] == 0x0a || p[strlen(p) - 1] == 0x0d)
				p[strlen(p) - 1] = 0;

			for(uint i = 0; i < strlen(p); p += 2) {
				dst.writeByte(htoi(p));
				totalBytes += 1;
			}
		}
		if(!strnicmp(readBuf, "push_s", 6)) {
			p = &readBuf[7];
			dst.writeByte(0x1d);
			dst.writeUint16LE(messageOffset[atoi(p)]);
			totalBytes += 3;
		}
		if(!strnicmp(readBuf, "add_si", 6)) {
			p = &readBuf[7];
			dst.writeByte(0x1c);
			dst.writeUint16LE(messageOffset[atoi(p)]);
			totalBytes += 3;
		}
		if(!strnicmp(readBuf, "func_e", 6)) {
			dst.writeByte(0x25);
			totalBytes++;
		}
		if(!strnicmp(readBuf, "?unc_e", 6)) {
			dst.writeByte(0x32);
			totalBytes++;
		}
	}
	dst.seek(2);
	dst.writeUint16LE(totalBytes + messageLen + 2);
	//dst.seek(4);
	dst.writeUint16LE(messageLen);

	printf("\n");

	printf("%d, %d, %d\n", totalBytes + messageLen, dst.size(), useTable[num].size);

	// 컴파일이 제대로 되는지 검사한다. 한글로 메시지 파일을 바꾸었을 경우 사용하지 않음
	if(totalBytes + messageLen + 2 != useTable[num].size) {
	//	printf("ERROR: 사이즈가 일치하지 않는다!\n");
	//	getch();
	//	exit(1);
	}
	if(dst.size() != useTable[num].size + 4) {
	//	printf("ERROR: 파일 사이즈가 일치하지 않는다!\n");
	//	getch();
	//	exit(1);
	}

	dst.close();
	src.close();

	return 1;
}
