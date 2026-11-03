// 1804902d0 FUN_1804902d0

void FUN_1804902d0(longlong param_1,longlong *param_2,longlong *param_3,longlong *param_4,
                  longlong *param_5,undefined8 param_6,longlong param_7,longlong param_8)

{
  float fVar1;
  undefined8 uVar2;
  longlong lVar3;
  longlong *plVar4;
  code *pcVar5;
  longlong *plVar6;
  char cVar7;
  longlong *plVar8;
  undefined8 *puVar9;
  undefined8 *puVar10;
  ulonglong uVar11;
  ulonglong uVar12;
  ulonglong uVar13;
  undefined8 *puVar14;
  longlong lVar15;
  longlong lVar16;
  longlong lVar17;
  undefined8 uVar18;
  ulonglong uVar19;
  undefined1 auStack_1a8 [32];
  undefined8 uStack_188;
  longlong lStack_178;
  longlong *plStack_170;
  undefined8 uStack_168;
  longlong lStack_160;
  longlong lStack_158;
  longlong lStack_150;
  longlong *plStack_148;
  ulonglong uStack_140;
  longlong *plStack_138;
  longlong *plStack_128;
  longlong *plStack_120;
  longlong lStack_118;
  undefined8 uStack_110;
  longlong lStack_108;
  longlong **pplStack_100;
  undefined8 *puStack_f8;
  undefined8 *puStack_f0;
  undefined8 *puStack_e8;
  longlong lStack_e0;
  longlong lStack_d8;
  longlong lStack_d0;
  longlong lStack_c8;
  longlong lStack_c0;
  longlong lStack_b8;
  longlong *plStack_b0;
  longlong *plStack_a8;
  longlong *plStack_a0;
  longlong lStack_98;
  longlong lStack_90;
  longlong lStack_88;
  longlong lStack_80;
  longlong lStack_78;
  longlong lStack_70;
  ulonglong uStack_68;
  
  uStack_68 = DAT_18264bfc0 ^ (ulonglong)auStack_1a8;
  plStack_170 = param_5;
  uStack_168 = param_6;
  lStack_150 = param_8;
  uVar19 = param_2[1] - *param_2 >> 3;
  lStack_c8 = 0;
  lStack_c0 = 0;
  lStack_b8 = 0;
  lStack_160 = param_1;
  plStack_148 = param_2;
  plStack_138 = param_3;
  if (uVar19 != 0) {
    if (uVar19 < 0x1c71c71c71c71c8) {
      lStack_c8 = FUN_180440bd0(uVar19 * 0x90);
      lStack_b8 = lStack_c8 + uVar19 * 0x90;
      lStack_c0 = lStack_c8;
      lStack_c0 = FUN_180486440(lStack_c8);
      goto LAB_1804903ac;
    }
    FUN_180444e60();
LAB_180490b67:
    FUN_180444e60();
LAB_180490b6d:
    uStack_188 = 0;
    FUN_18040e3e4(0,0,0,0);
    pcVar5 = (code *)swi(3);
    (*pcVar5)();
    return;
  }
LAB_1804903ac:
  plStack_b0 = (longlong *)0x0;
  plStack_a8 = (longlong *)0x0;
  plStack_a0 = (longlong *)0x0;
  if (uVar19 != 0) {
    if (0xfffffffffffffff < uVar19) goto LAB_180490b67;
    plStack_b0 = (longlong *)FUN_180440bd0(uVar19 * 0x10);
    plStack_a0 = plStack_b0 + uVar19 * 2;
    plStack_a8 = plStack_b0;
    uVar11 = uVar19;
    do {
      plStack_a8[1] = 0;
      *plStack_a8 = 0;
      plStack_a8 = plStack_a8 + 2;
      uVar11 = uVar11 - 1;
    } while (uVar11 != 0);
  }
  FUN_1804a0860(&lStack_178);
  uVar18 = uStack_168;
  uVar2 = *(undefined8 *)(lStack_160 + 0x10);
  plStack_120 = &lStack_c8;
  lStack_118 = lStack_160;
  uStack_110 = uStack_168;
  lStack_108 = param_7;
  pplStack_100 = &plStack_b0;
  plStack_128 = param_2;
  cVar7 = (*(code *)PTR__guard_dispatch_icall_1806db690)();
  FUN_180487570(uVar2,cVar7 == '\0',uVar19,&plStack_128);
  plVar8 = (longlong *)FUN_1804a0860(&lStack_158);
  LOCK();
  *(longlong *)(param_7 + 0x88) = *(longlong *)(param_7 + 0x88) + (*plVar8 - lStack_178);
  UNLOCK();
  if (*param_3 != param_3[1]) {
    FUN_180440f60();
    param_3[1] = *param_3;
  }
  if (*param_4 != param_4[1]) {
    FUN_180440f60();
    param_4[1] = *param_4;
  }
  if (*param_5 != param_5[1]) {
    FUN_180440f60();
    param_5[1] = *param_5;
  }
  puStack_f8 = (undefined8 *)0x0;
  puStack_f0 = (undefined8 *)0x0;
  puStack_e8 = (undefined8 *)0x0;
  lStack_e0 = 0;
  lStack_d8 = 0;
  lStack_d0 = 0;
  if (uVar19 == 0) {
    FUN_1804a13b0(&lStack_e0,0);
LAB_18049077d:
    lVar17 = lStack_160;
    if (*(char *)(lStack_160 + 0x4b) == '\0') {
      if (param_3 != &lStack_e0) {
        FUN_1804444e0(param_3);
        *param_3 = lStack_e0;
        param_3[1] = lStack_d8;
        param_3[2] = lStack_d0;
        lStack_e0 = 0;
        lStack_d0 = 0;
        lStack_d8 = 0;
      }
    }
    else {
      FUN_180491610(&lStack_98,&puStack_f8,&lStack_e0,uVar18);
      if (param_3 != &lStack_98) {
        lVar15 = *param_3;
        *param_3 = lStack_98;
        lVar16 = param_3[1];
        param_3[1] = lStack_90;
        lVar3 = param_3[2];
        param_3[2] = lStack_88;
        lStack_98 = lVar15;
        lStack_90 = lVar16;
        lStack_88 = lVar3;
      }
      if (param_5 != &lStack_80) {
        lVar15 = *param_5;
        *param_5 = lStack_80;
        lVar16 = param_5[1];
        param_5[1] = lStack_78;
        lVar3 = param_5[2];
        param_5[2] = lStack_70;
        lStack_80 = lVar15;
        lStack_78 = lVar16;
        lStack_70 = lVar3;
      }
      FUN_1804444e0(&lStack_80);
      FUN_1804444e0(&lStack_98);
    }
    if (*(char *)(lVar17 + 0x4a) != '\0') {
      FUN_1804a0860(&plStack_170);
      FUN_180499850(param_3);
      lVar17 = lStack_150;
      fVar1 = *(float *)(lStack_150 + 0x114);
      plVar8 = (longlong *)FUN_1804a0860(&lStack_150);
      *(float *)(lVar17 + 0x114) = (float)(*plVar8 - (longlong)plStack_170) / DAT_18258fcac + fVar1;
    }
    lVar17 = lStack_d8;
    if (lStack_e0 != 0) {
      if (lStack_e0 != lStack_d8) {
        lVar15 = lStack_e0 + 0x40;
        do {
          if (*(longlong *)(lVar15 + 0x40) != 0) {
            (*(code *)PTR__guard_dispatch_icall_1806db690)(*(longlong *)(lVar15 + 0x40),1);
          }
          FUN_1804446b0(lVar15);
          FUN_180444580(lVar15 + -0x18);
          lVar16 = lVar15 + 0x50;
          lVar15 = lVar15 + 0x90;
        } while (lVar16 != lVar17);
      }
      if ((0xfff < (ulonglong)((lStack_d0 - lStack_e0 >> 4) * 0x10)) &&
         (0x1f < (lStack_e0 - *(longlong *)(lStack_e0 + -8)) - 8U)) goto LAB_180490b92;
      FUN_1803eb2c8();
      lStack_e0 = 0;
      lStack_d8 = 0;
      lStack_d0 = 0;
    }
    if (puStack_f8 != (undefined8 *)0x0) {
      if ((0xfff < ((longlong)puStack_e8 - (longlong)puStack_f8 & 0xfffffffffffffff8U)) &&
         (0x1f < (ulonglong)((longlong)puStack_f8 + (-8 - puStack_f8[-1])))) goto LAB_180490bab;
      FUN_1803eb2c8();
      puStack_f8 = (undefined8 *)0x0;
      puStack_f0 = (undefined8 *)0x0;
      puStack_e8 = (undefined8 *)0x0;
    }
    plVar6 = plStack_a8;
    plVar8 = plStack_b0;
    if (plStack_b0 != (longlong *)0x0) {
      for (; plVar8 != plVar6; plVar8 = plVar8 + 2) {
        plVar4 = (longlong *)*plVar8;
        if (plVar4 != (longlong *)0x0) {
          FUN_1804448d0(plVar4 + 1);
          if (*plVar4 != 0) {
            (*(code *)PTR__guard_dispatch_icall_1806db690)(*plVar4,1);
          }
          FUN_1803eb2c8(plVar4,0x30);
        }
      }
      plVar8 = plStack_b0;
      if ((0xfff < ((longlong)plStack_a0 - (longlong)plStack_b0 & 0xfffffffffffffff0U)) &&
         (plVar8 = (longlong *)plStack_b0[-1],
         0x1f < (ulonglong)((longlong)plStack_b0 + (-8 - (longlong)plVar8)))) goto LAB_180490bc4;
      FUN_1803eb2c8(plVar8);
      plStack_b0 = (longlong *)0x0;
      plStack_a8 = (longlong *)0x0;
      plStack_a0 = (longlong *)0x0;
    }
    lVar17 = lStack_c0;
    if (lStack_c8 != 0) {
      if (lStack_c8 != lStack_c0) {
        lVar15 = lStack_c8 + 0x40;
        do {
          if (*(longlong *)(lVar15 + 0x40) != 0) {
            (*(code *)PTR__guard_dispatch_icall_1806db690)(*(longlong *)(lVar15 + 0x40),1);
          }
          FUN_1804446b0(lVar15);
          FUN_180444580(lVar15 + -0x18);
          lVar16 = lVar15 + 0x50;
          lVar15 = lVar15 + 0x90;
        } while (lVar16 != lVar17);
      }
      if ((0xfff < (ulonglong)((lStack_b8 - lStack_c8 >> 4) * 0x10)) &&
         (0x1f < (lStack_c8 - *(longlong *)(lStack_c8 + -8)) - 8U)) {
        uStack_188 = 0;
        FUN_18040e3e4(0,0,0,0);
        pcVar5 = (code *)swi(3);
        (*pcVar5)();
        return;
      }
      FUN_1803eb2c8();
    }
    FUN_1803eae20(uStack_68 ^ (ulonglong)auStack_1a8);
    return;
  }
  if (uVar19 < 0x2000000000000000) {
    puVar9 = (undefined8 *)FUN_180440bd0(uVar19 * 8);
    FUN_1806a7f00(puVar9,puStack_f8,(longlong)puStack_f0 - (longlong)puStack_f8);
    if (puStack_f8 != (undefined8 *)0x0) {
      if ((0xfff < ((longlong)puStack_e8 - (longlong)puStack_f8 & 0xfffffffffffffff8U)) &&
         (0x1f < (ulonglong)((longlong)puStack_f8 + (-8 - puStack_f8[-1])))) goto LAB_180490b6d;
      FUN_1803eb2c8();
    }
    puStack_e8 = puVar9 + uVar19;
    puStack_f8 = puVar9;
    puStack_f0 = puVar9;
    FUN_1804a13b0(&lStack_e0,uVar19);
    uVar11 = 0;
    lVar17 = 0;
    do {
      puVar9 = puStack_f0;
      if (*(char *)(lStack_c8 + lVar17 + 0x89) == '\0') {
        lStack_178 = *plStack_148;
        if (puStack_f0 == puStack_e8) {
          lVar15 = (longlong)puStack_f0 - (longlong)puStack_f8 >> 3;
          if (lVar15 == 0x1fffffffffffffff) goto LAB_180490b8c;
          uStack_140 = lVar15 + 1;
          uVar12 = (longlong)puStack_e8 - (longlong)puStack_f8 >> 3;
          if (0x1fffffffffffffff - (uVar12 >> 1) < uVar12) {
            uVar13 = 0x1fffffffffffffff;
          }
          else {
            uVar12 = uVar12 + (uVar12 >> 1);
            uVar13 = uStack_140;
            if (uStack_140 <= uVar12) {
              uVar13 = uVar12;
            }
            if (0x1fffffffffffffff < uVar13) {
              FUN_1804441f0();
              pcVar5 = (code *)swi(3);
              (*pcVar5)();
              return;
            }
          }
          lStack_158 = uVar13 * 8;
          puVar10 = (undefined8 *)FUN_180440bd0(lStack_158);
          puVar10[lVar15] = *(undefined8 *)(lStack_178 + uVar11 * 8);
          if (puVar9 == puStack_f0) {
            lVar16 = (longlong)puStack_f0 - (longlong)puStack_f8;
            puVar14 = puVar10;
            puVar9 = puStack_f8;
          }
          else {
            FUN_1806a7f00(puVar10,puStack_f8,(longlong)puVar9 - (longlong)puStack_f8);
            lVar16 = (longlong)puStack_f0 - (longlong)puVar9;
            puVar14 = puVar10 + lVar15 + 1;
          }
          FUN_1806a7f00(puVar14,puVar9,lVar16);
          if (puStack_f8 != (undefined8 *)0x0) {
            if ((0xfff < ((longlong)puStack_e8 - (longlong)puStack_f8 & 0xfffffffffffffff8U)) &&
               (0x1f < (ulonglong)((longlong)puStack_f8 + (-8 - puStack_f8[-1]))))
            goto LAB_180490b6d;
            FUN_1803eb2c8();
          }
          puStack_f0 = puVar10 + uStack_140;
          puStack_e8 = (undefined8 *)(lStack_158 + (longlong)puVar10);
          puStack_f8 = puVar10;
        }
        else {
          *puStack_f0 = *(undefined8 *)(lStack_178 + uVar11 * 8);
          puStack_f0 = puStack_f0 + 1;
        }
        if (lStack_d8 == lStack_d0) {
          lVar15 = lStack_d8;
          plVar8 = &lStack_e0;
          goto LAB_180490753;
        }
        FUN_180488ea0(lStack_d8,lStack_c8 + lVar17);
        lStack_d8 = lStack_d8 + 0x90;
      }
      else {
        lVar15 = param_4[1];
        plVar8 = param_4;
        if (lVar15 == param_4[2]) {
LAB_180490753:
          FUN_18047e2b0(plVar8,lVar15);
        }
        else {
          FUN_180488ea0(lVar15,lStack_c8 + lVar17);
          param_4[1] = param_4[1] + 0x90;
        }
      }
      uVar11 = uVar11 + 1;
      lVar17 = lVar17 + 0x90;
      param_3 = plStack_138;
      uVar18 = uStack_168;
      param_5 = plStack_170;
    } while (uVar11 < uVar19);
    goto LAB_18049077d;
  }
LAB_180490b8c:
  FUN_180444e60();
LAB_180490b92:
  uStack_188 = 0;
  FUN_18040e3e4(0,0,0,0);
LAB_180490bab:
  uStack_188 = 0;
  FUN_18040e3e4(0,0,0,0);
LAB_180490bc4:
  uStack_188 = 0;
  FUN_18040e3e4(0,0,0,0);
  pcVar5 = (code *)swi(3);
  (*pcVar5)();
  return;
}

