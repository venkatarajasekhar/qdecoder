/**************************************************************************
 * qDecoder - Web Application Interface for C/C++   http://www.qDecoder.org
 *
 * Copyright (C) 2007 Seung-young Kim.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published
 * by the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *************************************************************************/

/**
 * @file qFile.c File Handling API
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/file.h>
#include "qDecoder.h"
#include "qInternal.h"

/**
 * Lock opened file.
 *
 * @param fd		file descriptor
 *
 * @return		true if successful, otherwise returns false.
 *
 * @code
 *   // for file descriptor
 *   int fd = open(...);
 *   if(qFileLock(fd) == true) {
 *     (...atomic file access...)
 *     qFileUnlock(fd);
 *   }
 *
 *   // for FILE stream object
 *   FILE *fp = fopen(...);
 *   int fd = fileno(fp);
 *   if(qFileLock(fd) == true) {
 *     (...atomic file access...)
 *     qFileUnlock(fd);
 *   }
 * @endcode
 */
bool qFileLock(int fd) {
#ifdef _WIN32
	return false;
#else
	int ret = flock(fd, LOCK_EX);
	if(ret == 0) return true;
	return false;
#endif
}

/**
 * Unlock opened file.
 *
 * @param fd		file descriptor
 *
 * @return		true if successful, otherwise returns false.
 */
bool qFileUnlock(int fd) {
#ifdef _WIN32
	return false;
#else
	int ret = flock(fd, LOCK_EX);
	if(ret == 0) return true;
	return false;
#endif
}

/**
 * Check file existence.
 *
 * @param filepath	file or directory path
 *
 * @return		true if exists, otherwise returns false;
 */
bool qFileExist(const char *filepath) {
	struct stat finfo;
	if (stat(filepath, &finfo) < 0) return false;
	return true;
}

/**
 * Get file size.
 *
 * @param filepath	file or directory path
 *
 * @return		the file size if exists, otherwise returns -1.
 */
off_t qFileGetSize(const char *filepath) {
	struct stat finfo;
	if (stat(filepath, &finfo) < 0) return -1;
	return finfo.st_size;
}

/**
 * Transfer data between file descriptors
 *
 * @param outfd		output file descriptor
 * @param infd		input file descriptor
 * @param size		the number of bytes to copy between file descriptors. 0 means transfer until end of infd.
 *
 * @return		the number of bytes written to outfd.
 */
size_t qFileSend(int outfd, int infd, size_t size) {
#define _Q_FILESEND_CHUNK_SIZE		(256 * 1024)
	char buf[_Q_FILESEND_CHUNK_SIZE];

	size_t sent = 0; // total size sent
	while(size == 0 || sent < size) {
		size_t sendsize;	// this time sending size
		if(size - sent <= sizeof(buf)) sendsize = size - sent;
		else sendsize = sizeof(buf);

		// read
		ssize_t retr = read(infd, buf, sendsize);
		if (retr <= 0) break;

		// write
		ssize_t retw = write(outfd, buf, retr);
		if(retw <= 0) break;

		sent += retw;
		if(retr != retw) break;
	}

	return sent;
}

/**********************************************
** Usage : qReadFile(filepath, integer pointer to store file size);
** Return: Success stream pointer, Fail NULL.
** Do    : Read file to malloced memory.
**********************************************/
char *qFileLoad(const char *filepath, int *size) {
	FILE *fp;
	struct stat filestat;
	char *sp, *tmp;
	int c, i;

	if (stat(filepath, &filestat) < 0) return NULL;
	if ((fp = fopen(filepath, "r")) == NULL) return NULL;

	sp = (char *)malloc(filestat.st_size + 1);
	for (tmp = sp, i = 0; (c = fgetc(fp)) != EOF; tmp++, i++) *tmp = (char)c;
	*tmp = '\0';

	if (filestat.st_size != i) {
		DEBUG("Size(File:%d, Readed:%d) mismatch.", (int)filestat.st_size, i);
		free(sp);
		return NULL;
	}
	fclose(fp);
	if (size != NULL) *size = i;
	return sp;
}

/**********************************************
** Usage : qSaveStr(string pointer, string size, filepath, mode)
** Return: Success number bytes stored, File open fail -1.
** Do    : Store string to file.
**********************************************/
int qSaveStr(char *sp, int spsize, char *filepath, char *mode) {
	FILE *fp;
	int i;
	if ((fp = fopen(filepath, mode)) == NULL) return -1;
	for (i = 0; i < spsize; i++) fputc(*sp++, fp);
	fclose(fp);

	return i;
}


/*********************************************
** Usage : qfReadFile(file pointer);
** Return: Success string pointer, End of file NULL.
** Do    : Read text stream.
**********************************************/
char *qfReadFile(FILE *fp) {
	int memsize;
	int c, c_count;
	char *string = NULL;

	for (memsize = 1024, c_count = 0; (c = fgetc(fp)) != EOF;) {
		if (c_count == 0) {
			string = (char *)malloc(sizeof(char) * memsize);
			if (string == NULL) {
				DEBUG("Memory allocation failed.");
				return NULL;
			}
		} else if (c_count == memsize - 1) {
			char *stringtmp;

			memsize *= 2;

			/* Here, we do not use realloc(). Because sometimes it is unstable. */
			stringtmp = (char *)malloc(sizeof(char) * (memsize + 1));
			if (stringtmp == NULL) {
				DEBUG("Memory allocation failed.");
				free(string);
				return NULL;
			}
			memcpy(stringtmp, string, c_count);
			free(string);
			string = stringtmp;
		}
		string[c_count++] = (char)c;
	}

	if (c_count == 0 && c == EOF) return NULL;
	string[c_count] = '\0';

	return string;
}

/*********************************************
** Usage : qfGetLine(file pointer);
** Return: Success string pointer, End of file NULL.
** Do    : Read one line from file pointer without length
**         limitation. String will be saved into dynamically
**         allocated memory. The newline, if any, is retained.
**********************************************/
char *qfGetLine(FILE *fp) {
	int memsize;
	int c, c_count;
	char *string = NULL;

	for (memsize = 1024, c_count = 0; (c = fgetc(fp)) != EOF;) {
		if (c_count == 0) {
			string = (char *)malloc(sizeof(char) * memsize);
			if (string == NULL) {
				DEBUG("Memory allocation failed.");
				return NULL;
			}
		} else if (c_count == memsize - 1) {
			char *stringtmp;

			memsize *= 2;

			/* Here, we do not use realloc(). Because sometimes it is unstable. */
			stringtmp = (char *)malloc(sizeof(char) * (memsize + 1));
			if (stringtmp == NULL) {
				DEBUG("Memory allocation failed.");
				free(string);
				return NULL;
			}
			memcpy(stringtmp, string, c_count);
			free(string);
			string = stringtmp;
		}
		string[c_count++] = (char)c;
		if ((char)c == '\n') break;
	}

	if (c_count == 0 && c == EOF) return NULL;
	string[c_count] = '\0';

	return string;
}

/**********************************************
** Usage : qCmd(external command);
** Return: Execution output, File not found NULL.
**********************************************/
char *qCmd(char *cmd) {
	FILE *fp;
	char *str;

	fp = popen(cmd, "r");
	if (fp == NULL) return NULL;
	str = qfReadFile(fp);
	pclose(fp);

	if(str == NULL) str = strdup("");
	return str;
}
