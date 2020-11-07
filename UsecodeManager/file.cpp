/* ScummVM - Scumm Interpreter
 * Copyright (C) 2002-2003 The ScummVM project
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
#include <stdarg.h>

#define NORETURN _declspec(noreturn)

void NORETURN error(const char *s, ...) {
	va_list va;
	va_start(va, s);
	vprintf(s, va);
	va_end(va);
}

void warning(const char *s, ...) {
	va_list va;
	va_start(va, s);
	vprintf(s, va);
	va_end(va);
}

void debug(int level, const char *s, ...) {
	va_list va;
	va_start(va, s);
	vprintf(s, va);
	va_end(va);
}

FILE *File::fopenNoCase(const char *filename, const char *directory, const char *mode) {
	FILE *file;
	char buf[256];
	char *ptr;

	strcpy(buf, directory);

	// Record the length of the dir name (so we can cut of anything trailing it
	// later, when we try with different file names).
	const int dirLen = strlen(buf);

	if (dirLen > 0) {
		strcat(buf, "/");	// prevent double /
	}
	strcat(buf, filename);

	file = fopen(buf, mode);
	if (file)
		return file;

	return NULL;
}

File::File() {
	_handle = NULL;
	_ioFailed = false;
}

File::~File() {
	close();
}

bool File::open(const char *filename, const char *directory, int mode) {
	if (_handle) {
		debug(2, "File %s already opened", filename);
		return false;
	}

	if (filename == NULL || *filename == 0)
		return false;

	clearIOFailed();

	if (mode == kFileReadMode) {
		_handle = fopenNoCase(filename, directory, "rb");
		if (_handle == NULL) {
			debug(2, "File %s not found", filename);
			return false;
		}
	}
	else if (mode == kFileWriteMode) {
		_handle = fopenNoCase(filename, directory, "wb");
		if (_handle == NULL) {
			debug(2, "File %s not opened", filename);
			return false;
		}
	} else {
		warning("Only read/write mode supported!");
		return false;
	}

	return true;
}

void File::close() {
	if (_handle)
		fclose(_handle);
	_handle = NULL;
}

bool File::isOpen() {
	return _handle != NULL;
}

bool File::ioFailed() {
	return _ioFailed != 0;
}

void File::clearIOFailed() {
	_ioFailed = false;
}

bool File::eof() {
	if (_handle == NULL) {
		error("File is not open!");
		return false;
	}

	return feof(_handle) != 0;
}

uint32 File::pos() {
	if (_handle == NULL) {
		error("File is not open!");
		return 0;
	}

	return ftell(_handle);
}

uint32 File::size() {
	if (_handle == NULL) {
		error("File is not open!");
		return 0;
	}

	uint32 oldPos = ftell(_handle);
	fseek(_handle, 0, SEEK_END);
	uint32 length = ftell(_handle);
	fseek(_handle, oldPos, SEEK_SET);

	return length;
}

void File::seek(int32 offs, int whence) {
	if (_handle == NULL) {
		error("File is not open!");
		return;
	}

	if (fseek(_handle, offs, whence) != 0)
		clearerr(_handle);
}

uint32 File::readLine(char *ptr) {
	if (_handle == NULL) {
		error("File is not open!");
		return 0;
	}
	fgets(ptr, 16383, _handle);
	return strlen(ptr);
}

uint32 File::read(void *ptr, uint32 len) {
	byte *ptr2 = (byte *)ptr;
	uint32 real_len;

	if (_handle == NULL) {
		error("File is not open!");
		return 0;
	}

	if (len == 0)
		return 0;

	real_len = fread(ptr2, 1, len, _handle);
	if (real_len < len) {
		clearerr(_handle);
		_ioFailed = true;
	}

	return real_len;
}

byte File::readByte() {
	byte b;

	if (_handle == NULL) {
		error("File is not open!");
		return 0;
	}

	if (fread(&b, 1, 1, _handle) != 1) {
		clearerr(_handle);
		_ioFailed = true;
	}
	return b;
}

uint16 File::readUint16LE() {
	uint16 a = readByte();
	uint16 b = readByte();
	return a | (b << 8);
}

uint32 File::readUint32LE() {
	uint32 a = readUint16LE();
	uint32 b = readUint16LE();
	return (b << 16) | a;
}

uint16 File::readUint16BE() {
	uint16 b = readByte();
	uint16 a = readByte();
	return a | (b << 8);
}

uint32 File::readUint32BE() {
	uint32 b = readUint16BE();
	uint32 a = readUint16BE();
	return (b << 16) | a;
}

uint32 File::write(const void *ptr, uint32 len) {
	if (_handle == NULL) {
		error("File is not open!");
		return 0;
	}

	if (len == 0)
		return 0;

	if ((uint32)fwrite(ptr, 1, len, _handle) != len) {
		clearerr(_handle);
		_ioFailed = true;
	}

	return len;
}

void File::writeByte(byte value) {
	if (_handle == NULL) {
		error("File is not open!");
	}

	if (fwrite(&value, 1, 1, _handle) != 1) {
		clearerr(_handle);
		_ioFailed = true;
	}
}

void File::writeUint16LE(uint16 value) {
	writeByte((byte)(value & 0xff));
	writeByte((byte)(value >> 8));
}

void File::writeUint32LE(uint32 value) {
	writeUint16LE((uint16)(value & 0xffff));
	writeUint16LE((uint16)(value >> 16));
}

void File::writeUint16BE(uint16 value) {
	writeByte((byte)(value >> 8));
	writeByte((byte)(value & 0xff));
}

void File::writeUint32BE(uint32 value) {
	writeUint16BE((uint16)(value >> 16));
	writeUint16BE((uint16)(value & 0xffff));
}
