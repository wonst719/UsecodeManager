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

#ifndef __USECODE_H
#define __USECODE_H

struct STable {
	uint16 id;
	uint32 pos;
	uint32 size;
	byte *data;
};

class UseCodeManager {
	STable useTable[2000];
	int numFunc;
	char workDir[512];

public:
	UseCodeManager();

	bool readUsecode();
	bool writeUsecode();

	bool setWorkDir(const char *dir);

	int getFuncNum() { return numFunc; }

	int getFuncPos(int num);

	STable *getTable(int num) { return &useTable[num]; }

	bool importFuncBin(int num);
	bool exportFuncBin(int num);

	bool exportFuncTextOldType(int num);
	bool exportFuncTextCompiledOldType(int num);
};

#endif
