/**
 * An open-source implementation of the `rats` tool for the ODS/5
 * filesystem driver, intended to replicate the functionality in the
 * tool distributed by Hartmut Becker.
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

#include <unistd.h>
#include <stdio.h>
#include <stdbool.h>
#include <errno.h>
#include <string.h>
#include <libgen.h>

#include "./ods_type.h"
#include "./ods_xattr.h"

static int print_help(char* const p_name)
{
   fprintf(stderr, "\
%s [-abf] file... - inspect ODS file attributes\n\
Options:\n\
   -a    format all attributes\n\
   -b    print data as byte stream\n\
   -f    print field names with their data\n", p_name);
   return 0;
}

int main(int argc, char** argv)
{
   bool doPrintAll      = false;
   bool doPrintRaw      = false;
   bool doPrintFields   = false;

   if (argc > 1) {
      int flag = 0;
      while (flag = getopt(argc, argv, "abf"), flag != -1) {
         switch (flag) {
            case 'a':
               doPrintAll = true;
               break;
            case 'b':
               doPrintRaw = true;
               break;
            case 'f':
               doPrintFields = true;
               break;
            default:
               print_help(argv[0]);
               return EINVAL;
         }
      }
   } else {
      return print_help(argv[0]);
   }

   if (optind >= argc) {
      fprintf(stderr, "No filenames given\n");
      return EINVAL;
   };

   char* fname;
   struct ods_fat fat;
   for (size_t idx = optind; idx < argc; ++idx) {
      fname = argv[idx];

      int fat_err = get_fat(&fat, fname);

      if (fat_err == EOPNOTSUPP) {
         fprintf(stderr, "%s: not an ODS5 file\n", fname);
         continue;
      } else if (fat_err != 0) {
         fprintf(stderr, "%s: could not get FAT: %s\n", fname, strerror(fat_err));
         continue;
      }

      // print filename preamble (basename is used b/c the original `rats` did the same)
      printf("%s:\n", basename(fname));

      // print byte stream if requested
      if (doPrintRaw == true) {
         uint8_t* part = (uint8_t*) &fat;
         for(ssize_t len = 0; len < sizeof(fat); ++len, ++part) {
            printf("%02x", *part);
         }
         printf("\n");
      }

      // print fields, if requested
      if (doPrintFields == true) {
         printf("rtype: 0x%x\n",                   fat.rtype.rtype);
         printf("fileorg: 0x%x\n",                 fat.rtype.fileorg);
         printf("rattrib: 0x%02x\n",               *((uint8_t*) &fat.rattrib));
         printf("rsize: 0x%04x\n",                 fat.rsize);
         printf("hiblk(h,l): (0x%04x,0x%04x)\n",   fat.hiblk.high, fat.hiblk.low);
         printf("efblk(h,l): (0x%04x,0x%04x)\n",   fat.efblk.high, fat.efblk.low);
         printf("ffbyte: 0x%04x\n",                fat.ffbyte);
         printf("bktsize: 0x%02x\n",               fat.bktsize);
         printf("vfcsize: 0x%02x\n",               fat.vfcsize);
         printf("maxrec: 0x%04x\n",                fat.maxrec);
         printf("defext: 0x%04x\n",                fat.defext);
         printf("gbc: 0x%04x\n",                   fat.gbc);
         printf("recattr_flags: 0x%02x\n",         fat.recattr_flags);
         printf("gbc32: 0x%08x\n",                 fat.gbc32);
         printf("versions: 0x%04x\n",              fat.versions);
      }
   }
}
