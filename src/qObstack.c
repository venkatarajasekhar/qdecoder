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
 * @file qObstack.c qDecoder implementation of GNU obstack
 *
 * @code
 *   [Code sample - String]
 *   Q_OBSTACK *obstack;
 *
 *   char *final;
 *   char *tmp = "CDE";
 *
 *   // get new obstack
 *   obstack = qObstackInit();
 *
 *   // stack
 *   qObstackGrowStr(obstack, "AB");		// no need to supply size
 *   qObstackGrowStrf(obstack, "%s", tmp);	// for formatted string
 *   qObstackGrow(obstack, "FGH", 3);	// same effects as above but this can
 *   					// be used for object or binary
 *
 *   // final
 *   final = (char *)qObstackFinish(obstack);
 *
 *
 *   // print out
 *   printf("Final string = %s\n", final);
 *   printf("Total Size = %d, Number of Objects = %d\n", qObstackGetSize(obstack), qObstackGetNum(obstack));
 *
 *   qObstackFree(obstack);
 *
 *   [Result]
 *   Final string = ABCDEFGH
 *   Total Size = 8, Number of Objects = 3
 * @endcode
 *
 * @code
 *   [Code sample - Object]
 *   struct sampleobj {
 *   	int	num;
 *   	char	str[10];
 *   };
 *
 *   Q_OBSTACK *obstack;
 *   int i;
 *
 *   // sample object
 *   struct sampleobj obj;
 *   struct sampleobj *final;
 *
 *   // get new obstack
 *   obstack = qObstackInit();
 *
 *   // stack
 *   for(i = 0; i < 3; i++) {
 *   	// filling object with sample data
 *   	obj.num  = i;
 *   	sprintf(obj.str, "hello%d", i);
 *
 *   	// stack
 *   	qObstackGrow(obstack, (void *)&obj, sizeof(struct sampleobj));
 *   }
 *
 *   // final
 *   final = (struct sampleobj *)qObstackFinish(obstack);
 *
 *   // print out
 *   qContentType("text/plain");
 *   for(i = 0; i < qObstackGetNum(obstack); i++) {
 *   	printf("Object%d final = %d, %s\n", i+1, final[i].num, final[i].str);
 *   }
 *   printf("Total Size = %d, Number of Objects = %d\n", qObstackGetSize(obstack), qObstackGetNum(obstack));
 *
 *   qObstackFree(obstack);
 *
 *   [Result]
 *   Object1 final = 0, hello0
 *   Object2 final = 1, hello1
 *   Object3 final = 2, hello2
 *   Total Size = 48, Number of Objects = 3
 * @endcode
 */

#ifndef DISABLE_DATASTRUCTURE

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdarg.h>
#include <string.h>
#include "qDecoder.h"

/**
 * Under-development
 *
 * @since not released yet
 */
Q_OBSTACK *qObstackInit(void) {
	Q_OBSTACK *obstack;

	obstack = (Q_OBSTACK *)malloc(sizeof(Q_OBSTACK));
	if(obstack == NULL) return NULL;

	memset((void *)obstack, 0, sizeof(Q_OBSTACK));
	obstack->stack = qEntryInit();
	if(obstack->stack == NULL) {
		free(obstack);
		return NULL;
	}

	return obstack;
}

/**
 * Under-development
 *
 * @since not released yet
 */
bool qObstackGrow(Q_OBSTACK *obstack, const void *object, size_t size) {
	if(obstack == NULL || object == NULL || size <= 0) return false;
	return qEntryPut(obstack->stack, "", object, size, false);
}

/**
 * Under-development
 *
 * @since not released yet
 */
bool qObstackGrowStr(Q_OBSTACK *obstack, const char *str) {
	return qObstackGrow(obstack, (void *)str, strlen(str));
}

/**
 * Under-development
 *
 * @since not released yet
 */
bool qObstackGrowStrf(Q_OBSTACK *obstack, const char *format, ...) {
	if(obstack == NULL) return false;

	char str[1024];
	va_list arglist;

	va_start(arglist, format);
	vsnprintf(str, sizeof(str), format, arglist);
	va_end(arglist);

	return qObstackGrow(obstack, (void *)str, strlen(str));
}

/**
 * Under-development
 *
 * @since not released yet
 */
void *qObstackFinish(Q_OBSTACK *obstack) {
	if(obstack == NULL) return NULL;

	void *dp;
	if(obstack->final != NULL) free(obstack->final);
	obstack->final = dp = (void *)malloc(obstack->stack->size + 1);
	if(obstack->final == NULL) return NULL;

	const Q_NLOBJ *obj;
	for(obj = qEntryFirst(obstack->stack); obj; obj = qEntryNext(obstack->stack)) {
		memcpy(dp, obj->object, obj->size);
		dp += obj->size;
	}
	*((char *)dp) = '\0';

	return obstack->final;
}

/**
 * Under-development
 *
 * @since not released yet
 */
void *qObstackGetFinal(Q_OBSTACK *obstack) {
	if(obstack == NULL) return NULL;
	if(obstack->final == NULL) qObstackFinish(obstack);
	return obstack->final;
}

/**
 * Under-development
 *
 * @since not released yet
 */
int qObstackGetSize(Q_OBSTACK *obstack) {
	if(obstack == NULL) return 0;
	return obstack->stack->size;
}

/**
 * Under-development
 *
 * @since not released yet
 */
int qObstackGetNum(Q_OBSTACK *obstack) {
	if(obstack == NULL) return 0;
	return obstack->stack->num;
}

/**
 * Under-development
 *
 * @since not released yet
 */
bool qObstackFree(Q_OBSTACK *obstack) {
	if(obstack == NULL) return false;
	qEntryFree(obstack->stack);
	if(obstack->final != NULL) free(obstack->final);
	free(obstack);
	return true;
}

#endif /* DISABLE_DATASTRUCTURE */
