/**
 * ODS/5 xattr interface
 *
 * (C) 2019 Roman Hargrave <roman@hargrave.info>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301  USA
 */

#include <sys/types.h>
#include <sys/xattr.h>
#include <string.h>
#include <errno.h>

#include "./ods_type.h"
#include "./ods_xattr.h"

/**
 * Get FAT data for given filename.
 * Returns 0 on success, error code on failure.
 */
int get_fat(struct ods_fat* fat, char* const fname)
{
   memset(fat, 0, sizeof(*fat));

   ssize_t read_sz = getxattr(fname, ODS_XATTR_FAT, fat, sizeof(*fat));

   if (read_sz < 0) {
      return errno;
   } else {
      return 0;
   }
}
