/* tc-mips.c -- header file for tc-mips.c.
   Copyright (C) 1993 Free Software Foundation, Inc.
   Contributed by the OSF and Ralph Campbell.
   Written by Keith Knowles and Ralph Campbell, working independently.
   Modified for ECOFF support by Ian Lance Taylor of Cygnus Support.

   This file is part of GAS.

   GAS is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   GAS is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with GAS; see the file COPYING.  If not, write to
   the Free Software Foundation, 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.  */

#ifndef TC_MIPS

#define TC_MIPS

#define TARGET_ARCH bfd_arch_mips

#define ONLY_STANDARD_ESCAPES
#define WORKING_DOT_WORD	1
#define OLD_FLOAT_READS
#define REPEAT_CONS_EXPRESSIONS
#define RELOC_EXPANSION_POSSIBLE
#define MAX_RELOC_EXPANSION 3
#define LOCAL_LABELS_FB 1

/* Maximum symbol offset that can be encoded in a BFD_RELOC_MIPS_GPREL
   relocation: */
#define MAX_GPREL_OFFSET (0x7FF4)

#define LOCAL_LABEL(name) mips_local_label (name)
extern int mips_local_label PARAMS ((const char *));

#define md_relax_frag(fragp, stretch)	(0)
#define md_undefined_symbol(name)	(0)
#define md_operand(x)

/* We permit PC relative difference expressions when generating
   embedded PIC code.  */
#define DIFF_EXPR_OK

#define LITTLE_ENDIAN   1234
#define BIG_ENDIAN      4321

/* Default to big endian.  */
#ifndef TARGET_BYTES_LITTLE_ENDIAN
#undef  TARGET_BYTES_BIG_ENDIAN
#define TARGET_BYTES_BIG_ENDIAN		1
#endif

/* The endianness of the target format may change based on command
   line arguments.  */
#define TARGET_FORMAT mips_target_format()
extern const char *mips_target_format ();

struct mips_cl_insn {
    unsigned long		insn_opcode;
    const struct mips_opcode	*insn_mo;
};

extern int tc_get_register PARAMS ((int frame));

#define md_parse_long_option(arg) mips_parse_long_option (arg)
extern int mips_parse_long_option PARAMS ((const char *));

#define tc_frob_label(sym) mips_define_label (sym)
extern void mips_define_label PARAMS ((struct symbol *));

#define tc_frob_file() mips_frob_file ()
extern void mips_frob_file PARAMS ((void));

#define TC_CONS_FIX_NEW cons_fix_new_mips
extern void cons_fix_new_mips ();

/* When generating embedded PIC code we must keep PC relative
   relocations.  */
#define TC_FORCE_RELOCATION(fixp) mips_force_relocation (fixp)
extern int mips_force_relocation ();

/* md_apply_fix sets fx_done correctly.  */
#define TC_HANDLE_FX_DONE 1

/* Register mask variables.  These are set by the MIPS assembly code
   and used by ECOFF and possibly other object file formats.  */
extern unsigned long mips_gprmask;
extern unsigned long mips_cprmask[4];

#ifdef OBJ_ELF

#define elf_tc_final_processing mips_elf_final_processing
extern void mips_elf_final_processing PARAMS ((void));

#define ELF_TC_SPECIAL_SECTIONS \
  { ".sdata",	SHT_PROGBITS,	SHF_ALLOC + SHF_WRITE + SHF_MIPS_GPREL	}, \
  { ".sbss",	SHT_NOBITS,	SHF_ALLOC + SHF_WRITE + SHF_MIPS_GPREL	}, \
  { ".lit4",	SHT_PROGBITS,	SHF_ALLOC + SHF_WRITE + SHF_MIPS_GPREL	}, \
  { ".lit8",	SHT_PROGBITS,	SHF_ALLOC + SHF_WRITE + SHF_MIPS_GPREL	}, \
  { ".ucode",	SHT_MIPS_UCODE,	0					}, \
  { ".mdebug",	SHT_MIPS_DEBUG,	0					},
/* Other special sections not generated by the assembler: .reginfo,
   .liblist, .conflict, .gptab, .got, .dynamic, .rel.dyn.  */

#endif

extern void md_mips_end PARAMS ((void));
#define md_end()	md_mips_end()

#define USE_GLOBAL_POINTER_OPT	(OUTPUT_FLAVOR == bfd_target_ecoff_flavour \
				 || OUTPUT_FLAVOR == bfd_target_elf_flavour)

extern void mips_pop_insert PARAMS ((void));
#define md_pop_insert()		mips_pop_insert()

extern void mips_flush_pending_output PARAMS ((void));
#define md_flush_pending_output mips_flush_pending_output

extern void mips_enable_auto_align PARAMS ((void));
#define md_elf_section_change_hook()	mips_enable_auto_align()

extern void mips_init_after_args PARAMS ((void));
#define tc_init_after_args mips_init_after_args

#endif /* TC_MIPS */
