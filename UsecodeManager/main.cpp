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

#include <iostream>
#include "file.h"
#include "usecode.h"


void main(int argc, char **argv)
{
	UseCodeManager man;
	man.setWorkDir("work/");

	if(argc < 2) {
		printf("USECODE Manager v2.1\n - USAGE: %s option\n", argv[0]);
		return;
	}

	if(!man.readUsecode())
		return;

	for(int q = 1; q < argc; q++) {
		if(!strcmp(argv[q], "export")) {
			for(int i = 0; i < man.getFuncNum(); i++) {
				man.exportFuncTextOldType(i);
			}
			break;
		}
		if(!strcmp(argv[q], "import")) {
			int i;

			for(i = 0; i < man.getFuncNum(); i++) {
				man.exportFuncTextCompiledOldType(i);
			}

			for(i = 0; i < man.getFuncNum(); i++) {
				int tmp_size = man.getTable(i)->size;
				man.importFuncBin(i);
				if(tmp_size != man.getTable(i)->size) {
				//	printf("ERROR: 사이즈가 다릅니다 (%d, %d)\n", tmp_size, man.getTable(i)->size);
				//	getch();
				//	exit(1);
				}
			}

			man.writeUsecode();
			break;
		}
	}
	//getch();
}
