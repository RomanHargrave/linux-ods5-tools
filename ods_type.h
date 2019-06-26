// vim: set et ts=3 sts=3 sw=3 :
/**
 * ODS/5 & ODS/2 constants
 *
 * (C) 2019 Roman Hargrave <roman@hargrave.info>
 * (C) 2010 Hartmut Becker
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

#include <stdint.h>

#if !defined(_ODS_TYPE_H)
#define _ODS_TYPE_H

typedef uint8_t   vms_byte;
typedef uint16_t  vms_word;
typedef uint32_t  vms_long;
typedef uint64_t  vms_quad;

enum ods_file_org {
   FILE_ORG_SEQ      = 0,
   FILE_ORG_REL      = 1,
   FILE_ORG_IDX      = 2,
   FILE_ORG_DIR      = 3,
   FILE_ORG_SPECIAL  = 4
};

enum ods_record_format {
   RECORD_FORMAT_UDF    = 0,
   RECORD_FORMAT_FIX    = 1,
   RECORD_FORMAT_VAR    = 2,
   RECORD_FORMAT_VFC    = 3,
   RECORD_FORMAT_STM    = 4,
   RECORD_FORMAT_STMLF  = 5,
   RECORD_FORMAT_STMCR  = 6
};

enum ods_special_type {
   SPECIAL_TYPE_NONE          = 0,
   SPECIAL_TYPE_FIFO          = 1,
   SPECIAL_TYPE_CHAR          = 2,
   SPECIAL_TYPE_BLOCK         = 3,
   SPECIAL_TYPE_SYMLINK       = 4,
   SPECIAL_TYPE_SYMBOLIC_LINK = 5
};

struct ods_fat_rtype {
   vms_byte rtype     : 4; // Record type
   vms_byte fileorg   : 4; // File organization
};

struct ods_fat_rattrib {
   vms_byte fortran_cc : 1; // FORTRAN carriage control
   vms_byte implied_cc : 1; // implied carriage control
   vms_byte print_cc   : 1; // print file carriage control
   vms_byte nospan     : 1; // No spanned records
   vms_byte msbrcw     : 1; // Format of RCW (0=LSB, 1=MSB)
};

struct ods_fat_block {
   vms_word high;
   vms_word low;
};

struct ods_fat {
   struct ods_fat_rtype    rtype;         // Record type
   struct ods_fat_rattrib  rattrib;       // Record attributes
   vms_word                rsize;         // Record size, in bytes
   struct ods_fat_block    hiblk;         // Highest allocated VBN
   struct ods_fat_block    efblk;         // End-of-file VBN
   vms_word                ffbyte;        // First free byte in efblk
   vms_byte                bktsize;       // Bucket size, in blocks
   vms_byte                vfcsize;       // Size, in bytes, of fixed-length control for VFC records
   vms_word                maxrec;        // Maximum record size, in bytes
   vms_word                defext;        // Default extend quantity
   vms_word                gbc;           // Global buffer count (original word)
   vms_byte                recattr_flags; // Flags for record attribute area
   vms_byte                fill_0;        // Unused/unknown space
   vms_long                gbc32;         // Longword global buffer count
   vms_word                fill_1;        // Spare space, as documented in VMS I/O REF
   vms_word                versions;      // Default version limit for directory file
};

#endif

