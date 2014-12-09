/* FCE Ultra - NES/Famicom Emulator
 *
 * Copyright notice for this file:
 *  Copyright (C) 2002 Xodnizel
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef _WIN32
#include <direct.h>
#else
#include <unistd.h>
#endif

#include "fceu-types.h"
#include "file.h"
#include "fceu-endian.h"
#include "fceu-memory.h"
#include "driver.h"
#include "general.h"

#ifndef __GNUC__
 #define strcasecmp strcmp
#endif

static MEMWRAP *MakeMemWrap(void *tz, int type)
{
   MEMWRAP *tmp;

   if (!(tmp = (MEMWRAP*)FCEU_malloc(sizeof(MEMWRAP))))
      goto doret;
   tmp->location = 0;

   fseek((FILE*)tz, 0, SEEK_END);
   tmp->size = ftell((FILE*)tz);
   fseek((FILE*)tz, 0, SEEK_SET);
   if (!(tmp->data = (uint8*)FCEU_malloc(tmp->size)))
   {
      free(tmp);
      tmp = 0;
      goto doret;
   }
   fread(tmp->data, 1, tmp->size, (FILE*)tz);

doret:
   if (type == 0)
      fclose((FILE*)tz);
   return tmp;
}

FCEUFILE * FCEU_fopen(const char *path, const char *ipsfn, char *mode, char *ext)
{
   FCEUFILE *fceufp = (FCEUFILE*)malloc(sizeof(FCEUFILE));
   void *t = fopen(path, mode);

   if (!t)
   {
      free(fceufp);
      return 0;
   }

   fseek((FILE*)t, 0, SEEK_SET);
   fceufp->type = 0;
   fceufp->fp = MakeMemWrap(t, 0);
   return fceufp;
}

int FCEU_fclose(FCEUFILE *fp)
{
   if (fp->fp)
      free(fp->fp);
   fp->fp = NULL;
	free(fp);
	fp = 0;
	return 1;
}

uint64 FCEU_fread(void *ptr, size_t element_size, size_t nmemb, FCEUFILE *fp)
{
   uint32_t total = nmemb * element_size;

   if (fp->fp->location >= fp->fp->size)
      return 0;

   if((fp->fp->location + total) > fp->fp->size)
   {
      int64_t ak = fp->fp->size - fp->fp->location;

      memcpy((uint8_t*)ptr, fp->fp->data + fp->fp->location, ak);

      fp->fp->location = fp->fp->size;

      return (ak / element_size);
   }
   
   memcpy((uint8_t*)ptr, fp->fp->data + fp->fp->location, total);

   fp->fp->location += total;

   return nmemb;
}

int FCEU_fseek(FCEUFILE *fp, long offset, int whence)
{
	return fseek((FILE*)fp->fp, offset, whence);
   switch (whence)
   {
      case SEEK_SET:
         if (offset >= fp->fp->size)
            return -1;

         fp->fp->location = offset;
         break;
      case SEEK_CUR:
         if ((offset + fp->fp->location) > fp->fp->size)
            return -1;

         fp->fp->location += offset;
         break;
   }

   return 0;
}

static uint16 FCEU_de16lsb(const uint8 *morp)
{
   return (morp[0] | (morp[1] << 8));
}

int FCEU_read32le(uint32 *Bufo, FCEUFILE *fp)
{
   if ((fp->fp->location + 2) > fp->fp->size)
      return 0;

   *Bufo = FCEU_de16lsb(fp->fp->data + fp->fp->location);

   fp->fp->location += 2;

   return 1;
}

int FCEU_fgetc(FCEUFILE *fp)
{
   if (fp->fp->location < fp->fp->size)
      return fp->fp->data[fp->fp->location++];

   return EOF;
}

uint64 FCEU_ftell(FCEUFILE *fp)
{
   return fp->fp->location;
}

uint64 FCEU_fgetsize(FCEUFILE *fp)
{
   return fp->fp->size;
}
