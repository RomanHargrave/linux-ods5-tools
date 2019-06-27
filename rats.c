// vim: set et ts=3 sts=3 sw=3 :
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
         putchar('\n');
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

      // Print out friendly attributes, but only if printing all or no other options given
      if (doPrintAll == true || (doPrintRaw == false && doPrintFields == false)) {
         char stbuf[9];
         char* val_tmp;

         // File organization
         switch (fat.rtype.fileorg) {
            case FILE_ORG_SEQ:
               val_tmp = "seq";
               break;
            case FILE_ORG_REL:
               val_tmp = "rel";
               break;
            case FILE_ORG_IDX:
               val_tmp = "idx";
               break;
            case FILE_ORG_DIR:
               val_tmp = "dir";
               break;
            case FILE_ORG_SPECIAL:
               val_tmp = "special";
               break;
            default:
               snprintf(stbuf, 9, "0x%02x", fat.rtype.fileorg);
               val_tmp = stbuf;
         }
         printf("org=%s", val_tmp);

         // Record type
         switch (fat.rtype.rtype) {
            case RECORD_FORMAT_UDF:
               val_tmp = "udf";
               break;
            case RECORD_FORMAT_FIX:
               val_tmp = "fix";
               break;
            case RECORD_FORMAT_VAR:
               val_tmp = "var";
               break;
            case RECORD_FORMAT_VFC:
               val_tmp = "vfc";
               break;
            case RECORD_FORMAT_STM:
               val_tmp = "stm";
               break;
            case RECORD_FORMAT_STMLF:
               val_tmp = "stmlf";
               break;
            case RECORD_FORMAT_STMCR:
               val_tmp = "stmcr";
               break;
            default:
               snprintf(stbuf, 9, "0x%02x", fat.rtype.rtype);
               val_tmp = stbuf;
               break;
         }
         printf(" rfm=%s", val_tmp);

         // Handle special files, which use fat.rattrib to indicate type
         // or, print record attributes
         if (fat.rtype.fileorg == FILE_ORG_SPECIAL) {
            vms_byte const special_type = *((vms_byte*) &fat.rattrib);

            switch (special_type) {
               case SPECIAL_TYPE_NONE:
                  val_tmp = "none";
                  break;
               case SPECIAL_TYPE_FIFO:
                  val_tmp = "fifo";
                  break;
               case SPECIAL_TYPE_CHAR:
                  val_tmp = "char_special";
                  break;
               case SPECIAL_TYPE_BLOCK:
                  val_tmp = "block_special";
                  break;
               case SPECIAL_TYPE_SYMLINK:
                  val_tmp = "symlink";
                  break;
               case SPECIAL_TYPE_SYMBOLIC_LINK:
                  val_tmp = "symbolic_link";
                  break;
               default:
                  snprintf(stbuf, 9, "0x%02x", special_type);
                  val_tmp = stbuf;
                  break;
            }
            printf(" special_type=%s", val_tmp);
         } else {
            struct ods_fat_rattrib rat = fat.rattrib;
            vms_byte b_rat = *((vms_byte*) &rat);

            // if unused bits are set, this is probably not rattribs proper
            // XXX side note, a lot of this relies on the compiler aligning structures consistently
            // XXX maybe some manual aligning should be done.
            // XXX e.g. only five bits are defined in `struct ods_fat_rattrib` but it is expected to be
            // XXX eight bits. this will normally be true.
            if (rat.fill_0 != 0) {
               printf(" rat=0x%02x", b_rat);
            } else if (b_rat == 0) {
               printf(" rat=none");
            } else {
               if (rat.fortran_cc != 0) printf(" rat=ftn");
               if (rat.implied_cc != 0) printf(" rat=cr");
               if (rat.print_cc   != 0) printf(" rat=prn");
               if (rat.nospan     != 0) printf(" rat=blk");
               if (rat.msbrcw     != 0) printf(" rat=msb");
            }
         }

         // Print 'lrl' (rsize)
         printf(" lrl=%u", fat.rsize);

         // Print bucket and VFC size if not printing all items
         // otherwise, print the rest of the FAT information
         if (doPrintAll == false) {
            if (fat.rtype.fileorg == FILE_ORG_REL) {
               printf(" bks=%u", fat.bktsize);
            }

            if (fat.rtype.rtype == RECORD_FORMAT_VFC) {
               printf(" fsz=%u", fat.vfcsize);
            }
         } else {
            vms_long const hbk = (fat.hiblk.high * 0x10000) + fat.hiblk.low;
            vms_long const ebk = (fat.efblk.high * 0x10000) + fat.efblk.low;
            printf(" hbk=%u", hbk);
            printf(" ebk=%u", ebk);
            printf(" ffb=%u", fat.ffbyte);
            printf(" bks=%u", fat.bktsize);
            printf(" fsz=%u", fat.vfcsize);
            printf(" mrs=%u", fat.maxrec);
            printf(" deq=%u", fat.defext);
            printf(" gbc=%u", fat.gbc);
            printf(" gbx=%u", fat.gbc32);
            printf(" vrs=%u", fat.versions);
         }

         putchar('\n');
      }
   }
}
