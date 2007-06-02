/* Generated automatically by the program `gencodes'
   from the machine description file `md'.  */

#ifndef GCC_INSN_CODES_H
#define GCC_INSN_CODES_H

enum insn_code {
  CODE_FOR_movbi = 1,
  CODE_FOR_movsi_symbolic = 4,
  CODE_FOR_movdi_symbolic = 6,
  CODE_FOR_load_gprel = 9,
  CODE_FOR_gprel64_offset = 10,
  CODE_FOR_load_ltoff_dtpmod = 13,
  CODE_FOR_load_ltoff_dtprel = 14,
  CODE_FOR_load_ltoff_tprel = 19,
  CODE_FOR_extendqidi2 = 29,
  CODE_FOR_extendhidi2 = 30,
  CODE_FOR_extendsidi2 = 31,
  CODE_FOR_zero_extendqidi2 = 32,
  CODE_FOR_zero_extendhidi2 = 33,
  CODE_FOR_zero_extendsidi2 = 34,
  CODE_FOR_extendsfdf2 = 35,
  CODE_FOR_extendsftf2 = 36,
  CODE_FOR_extenddftf2 = 37,
  CODE_FOR_truncdfsf2 = 38,
  CODE_FOR_trunctfsf2 = 39,
  CODE_FOR_trunctfdf2 = 40,
  CODE_FOR_floatditf2 = 41,
#define CODE_FOR_floatdidf2 CODE_FOR_nothing
#define CODE_FOR_floatdisf2 CODE_FOR_nothing
  CODE_FOR_fix_truncsfdi2 = 42,
  CODE_FOR_fix_truncdfdi2 = 43,
  CODE_FOR_fix_trunctfdi2 = 44,
  CODE_FOR_fix_trunctfdi2_alts = 45,
  CODE_FOR_floatunsdisf2 = 46,
  CODE_FOR_floatunsdidf2 = 47,
  CODE_FOR_floatunsditf2 = 48,
  CODE_FOR_fixuns_truncsfdi2 = 49,
  CODE_FOR_fixuns_truncdfdi2 = 50,
  CODE_FOR_fixuns_trunctfdi2 = 51,
  CODE_FOR_fixuns_trunctfdi2_alts = 52,
  CODE_FOR_extv = 53,
  CODE_FOR_extzv = 54,
  CODE_FOR_shift_mix4left = 57,
  CODE_FOR_mix4right = 59,
  CODE_FOR_andbi3 = 61,
  CODE_FOR_iorbi3 = 63,
  CODE_FOR_one_cmplbi2 = 65,
  CODE_FOR_mulhi3 = 90,
  CODE_FOR_addsi3 = 91,
  CODE_FOR_subsi3 = 95,
  CODE_FOR_mulsi3 = 97,
  CODE_FOR_maddsi4 = 98,
  CODE_FOR_negsi2 = 99,
  CODE_FOR_divsi3_internal = 100,
  CODE_FOR_adddi3 = 101,
  CODE_FOR_subdi3 = 104,
  CODE_FOR_muldi3 = 106,
  CODE_FOR_madddi4 = 107,
  CODE_FOR_smuldi3_highpart = 109,
  CODE_FOR_umuldi3_highpart = 110,
  CODE_FOR_negdi2 = 111,
  CODE_FOR_divdi3_internal_lat = 113,
  CODE_FOR_divdi3_internal_thr = 114,
  CODE_FOR_addsf3 = 115,
  CODE_FOR_subsf3 = 116,
  CODE_FOR_mulsf3 = 117,
  CODE_FOR_abssf2 = 118,
  CODE_FOR_negsf2 = 119,
  CODE_FOR_minsf3 = 121,
  CODE_FOR_maxsf3 = 122,
  CODE_FOR_divsf3_internal_lat = 127,
  CODE_FOR_divsf3_internal_thr = 128,
  CODE_FOR_adddf3 = 129,
  CODE_FOR_subdf3 = 131,
  CODE_FOR_muldf3 = 133,
  CODE_FOR_absdf2 = 135,
  CODE_FOR_negdf2 = 136,
  CODE_FOR_mindf3 = 138,
  CODE_FOR_maxdf3 = 139,
  CODE_FOR_divdf3_internal_lat = 149,
  CODE_FOR_divdf3_internal_thr = 150,
  CODE_FOR_addtf3 = 151,
  CODE_FOR_subtf3 = 154,
  CODE_FOR_multf3 = 157,
  CODE_FOR_abstf2 = 163,
  CODE_FOR_negtf2 = 164,
  CODE_FOR_mintf3 = 166,
  CODE_FOR_maxtf3 = 167,
  CODE_FOR_divtf3_internal_lat = 184,
  CODE_FOR_divtf3_internal_thr = 185,
  CODE_FOR_ashldi3 = 190,
  CODE_FOR_ashrdi3 = 193,
  CODE_FOR_lshrdi3 = 194,
  CODE_FOR_one_cmplsi2 = 197,
  CODE_FOR_anddi3 = 198,
  CODE_FOR_iordi3 = 200,
  CODE_FOR_xordi3 = 201,
  CODE_FOR_one_cmpldi2 = 202,
  CODE_FOR_doloop_end_internal = 214,
  CODE_FOR_call_nogp = 223,
  CODE_FOR_call_value_nogp = 224,
  CODE_FOR_sibcall_nogp = 225,
  CODE_FOR_call_gp = 226,
  CODE_FOR_call_value_gp = 227,
  CODE_FOR_sibcall_gp = 228,
  CODE_FOR_return_internal = 229,
  CODE_FOR_return = 230,
  CODE_FOR_jump = 233,
  CODE_FOR_indirect_jump = 234,
  CODE_FOR_prologue_allocate_stack = 236,
  CODE_FOR_epilogue_deallocate_stack = 237,
  CODE_FOR_prologue_use = 238,
  CODE_FOR_alloc = 239,
  CODE_FOR_gr_spill_internal = 240,
  CODE_FOR_gr_restore_internal = 241,
  CODE_FOR_fr_spill = 242,
  CODE_FOR_fr_restore = 243,
  CODE_FOR_bsp_value = 244,
  CODE_FOR_set_bsp = 245,
  CODE_FOR_flushrs = 246,
  CODE_FOR_nop = 247,
  CODE_FOR_nop_m = 248,
  CODE_FOR_nop_i = 249,
  CODE_FOR_nop_f = 250,
  CODE_FOR_nop_b = 251,
  CODE_FOR_nop_x = 252,
  CODE_FOR_bundle_selector = 253,
  CODE_FOR_blockage = 254,
  CODE_FOR_insn_group_barrier = 255,
  CODE_FOR_break_f = 258,
  CODE_FOR_prefetch = 259,
  CODE_FOR_builtin_setjmp_receiver = 260,
  CODE_FOR_fetchadd_acq_si = 262,
  CODE_FOR_fetchadd_acq_di = 263,
  CODE_FOR_cmpxchg_acq_si = 264,
  CODE_FOR_cmpxchg_acq_di = 265,
  CODE_FOR_xchgsi = 266,
  CODE_FOR_xchgdi = 267,
  CODE_FOR_pred_rel_mutex = 268,
  CODE_FOR_safe_across_calls_all = 269,
  CODE_FOR_safe_across_calls_normal = 270,
  CODE_FOR_ptr_extend = 271,
  CODE_FOR_movqi = 276,
  CODE_FOR_movhi = 277,
  CODE_FOR_movsi = 278,
  CODE_FOR_movdi = 280,
  CODE_FOR_load_fptr = 283,
  CODE_FOR_load_gprel64 = 284,
  CODE_FOR_load_symptr = 285,
  CODE_FOR_load_dtprel = 286,
  CODE_FOR_add_dtprel = 287,
  CODE_FOR_load_tprel = 288,
  CODE_FOR_add_tprel = 289,
  CODE_FOR_movti = 290,
  CODE_FOR_reload_inti = 293,
  CODE_FOR_reload_outti = 294,
  CODE_FOR_movsf = 295,
  CODE_FOR_movdf = 296,
  CODE_FOR_movtf = 297,
  CODE_FOR_insv = 298,
  CODE_FOR_abssi2 = 312,
  CODE_FOR_sminsi3 = 313,
  CODE_FOR_smaxsi3 = 314,
  CODE_FOR_uminsi3 = 315,
  CODE_FOR_umaxsi3 = 316,
  CODE_FOR_divsi3 = 317,
  CODE_FOR_modsi3 = 318,
  CODE_FOR_udivsi3 = 319,
  CODE_FOR_umodsi3 = 320,
  CODE_FOR_absdi2 = 323,
  CODE_FOR_smindi3 = 324,
  CODE_FOR_smaxdi3 = 325,
  CODE_FOR_umindi3 = 326,
  CODE_FOR_umaxdi3 = 327,
  CODE_FOR_ffsdi2 = 328,
  CODE_FOR_divdi3 = 329,
  CODE_FOR_moddi3 = 330,
  CODE_FOR_udivdi3 = 331,
  CODE_FOR_umoddi3 = 332,
  CODE_FOR_divsf3 = 335,
  CODE_FOR_divdf3 = 338,
  CODE_FOR_divtf3 = 341,
  CODE_FOR_ashlsi3 = 344,
  CODE_FOR_ashrsi3 = 345,
  CODE_FOR_lshrsi3 = 346,
  CODE_FOR_rotrsi3 = 347,
  CODE_FOR_rotlsi3 = 349,
  CODE_FOR_rotrdi3 = 352,
  CODE_FOR_rotldi3 = 353,
  CODE_FOR_cmpbi = 354,
  CODE_FOR_cmpsi = 355,
  CODE_FOR_cmpdi = 356,
  CODE_FOR_cmpsf = 357,
  CODE_FOR_cmpdf = 358,
  CODE_FOR_cmptf = 359,
  CODE_FOR_beq = 360,
  CODE_FOR_bne = 361,
  CODE_FOR_blt = 362,
  CODE_FOR_ble = 363,
  CODE_FOR_bgt = 364,
  CODE_FOR_bge = 365,
  CODE_FOR_bltu = 366,
  CODE_FOR_bleu = 367,
  CODE_FOR_bgtu = 368,
  CODE_FOR_bgeu = 369,
  CODE_FOR_bunordered = 370,
  CODE_FOR_bordered = 371,
  CODE_FOR_doloop_end = 372,
  CODE_FOR_seq = 373,
  CODE_FOR_sne = 374,
  CODE_FOR_slt = 375,
  CODE_FOR_sle = 376,
  CODE_FOR_sgt = 377,
  CODE_FOR_sge = 378,
  CODE_FOR_sltu = 379,
  CODE_FOR_sleu = 380,
  CODE_FOR_sgtu = 381,
  CODE_FOR_sgeu = 382,
  CODE_FOR_sunordered = 383,
  CODE_FOR_sordered = 384,
  CODE_FOR_call = 394,
  CODE_FOR_sibcall = 395,
  CODE_FOR_call_value = 396,
  CODE_FOR_sibcall_value = 397,
  CODE_FOR_untyped_call = 398,
  CODE_FOR_tablejump = 404,
  CODE_FOR_prologue = 405,
  CODE_FOR_epilogue = 406,
  CODE_FOR_sibcall_epilogue = 407,
  CODE_FOR_gr_spill = 408,
  CODE_FOR_gr_restore = 409,
  CODE_FOR_trap = 410,
  CODE_FOR_conditional_trap = 411,
  CODE_FOR_save_stack_nonlocal = 412,
  CODE_FOR_nonlocal_goto = 413,
  CODE_FOR_eh_epilogue = 415,
  CODE_FOR_restore_stack_nonlocal = 416,
  CODE_FOR_mf = 417,
  CODE_FOR_nothing
};

#endif /* GCC_INSN_CODES_H */