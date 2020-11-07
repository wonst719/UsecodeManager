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

UseCodeManager::UseCodeManager()
{
	numFunc = 0;
	memset(workDir, 0, 512);
	strcpy(workDir, "./");
}

bool UseCodeManager::readUsecode()
{
	File file;
	if(!file.open("USECODE", workDir, File::kFileReadMode)) {
		printf("USECODE 파일을 열 수 없습니다!\n");
		return 0;
	}
	do {
		STable *t = &useTable[numFunc];
		t->pos = file.pos();
		t->id = file.readUint16LE();
		t->size = file.readUint16LE();

//		printf("Function %5d - #%04X, %5d(%04Xh)\n", numFunc, t->id, t->pos, t->pos);
//		printf(" - size: %5d (%04X)\n", t->size, t->size);

		t->data = new byte[t->size];
		file.read(t->data, t->size);

		numFunc++;
	} while(file.pos() < file.size());
	file.close();
	printf("USECODE 파일을 로드했습니다.\n");
	return 1;
}

bool UseCodeManager::writeUsecode()
{
	File file;
	if(!file.open("USECODE_K", workDir, File::kFileWriteMode)) {
		printf("USECODE_K 파일에 쓸 수 없습니다!\n");
		return 0;
	}
	for(int i = 0; i < numFunc; i++) {
		STable *t = &useTable[i];
		file.writeUint16LE(t->id);
		file.writeUint16LE(t->size);
		file.write(t->data, t->size);
	}
	file.close();
	printf("USECODE_K 파일에 기록했습니다.\n");
	return 1;
}

bool UseCodeManager::exportFuncBin(int num)
{
	File file;
	char fn[256];
	sprintf(fn, "COMPILED/USE_%04d.FNC", num);
	if(!file.open(fn, workDir, File::kFileWriteMode)) {
		printf("%s 파일에 쓸 수 없습니다!\n", fn);
		return 0;
	}
	STable *t = &useTable[num];
	file.writeUint16LE(t->id);
	file.writeUint16LE(t->size);
	file.write(t->data, t->size);
	printf("%s 파일에 기록했습니다.\n", fn);
	file.close();
	return 1;
}

int UseCodeManager::getFuncPos(int num)
{
	STable *t = &useTable[num];
	int pos = 0;
	for(int i = 0; i < num; i++) {
		pos += useTable[i].size + 4;
	}
	return pos;
}

bool UseCodeManager::importFuncBin(int num)
{
	File file;
	char fn[256];
	sprintf(fn, "COMPILED/USE_%04d.FNC", num);
	if(!file.open(fn, workDir, File::kFileReadMode)) {
		printf("%s 파일을 읽을 수 없습니다!\n", fn);
		return 0;
	}
	STable *t = &useTable[num];
	t->pos = getFuncPos(num);
	t->id = file.readUint16LE();
	t->size = file.readUint16LE();

	printf("Function %5d - #%04X, %5d(%04Xh)\n", numFunc, t->id, t->pos, t->pos);
	printf(" - size: %5d (%04X)\n", t->size, t->size);
	delete []t->data;

	t->data = new byte[t->size];
	file.read(t->data, t->size);
	printf("%s 파일에서 읽었습니다.\n", fn);
	return 1;
}

bool UseCodeManager::setWorkDir(const char *dir)
{
	strcpy(workDir, dir);
	return 1;
}