// 18052ab40 FUN_18052ab40

/* WARNING: Removing unreachable block (ram,0x00018052b018) */

void FUN_18052ab40(undefined8 param_1,undefined8 param_2,longlong *param_3,ulonglong param_4,
                  float *param_5,int *param_6,undefined8 param_7)

{
  float fVar1;
  float fVar2;
  float fVar3;
  float fVar4;
  float fVar5;
  float fVar6;
  float fVar7;
  float fVar8;
  float fVar9;
  code *pcVar10;
  bool bVar11;
  int iVar12;
  undefined8 uVar13;
  longlong lVar14;
  int *piVar15;
  float *pfVar16;
  float *pfVar17;
  int *piVar18;
  ulonglong uVar19;
  int *piVar20;
  longlong lVar21;
  int *piVar22;
  longlong lVar23;
  float fVar24;
  float fVar25;
  float fVar26;
  undefined4 uVar27;
  float fVar28;
  float fVar29;
  undefined1 auStack_448 [32];
  int *piStack_428;
  longlong *plStack_420;
  int *piStack_418;
  undefined8 **ppuStack_410;
  undefined8 uStack_3f8;
  undefined1 auStack_3f0 [24];
  int *piStack_3d8;
  longlong *plStack_3d0;
  float *pfStack_3c8;
  undefined8 uStack_3c0;
  undefined8 uStack_3b8;
  undefined8 uStack_3b0;
  ulonglong uStack_3a8;
  undefined1 auStack_398 [80];
  undefined1 auStack_348 [16];
  undefined1 auStack_338 [96];
  undefined1 auStack_2d8 [96];
  undefined1 auStack_278 [144];
  undefined8 uStack_1e8;
  int *piStack_1e0;
  undefined8 *puStack_1d8;
  int *piStack_1d0;
  int aiStack_1c8 [2];
  longlong lStack_1c0;
  longlong lStack_1b8;
  longlong lStack_1b0;
  undefined8 *puStack_1a8;
  undefined8 *puStack_1a0;
  undefined8 *puStack_198;
  undefined8 uStack_188;
  undefined8 uStack_180;
  undefined8 uStack_178;
  undefined8 uStack_170;
  undefined8 uStack_168;
  undefined8 uStack_160;
  undefined8 uStack_158;
  undefined8 uStack_150;
  undefined8 uStack_148;
  undefined8 uStack_140;
  undefined8 uStack_138;
  undefined8 uStack_130;
  undefined8 uStack_128;
  undefined8 uStack_120;
  undefined8 uStack_118;
  undefined8 uStack_110;
  undefined8 uStack_108;
  undefined8 uStack_100;
  undefined8 uStack_f8;
  undefined8 uStack_f0;
  undefined8 uStack_e8;
  undefined8 uStack_e0;
  undefined8 uStack_d8;
  undefined8 uStack_d0;
  ulonglong uStack_c8;
  
  uStack_c8 = DAT_18264bfc0 ^ (ulonglong)auStack_448;
  pfStack_3c8 = param_5;
  uStack_3f8 = param_6;
  uStack_3b8 = param_7;
  piVar22 = (int *)((ulonglong)(param_3[1] - *param_3 >> 3) >> 1);
  piStack_1e0 = (int *)0x0;
  puStack_1d8 = (undefined8 *)0x0;
  plStack_3d0 = param_3;
  uStack_3c0 = param_1;
  uStack_3b0 = param_2;
  uStack_3a8 = param_4;
  if (piVar22 == (int *)0x0) {
    uStack_1e8 = 0;
    FUN_1800daec0(&uStack_1e8,&DAT_182586fc8);
    FUN_18048c440(uStack_1e8,0);
    uVar13 = FUN_1800dc480(&uStack_1e8);
    FUN_18048bfa0(uVar13,0);
    FUN_1800de7f0(&uStack_1e8,&piStack_3d8);
    FUN_1804f4a50(&uStack_1e8);
    piVar18 = piStack_3d8;
  }
  else {
    piVar18 = (int *)0x0;
  }
  piStack_3d8 = (int *)0x0;
  puStack_1d8 = (undefined8 *)((ulonglong)puStack_1d8 & 0xffffffffffffff00);
  bVar11 = false;
  piStack_1e0 = piVar18;
  while ((piVar18 != (int *)0x0 && (!bVar11))) {
    uVar13 = FUN_1800daf20(auStack_398,&DAT_182586f88,0xbf);
    uVar13 = FUN_1800e2f70(uVar13);
    uVar13 = FUN_18047c970(uVar13,&DAT_1825829c0);
    piVar20 = piVar18;
    if (0xf < *(ulonglong *)(piVar18 + 6)) {
      piVar20 = *(int **)piVar18;
    }
    uVar13 = FUN_180480360(uVar13,piVar20);
    FUN_18047c970(uVar13,&DAT_1825829b8);
    FUN_1800db4f0(auStack_398);
    puStack_1d8 = (undefined8 *)CONCAT71(puStack_1d8._1_7_,1);
    bVar11 = true;
  }
  if (piVar18 == (int *)0x0) {
    fVar25 = 0.0;
    piVar18 = (int *)0x0;
    if (piVar22 < (int *)0x4) {
      if (piVar22 != (int *)0x0) {
        lVar14 = *param_3;
        lVar21 = param_3[1];
        goto LAB_18052ae84;
      }
    }
    else {
      lVar14 = *param_3;
      lVar21 = param_3[1];
      pfVar16 = (float *)(lVar14 + 8);
      pfVar17 = (float *)(lVar14 + -4 + (lVar21 - lVar14 >> 3) * 8);
      lVar23 = ((ulonglong)(piVar22 + -1) >> 2) + 1;
      piVar18 = (int *)(lVar23 * 4);
      do {
        fVar29 = *pfVar16;
        fVar28 = pfVar17[-3];
        fVar26 = pfVar16[1];
        fVar1 = pfVar17[-2];
        fVar2 = pfVar16[2];
        fVar3 = pfVar17[-5];
        fVar4 = pfVar16[3];
        fVar5 = pfVar17[-4];
        fVar6 = pfVar16[4];
        fVar7 = pfVar17[-7];
        fVar8 = pfVar16[5];
        fVar9 = pfVar17[-6];
        fVar24 = (pfVar16[-1] - *pfVar17) * (pfVar16[-1] - *pfVar17) +
                 (pfVar16[-2] - pfVar17[-1]) * (pfVar16[-2] - pfVar17[-1]);
        if (fVar24 < 0.0) {
          fVar24 = (float)FUN_180425010(fVar24);
        }
        else {
          fVar24 = SQRT(fVar24);
        }
        fVar29 = (fVar26 - fVar1) * (fVar26 - fVar1) + (fVar29 - fVar28) * (fVar29 - fVar28);
        if (fVar29 < 0.0) {
          fVar29 = (float)FUN_180425010(fVar29);
        }
        else {
          fVar29 = SQRT(fVar29);
        }
        fVar28 = (fVar4 - fVar5) * (fVar4 - fVar5) + (fVar2 - fVar3) * (fVar2 - fVar3);
        if (fVar28 < 0.0) {
          fVar28 = (float)FUN_180425010(fVar28);
        }
        else {
          fVar28 = SQRT(fVar28);
        }
        fVar26 = (fVar8 - fVar9) * (fVar8 - fVar9) + (fVar6 - fVar7) * (fVar6 - fVar7);
        if (fVar26 < 0.0) {
          fVar26 = (float)FUN_180425010(fVar26);
        }
        else {
          fVar26 = SQRT(fVar26);
        }
        fVar25 = fVar24 + fVar25 + fVar29 + fVar28 + fVar26;
        pfVar16 = pfVar16 + 8;
        pfVar17 = pfVar17 + -8;
        lVar23 = lVar23 + -1;
      } while (lVar23 != 0);
      param_3 = plStack_3d0;
      param_6 = uStack_3f8;
      if (piVar18 < piVar22) {
LAB_18052ae84:
        pfVar16 = (float *)(((lVar21 - lVar14 >> 3) - (longlong)piVar18) * 8 + -4 + lVar14);
        do {
          fVar29 = *(float *)(lVar14 + (longlong)piVar18 * 8) - pfVar16[-1];
          fVar28 = *(float *)(lVar14 + 4 + (longlong)piVar18 * 8) - *pfVar16;
          fVar29 = fVar28 * fVar28 + fVar29 * fVar29;
          if (fVar29 < 0.0) {
            fVar29 = (float)FUN_180425010(fVar29);
          }
          else {
            fVar29 = SQRT(fVar29);
          }
          fVar25 = fVar25 + fVar29;
          piVar18 = (int *)((longlong)piVar18 + 1);
          pfVar16 = pfVar16 + -2;
          param_3 = plStack_3d0;
          param_6 = uStack_3f8;
        } while (piVar18 < piVar22);
      }
    }
    piStack_1e0 = (int *)0x0;
    puStack_1d8 = (undefined8 *)0x0;
    if (fVar25 <= 0.0) {
      uStack_1e8 = 0;
      FUN_1800daec0(&uStack_1e8,&DAT_182587008);
      FUN_18048c1e0(uStack_1e8,fVar25);
      uVar13 = FUN_1800dc480(&uStack_1e8);
      FUN_18048c1e0(uVar13);
      FUN_1800de7f0(&uStack_1e8,&uStack_3f8);
      FUN_1804f4a50(&uStack_1e8);
      piVar18 = uStack_3f8;
    }
    else {
      piVar18 = (int *)0x0;
    }
    uStack_3f8 = (int *)0x0;
    puStack_1d8 = (undefined8 *)((ulonglong)puStack_1d8 & 0xffffffffffffff00);
    bVar11 = false;
    piStack_1e0 = piVar18;
    while ((pfVar16 = pfStack_3c8, piVar18 != (int *)0x0 && (!bVar11))) {
      uVar13 = FUN_1800daf20(auStack_398,&DAT_182586f88,0xd0);
      uVar13 = FUN_1800e2f70(uVar13);
      uVar13 = FUN_18047c970(uVar13,&DAT_1825829c0);
      piVar20 = piVar18;
      if (0xf < *(ulonglong *)(piVar18 + 6)) {
        piVar20 = *(int **)piVar18;
      }
      uVar13 = FUN_180480360(uVar13,piVar20);
      FUN_18047c970(uVar13,&DAT_1825829b8);
      FUN_1800db4f0(auStack_398);
      puStack_1d8 = (undefined8 *)CONCAT71(puStack_1d8._1_7_,1);
      bVar11 = true;
    }
    if (piVar18 == (int *)0x0) {
      *pfStack_3c8 = fVar25 / (float)(longlong)piVar22;
      lStack_1c0 = 0;
      lStack_1b8 = 0;
      lStack_1b0 = 0;
      piVar18 = (int *)((longlong)piVar22 - 1);
      if (piVar18 != (int *)0x0) {
        if ((int *)0x3fffffffffffffff < piVar18) goto LAB_18052b8a8;
        lVar14 = FUN_180440bd0((longlong)piVar18 * 4);
        FUN_1806a7f00(lVar14,lStack_1c0,lStack_1b8 - lStack_1c0);
        if (lStack_1c0 == 0) {
LAB_18052b0eb:
          lStack_1b0 = (longlong)piVar18 * 4 + lVar14;
          lStack_1b8 = lVar14;
          goto LAB_18052b104;
        }
        if (((lStack_1b0 - lStack_1c0 & 0xfffffffffffffffcU) < 0x1000) ||
           ((lStack_1c0 - *(longlong *)(lStack_1c0 + -8)) - 8U < 0x20)) {
          FUN_1803eb2c8();
          goto LAB_18052b0eb;
        }
LAB_18052b83d:
        piStack_428 = (int *)0x0;
        FUN_18040e3e4(0,0,0,0);
        goto LAB_18052b852;
      }
LAB_18052b104:
      aiStack_1c8[0] = 0;
      uVar19 = param_3[1] - *param_3 >> 3;
      puStack_1a8 = (undefined8 *)0x0;
      puStack_1a0 = (undefined8 *)0x0;
      puStack_198 = (undefined8 *)0x0;
      lStack_1c0 = lStack_1b8;
      if (uVar19 == 0) {
LAB_18052b184:
        ppuStack_410 = &puStack_1a8;
        piStack_418 = aiStack_1c8;
        plStack_420 = &lStack_1c0;
        piStack_428 = (int *)uStack_3b8;
        (*(code *)PTR__guard_dispatch_icall_1806db690)(uStack_3c0,*pfVar16,piVar22,param_3);
        piStack_1e0 = (int *)0x0;
        puStack_1d8 = (undefined8 *)0x0;
        piVar22 = (int *)0x0;
        piVar20 = piVar22;
        if (lStack_1b8 - lStack_1c0 >> 2 == 0) {
          uStack_1e8 = 0;
          FUN_1800daec0(&uStack_1e8,&DAT_182586ff0);
          FUN_18048c440(uStack_1e8,0);
          uVar13 = FUN_1800dc480(&uStack_1e8);
          FUN_18048bfa0(uVar13,0);
          FUN_1800de7f0(&uStack_1e8,&uStack_3f8);
          FUN_1804f4a50(&uStack_1e8);
          piVar20 = uStack_3f8;
        }
        uStack_3f8 = (int *)0x0;
        puStack_1d8 = (undefined8 *)((ulonglong)puStack_1d8 & 0xffffffffffffff00);
        bVar11 = false;
        piStack_1e0 = piVar20;
        while ((piVar20 != (int *)0x0 && (!bVar11))) {
          uVar13 = FUN_1800daf20(auStack_398,&DAT_182586f88,0xda);
          uVar13 = FUN_1800e2f70(uVar13);
          uVar13 = FUN_18047c970(uVar13,&DAT_1825829c0);
          piVar15 = piVar20;
          if (0xf < *(ulonglong *)(piVar20 + 6)) {
            piVar15 = *(int **)piVar20;
          }
          uVar13 = FUN_180480360(uVar13,piVar15);
          FUN_18047c970(uVar13,&DAT_1825829b8);
          FUN_1800db4f0(auStack_398);
          puStack_1d8 = (undefined8 *)CONCAT71(puStack_1d8._1_7_,1);
          bVar11 = true;
        }
        if (piVar20 != (int *)0x0) goto LAB_18052b8b4;
        fVar25 = *(float *)(lStack_1b8 + -4);
        piStack_1e0 = (int *)0x0;
        puStack_1d8 = (undefined8 *)0x0;
        piVar20 = piVar22;
        if (fVar25 <= 0.0) {
          uStack_1e8 = 0;
          FUN_1800daec0(&uStack_1e8,&DAT_182586f50);
          FUN_18048c1e0(uStack_1e8,fVar25);
          uVar13 = FUN_1800dc480(&uStack_1e8);
          FUN_18048bfa0(uVar13,0);
          FUN_1800de7f0(&uStack_1e8,&uStack_3f8);
          FUN_1804f4a50(&uStack_1e8);
          piVar20 = uStack_3f8;
        }
        uStack_3f8 = (int *)0x0;
        puStack_1d8 = (undefined8 *)((ulonglong)puStack_1d8 & 0xffffffffffffff00);
        bVar11 = false;
        piStack_1e0 = piVar20;
        while ((iVar12 = aiStack_1c8[0], piVar20 != (int *)0x0 && (!bVar11))) {
          uVar13 = FUN_1800daf20(auStack_398,&DAT_182586f88,0xdd);
          uVar13 = FUN_1800e2f70(uVar13);
          uVar13 = FUN_18047c970(uVar13,&DAT_1825829c0);
          piVar15 = piVar20;
          if (0xf < *(ulonglong *)(piVar20 + 6)) {
            piVar15 = *(int **)piVar20;
          }
          uVar13 = FUN_180480360(uVar13,piVar15);
          FUN_18047c970(uVar13,&DAT_1825829b8);
          FUN_1800db4f0(auStack_398);
          puStack_1d8 = (undefined8 *)CONCAT71(puStack_1d8._1_7_,1);
          bVar11 = true;
        }
        if (piVar20 != (int *)0x0) goto LAB_18052b8df;
        piStack_1e0 = (int *)0x0;
        puStack_1d8 = (undefined8 *)0x0;
        piVar20 = piVar22;
        if (aiStack_1c8[0] < 1) {
          uStack_1e8 = 0;
          FUN_1800daec0(&uStack_1e8,&DAT_182586f78);
          FUN_18048bfa0(uStack_1e8,iVar12);
          uVar13 = FUN_1800dc480(&uStack_1e8);
          FUN_18048bfa0(uVar13,0);
          FUN_1800de7f0(&uStack_1e8,&uStack_3f8);
          FUN_1804f4a50(&uStack_1e8);
          piVar20 = uStack_3f8;
        }
        uStack_3f8 = (int *)0x0;
        puStack_1d8 = (undefined8 *)((ulonglong)puStack_1d8 & 0xffffffffffffff00);
        bVar11 = false;
        piStack_1e0 = piVar20;
        while ((piVar20 != (int *)0x0 && (!bVar11))) {
          uVar13 = FUN_1800daf20(auStack_398,&DAT_182586f88,0xde);
          uVar13 = FUN_1800e2f70(uVar13);
          uVar13 = FUN_18047c970(uVar13,&DAT_1825829c0);
          piVar15 = piVar20;
          if (0xf < *(ulonglong *)(piVar20 + 6)) {
            piVar15 = *(int **)piVar20;
          }
          uVar13 = FUN_180480360(uVar13,piVar15);
          FUN_18047c970(uVar13,&DAT_1825829b8);
          FUN_1800db4f0(auStack_398);
          puStack_1d8 = (undefined8 *)CONCAT71(puStack_1d8._1_7_,1);
          bVar11 = true;
        }
        if (piVar20 == (int *)0x0) {
          uStack_128 = 0;
          uStack_120 = 0;
          uStack_118 = 0;
          uStack_110 = 0;
          uStack_108 = 0;
          uStack_100 = 0;
          uStack_f8 = 0;
          uStack_f0 = 0;
          uStack_e8 = 0;
          uStack_e0 = 0;
          uStack_d8 = 0;
          uStack_d0 = 0;
          FUN_1801caa60(&uStack_128);
          piStack_1e0 = (int *)CONCAT44(piStack_1e0._4_4_,0x2010000);
          puStack_1d8 = &uStack_128;
          piStack_428 = (int *)0x0;
          uVar27 = (undefined4)DAT_18258fb60;
          piStack_1d0 = piVar20;
          FUN_1801d7b80(uStack_3b0,&piStack_1e0,0x15,uVar27);
          uStack_3f8 = (int *)CONCAT44(aiStack_1c8[0],(int)(fVar25 + DAT_18258fae8));
          uStack_188 = 0;
          uStack_180 = 0;
          uStack_178 = 0;
          uStack_170 = 0;
          uStack_168 = 0;
          uStack_160 = 0;
          uStack_158 = 0;
          uStack_150 = 0;
          uStack_148 = 0;
          uStack_140 = 0;
          uStack_138 = 0;
          uStack_130 = 0;
          uVar13 = FUN_1801e79b0(auStack_348,uStack_3f8,0x15);
          FUN_18052a3b0(uVar13,&uStack_188);
          FUN_1801cab00(auStack_278);
          FUN_1801cab00(auStack_2d8);
          FUN_1801cab00(auStack_338);
          FUN_1804c0110(param_6,(lStack_1b8 - lStack_1c0 >> 2) * 0x20 + 1);
          uStack_1e8 = CONCAT44(uStack_1e8._4_4_,0x3f800000);
          FUN_180446990(param_6,&uStack_1e8);
          piVar20 = piVar22;
          if (piVar18 != (int *)0x0) {
            do {
              plStack_420._0_1_ = 1;
              piStack_428 = (int *)(longlong)(int)piVar20;
              piStack_418 = param_6;
              FUN_180531ec0(&uStack_128,&uStack_188,param_3,&puStack_1a8);
              plStack_420 = (longlong *)((ulonglong)plStack_420._1_7_ << 8);
              piStack_428 = (int *)(longlong)(int)piVar20;
              piStack_418 = param_6;
              FUN_180531ec0(&uStack_128,&uStack_188,param_3,&puStack_1a8);
              piVar20 = (int *)((longlong)piVar20 + 1);
            } while (piVar20 < piVar18);
          }
          piStack_1d0 = (int *)0x0;
          piStack_1e0 = (int *)CONCAT44(piStack_1e0._4_4_,0x2010000);
          puStack_1d8 = (undefined8 *)uStack_3a8;
          piStack_428 = (int *)0x0;
          FUN_1801d7b80(&uStack_188,&piStack_1e0,0x10,uVar27);
          FUN_1801cab00(&uStack_188);
          FUN_1801cab00(&uStack_128);
          if (puStack_1a8 != (undefined8 *)0x0) {
            if ((0xfff < ((longlong)puStack_198 - (longlong)puStack_1a8 & 0xfffffffffffffff8U)) &&
               (0x1f < (ulonglong)((longlong)puStack_1a8 + (-8 - puStack_1a8[-1]))))
            goto LAB_18052b83d;
            FUN_1803eb2c8();
            puStack_1a8 = (undefined8 *)0x0;
            puStack_1a0 = (undefined8 *)0x0;
            puStack_198 = (undefined8 *)0x0;
          }
          if (lStack_1c0 == 0) {
LAB_18052b7e8:
            FUN_1803eae20(uStack_c8 ^ (ulonglong)auStack_448);
            return;
          }
          if (((lStack_1b0 - lStack_1c0 & 0xfffffffffffffffcU) < 0x1000) ||
             ((lStack_1c0 - *(longlong *)(lStack_1c0 + -8)) - 8U < 0x20)) {
            FUN_1803eb2c8();
            goto LAB_18052b7e8;
          }
          goto LAB_18052b935;
        }
      }
      else {
        if (uVar19 < 0x2000000000000000) {
          puStack_1a8 = (undefined8 *)FUN_180440bd0(uVar19 * 8);
          puStack_198 = puStack_1a8 + uVar19;
          puStack_1a0 = puStack_1a8;
          do {
            *puStack_1a0 = 0;
            puStack_1a0 = puStack_1a0 + 1;
            uVar19 = uVar19 - 1;
          } while (uVar19 != 0);
          goto LAB_18052b184;
        }
        FUN_180444e60();
LAB_18052b8b4:
        uVar13 = FUN_18048c900(&piStack_1e0);
        FUN_18064aef0(auStack_3f0,uVar13);
        FUN_180409bb8(auStack_3f0,&DAT_182640400);
LAB_18052b8df:
        uVar13 = FUN_18048c900(&piStack_1e0);
        FUN_18064aef0(auStack_3f0,uVar13);
        FUN_180409bb8(auStack_3f0,&DAT_182640400);
      }
      uVar13 = FUN_18048c900(&piStack_1e0);
      FUN_18064aef0(auStack_3f0,uVar13);
      FUN_180409bb8(auStack_3f0,&DAT_182640400);
LAB_18052b935:
      piStack_428 = piVar22;
      FUN_18040e3e4(0,0,0,0);
      pcVar10 = (code *)swi(3);
      (*pcVar10)();
      return;
    }
  }
  else {
LAB_18052b852:
    uVar13 = FUN_18048c900(&piStack_1e0);
    FUN_18064aef0(auStack_3f0,uVar13);
    FUN_180409bb8(auStack_3f0,&DAT_182640400);
  }
  uVar13 = FUN_18048c900(&piStack_1e0);
  FUN_18064aef0(auStack_3f0,uVar13);
  FUN_180409bb8(auStack_3f0,&DAT_182640400);
LAB_18052b8a8:
  FUN_180444e60();
  pcVar10 = (code *)swi(3);
  (*pcVar10)();
  return;
}

