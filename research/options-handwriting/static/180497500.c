
/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_180497500(ulonglong param_1,longlong *param_2,longlong *param_3,longlong param_4,
                  undefined **param_5)

{
  bool bVar1;
  double dVar2;
  ulonglong uVar3;
  undefined8 **ppuVar4;
  undefined **ppuVar5;
  undefined8 uVar6;
  code *pcVar7;
  longlong *plVar8;
  undefined8 *puVar9;
  ulonglong uVar10;
  longlong lVar11;
  longlong lVar12;
  undefined8 ****ppppuVar13;
  ulonglong uVar14;
  longlong lVar15;
  longlong *plVar16;
  ulonglong uVar17;
  longlong lVar18;
  undefined **ppuVar19;
  undefined8 *puVar20;
  undefined8 ***pppuVar21;
  uint uVar22;
  ulonglong uVar23;
  double dVar24;
  double dVar25;
  undefined1 auStack_578 [32];
  undefined8 ***pppuStack_558;
  longlong lStack_550;
  undefined8 *puStack_548;
  undefined **ppuStack_540;
  uint uStack_528;
  longlong lStack_520;
  undefined **ppuStack_518;
  ulonglong uStack_510;
  undefined8 **ppuStack_508;
  ulonglong uStack_500;
  longlong *plStack_4f8;
  undefined **ppuStack_4f0;
  longlong lStack_4e8;
  int iStack_4dc;
  longlong lStack_4d8;
  undefined1 auStack_4d0 [136];
  undefined *apuStack_448 [12];
  longlong *plStack_3e8;
  undefined *puStack_3d8;
  longlong *plStack_3d0;
  undefined **ppuStack_3a0;
  undefined1 auStack_398 [80];
  undefined8 ***pppuStack_348;
  undefined8 *puStack_340;
  undefined8 uStack_338;
  undefined8 ***pppuStack_330;
  undefined8 uStack_328;
  ulonglong uStack_320;
  ulonglong uStack_318;
  longlong lStack_310;
  longlong lStack_308;
  longlong lStack_300;
  undefined8 ***pppuStack_2f8;
  longlong lStack_2f0;
  ulonglong uStack_2e8;
  ulonglong uStack_2e0;
  longlong lStack_2d8;
  longlong lStack_2d0;
  longlong lStack_2c8;
  undefined8 uStack_2c0;
  undefined8 uStack_2b8;
  undefined8 uStack_2b0;
  undefined8 uStack_2a8;
  undefined8 uStack_2a0;
  undefined8 uStack_298;
  longlong lStack_290;
  longlong lStack_288;
  undefined8 uStack_280;
  undefined8 uStack_278;
  undefined8 uStack_270;
  undefined8 uStack_268;
  undefined8 uStack_258;
  undefined8 uStack_250;
  undefined8 uStack_248;
  undefined4 uStack_240;
  undefined8 uStack_238;
  undefined8 uStack_230;
  undefined8 uStack_228;
  undefined8 uStack_220;
  undefined8 uStack_218;
  undefined8 uStack_210;
  undefined8 uStack_208;
  undefined8 uStack_200;
  undefined8 uStack_1f8;
  undefined8 uStack_1f0;
  undefined8 uStack_1e8;
  undefined8 uStack_1e0;
  undefined8 uStack_1d8;
  undefined8 uStack_1d0;
  undefined8 uStack_1c8;
  undefined8 uStack_1c0;
  undefined8 uStack_1b8;
  undefined8 uStack_1b0;
  undefined8 uStack_1a8;
  undefined8 uStack_1a0;
  undefined *puStack_198;
  ulonglong uStack_190;
  undefined1 auStack_188 [272];
  ulonglong uStack_78;
  
  uStack_78 = DAT_18264bfc0 ^ (ulonglong)auStack_578;
  ppuStack_4f0 = param_5;
  uStack_338 = (undefined8 *)((ulonglong)uStack_338 & 0xffffffff00000000);
  uStack_528 = 0;
  lStack_520 = param_4;
  uStack_510 = param_1;
  plStack_4f8 = param_2;
  plStack_3e8 = param_2;
  FUN_1804a0860(&lStack_4e8);
  FUN_180451390(&puStack_198,0);
  puStack_198 = &DAT_182570680;
  ppuVar19 = &puStack_198;
  if (param_5 != (undefined **)0x0) {
    ppuVar19 = param_5;
  }
  uStack_258 = 0;
  uStack_250 = 0;
  uStack_248 = 0;
  uStack_240 = 0;
  uStack_238 = 0;
  uStack_230 = 0;
  uStack_228 = 0;
  uStack_220 = 0;
  uStack_218 = 0;
  uStack_210 = 0;
  uStack_208 = 0;
  uStack_200 = 0;
  uStack_1f8 = 0;
  uStack_1f0 = 0;
  uStack_1e8 = 0;
  uStack_1e0 = 0;
  uStack_1d8 = 0;
  uStack_1d0 = 0;
  uStack_1c8 = 0;
  uStack_1c0 = 0;
  uStack_1b8 = 0;
  uStack_1b0 = 0;
  uStack_1a8 = 0;
  uStack_1a0 = 0;
  uVar23 = param_3[1] - *param_3 >> 3;
  ppuStack_518 = ppuVar19;
  uVar6 = FUN_1800db1f0(&lStack_4d8,1);
  FUN_18045d4c0(uVar6);
  FUN_18048a6b0(auStack_4d0);
  *(undefined **)(auStack_4d0 + (longlong)*(int *)(lStack_4d8 + 4) + -8) = &DAT_1806e7c60;
  *(int *)((longlong)&iStack_4dc + (longlong)*(int *)(lStack_4d8 + 4)) =
       *(int *)(lStack_4d8 + 4) + -0x10;
  apuStack_448[0] = &DAT_1806e62b0;
  FUN_1803e4954(apuStack_448);
  uVar6 = FUN_1800db1f0(&lStack_4d8,1);
  FUN_18045d4c0(uVar6);
  (*(code *)PTR__guard_dispatch_icall_1806db690)();
  FUN_18048a6b0(auStack_4d0);
  *(undefined **)(auStack_4d0 + (longlong)*(int *)(lStack_4d8 + 4) + -8) = &DAT_1806e7c60;
  *(int *)((longlong)&iStack_4dc + (longlong)*(int *)(lStack_4d8 + 4)) =
       *(int *)(lStack_4d8 + 4) + -0x10;
  apuStack_448[0] = &DAT_1806e62b0;
  FUN_1803e4954(apuStack_448);
  uVar3 = _UNK_18258ff38;
  uVar10 = _DAT_18258ff30;
  uVar22 = 0;
  if ((2 < *(int *)(param_4 + 0x68)) && (uVar22 = 0, *(char *)(param_4 + 0x6c) != '\0')) {
    puVar20 = (undefined8 *)(param_4 + 0x30);
    lVar11 = *(longlong *)(param_4 + 0x40);
    if (lVar11 == 0x7fffffffffffffff) goto LAB_180498882;
    uStack_338 = puVar20;
    if (0xf < *(ulonglong *)(param_4 + 0x48)) {
      uStack_338 = (undefined8 *)*puVar20;
    }
    pppuStack_330 = (undefined8 ****)0x0;
    uStack_328 = 0;
    uStack_320 = 0;
    uStack_318 = 0;
    uVar17 = lVar11 + 1;
    uVar14 = 0xf;
    pcVar7 = (code *)&pppuStack_330;
    if (0xf < uVar17) {
      uVar14 = uVar17 | 0xf;
      if (uVar14 < 0x8000000000000000) {
        if (uVar14 < 0x16) {
          uVar14 = 0x16;
        }
      }
      else {
        uVar14 = 0x7fffffffffffffff;
      }
      pcVar7 = (code *)FUN_180440bd0(uVar14 + 1);
      pppuStack_330 = (undefined8 ***)pcVar7;
    }
    uStack_320 = uVar17;
    uStack_318 = uVar14;
    FUN_1806a7f00(pcVar7,uStack_338,lVar11);
    *(undefined1 *)((longlong)pcVar7 + lVar11) = 0x2f;
    *(undefined1 *)((longlong)pcVar7 + uVar17) = 0;
    uStack_528 = 2;
    plVar8 = (longlong *)FUN_180444e80(&pppuStack_330,&DAT_1825830d8,9);
    pppuStack_2f8 = (undefined8 ***)*plVar8;
    lStack_2f0 = plVar8[1];
    puStack_340 = (undefined8 *)plVar8[2];
    uStack_2e0 = plVar8[3];
    param_1 = 0;
    plVar8[2] = 0;
    plVar8[3] = 0xf;
    *(undefined1 *)plVar8 = 0;
    uStack_528 = 6;
    pppuStack_348 = &pppuStack_2f8;
    if (0xf < uStack_2e0) {
      pppuStack_348 = pppuStack_2f8;
    }
    uStack_2e8 = (ulonglong)puStack_340;
    FUN_180440d00(&uStack_278,&pppuStack_348);
    uStack_528 = 0xe;
    pppuStack_348 = (undefined8 ***)0x0;
    puStack_340 = &DAT_18264ccb0;
    FUN_18049e830(&uStack_278,&pppuStack_348);
    if ((int)pppuStack_348 != 0) goto LAB_180498888;
    FUN_180444950(&uStack_278);
    if (0xf < uStack_2e0) {
      if ((uStack_2e0 + 1 < 0x1000) ||
         ((ulonglong)((longlong)pppuStack_2f8 + (-8 - (longlong)pppuStack_2f8[-1])) < 0x20)) {
        FUN_1803eb2c8();
        goto LAB_1804978ee;
      }
      goto LAB_1804988a3;
    }
LAB_1804978ee:
    uStack_2e8 = uVar10;
    uStack_2e0 = uVar3;
    pppuStack_2f8 = (undefined8 ***)((ulonglong)pppuStack_2f8 & 0xffffffffffffff00);
    if (0xf < uStack_318) {
      if ((0xfff < uStack_318 + 1) &&
         (0x1f < (ulonglong)((longlong)pppuStack_330 + (-8 - (longlong)pppuStack_330[-1]))))
      goto LAB_1804988b8;
      FUN_1803eb2c8();
    }
    uStack_320 = uVar10;
    uStack_318 = uVar3;
    pppuStack_330 = (undefined8 ***)((ulonglong)pppuStack_330 & 0xffffffffffffff00);
    lVar11 = *(longlong *)(param_4 + 0x40);
    if (lVar11 == 0x7fffffffffffffff) {
      FUN_180444e40();
LAB_1804988d3:
      FUN_180444210(&DAT_182582bd8,&pppuStack_348,&uStack_278);
LAB_1804988ee:
      pppuStack_558 = (undefined8 ***)uVar17;
      FUN_18040e3e4(0,0,0,0);
    }
    else {
      if (0xf < *(ulonglong *)(param_4 + 0x48)) {
        puVar20 = (undefined8 *)*puVar20;
      }
      pppuStack_330 = (undefined8 ****)0x0;
      uStack_328 = 0;
      uStack_320 = 0;
      uStack_318 = 0;
      uVar17 = lVar11 + 1;
      uVar14 = 0xf;
      pcVar7 = (code *)&pppuStack_330;
      if (0xf < uVar17) {
        uVar14 = uVar17 | 0xf;
        if (uVar14 < 0x8000000000000000) {
          if (uVar14 < 0x16) {
            uVar14 = 0x16;
          }
        }
        else {
          uVar14 = 0x7fffffffffffffff;
        }
        pcVar7 = (code *)FUN_180440bd0(uVar14 + 1);
        pppuStack_330 = (undefined8 ***)pcVar7;
      }
      uStack_320 = uVar17;
      uStack_318 = uVar14;
      FUN_1806a7f00(pcVar7,puVar20,lVar11);
      *(undefined1 *)((longlong)pcVar7 + lVar11) = 0x2f;
      *(undefined1 *)((longlong)pcVar7 + uVar17) = 0;
      uStack_528 = 0x1e;
      plVar8 = (longlong *)FUN_180444e80(&pppuStack_330,&DAT_182583110,0x11);
      pppuStack_2f8 = (undefined8 ***)*plVar8;
      lStack_2f0 = plVar8[1];
      puStack_340 = (undefined8 *)plVar8[2];
      uStack_2e0 = plVar8[3];
      uVar17 = 0;
      plVar8[2] = 0;
      plVar8[3] = 0xf;
      *(undefined1 *)plVar8 = 0;
      uStack_528 = 0x3e;
      pppuStack_348 = &pppuStack_2f8;
      if (0xf < uStack_2e0) {
        pppuStack_348 = pppuStack_2f8;
      }
      uStack_2e8 = (ulonglong)puStack_340;
      FUN_180440d00(&uStack_278,&pppuStack_348);
      uVar22 = 0x7e;
      uStack_338 = (undefined8 *)CONCAT44(uStack_338._4_4_,0x7e);
      uStack_528 = 0x7e;
      pppuStack_348 = (undefined8 ***)0x0;
      puStack_340 = &DAT_18264ccb0;
      FUN_18049e830(&uStack_278,&pppuStack_348);
      if ((int)pppuStack_348 != 0) goto LAB_1804988d3;
      FUN_180444950(&uStack_278);
      if (0xf < uStack_2e0) {
        if ((uStack_2e0 + 1 < 0x1000) ||
           ((ulonglong)((longlong)pppuStack_2f8 + (-8 - (longlong)pppuStack_2f8[-1])) < 0x20)) {
          FUN_1803eb2c8();
          goto LAB_180497b2b;
        }
        goto LAB_1804988ee;
      }
LAB_180497b2b:
      uStack_2e8 = uVar10;
      uStack_2e0 = uVar3;
      pppuStack_2f8 = (undefined8 ***)((ulonglong)pppuStack_2f8 & 0xffffffffffffff00);
      if (uStack_318 < 0x10) {
LAB_180497b7b:
        uStack_320 = uVar10;
        uStack_318 = uVar3;
        pppuStack_330 = (undefined8 ***)((ulonglong)pppuStack_330 & 0xffffffffffffff00);
        ppuVar19 = ppuStack_518;
        param_4 = lStack_520;
        goto LAB_180497b94;
      }
      if ((uStack_318 + 1 < 0x1000) ||
         ((ulonglong)((longlong)pppuStack_330 + (-8 - (longlong)pppuStack_330[-1])) < 0x20)) {
        FUN_1803eb2c8();
        goto LAB_180497b7b;
      }
    }
    pppuStack_558 = (undefined8 ***)uVar17;
    FUN_18040e3e4(0,0,0,0);
LAB_180498918:
    pppuStack_558 = (undefined8 ***)0x0;
    FUN_18040e3e4(0,0,0,0);
LAB_180498931:
    uVar6 = FUN_18048c900(&pppuStack_348);
    FUN_18064aef0(&pppuStack_330,uVar6);
    FUN_180409bb8(&pppuStack_330,&DAT_182640400);
LAB_180498960:
    pppuStack_558 = (undefined8 ***)pcVar7;
    FUN_18040e3e4(0,0,0,0);
    pcVar7 = (code *)swi(3);
    (*pcVar7)();
    return;
  }
LAB_180497b94:
  lStack_290 = 0;
  lStack_288 = 0;
  uStack_280 = 0;
  uStack_2a8 = 0;
  uStack_2a0 = 0;
  uStack_298 = 0;
  uStack_2c0 = 0;
  uStack_2b8 = 0;
  uStack_2b0 = 0;
  uVar17 = (ulonglong)*(uint *)(param_4 + 0x70);
  if ((*(uint *)(param_4 + 0x70) == 0) || (uStack_500 = uVar17, uVar23 <= uVar17)) {
    lStack_2d8 = 0;
    lStack_2d0 = 0;
    param_1 = 0;
    lStack_2c8 = 0;
    if (uVar23 != 0) {
      do {
        pppuStack_348 = *(undefined8 ****)(*param_3 + param_1 * 8);
        FUN_18047d7f0(&lStack_2d8,&pppuStack_348);
        param_1 = param_1 + 1;
      } while (param_1 < uVar23);
    }
    uVar10 = uStack_510;
    puStack_548 = &uStack_258;
    pppuStack_558 = (undefined8 ***)&uStack_2a8;
    lStack_550 = param_4;
    ppuStack_540 = ppuVar19;
    FUN_1804902d0(uStack_510,&lStack_2d8,&lStack_290,&uStack_2c0);
    if (lStack_2d8 == 0) goto LAB_1804985c3;
    if (((lStack_2c8 - lStack_2d8 & 0xfffffffffffffff8U) < 0x1000) ||
       ((lStack_2d8 - *(longlong *)(lStack_2d8 + -8)) - 8U < 0x20)) {
      FUN_1803eb2c8();
      goto LAB_1804985c3;
    }
    pppuStack_558 = (undefined8 ***)0x0;
    FUN_18040e3e4(0,0,0,0);
LAB_180498882:
    FUN_180444e40();
LAB_180498888:
    FUN_180444210(&DAT_182582bd8,&pppuStack_348,&uStack_278);
LAB_1804988a3:
    pppuStack_558 = (undefined8 ***)param_1;
    FUN_18040e3e4(0,0,0,0);
LAB_1804988b8:
    pppuStack_558 = (undefined8 ***)param_1;
    FUN_18040e3e4(0,0,0,0);
    pcVar7 = (code *)swi(3);
    (*pcVar7)();
    return;
  }
  pcVar7 = (code *)0x0;
  if (*(int *)(param_4 + 0x80) == 0) {
    pcVar7 = FUN_180490c80;
  }
  else if (*(int *)(param_4 + 0x80) == 1) {
    pcVar7 = FUN_180490ca0;
  }
  else {
    pppuStack_348 = (undefined8 ***)0x0;
    puStack_340 = (undefined8 *)0x0;
    uStack_338 = (undefined8 *)0x0;
    FUN_1800daec0(&uStack_338,&DAT_18256ea27);
    FUN_18048bfa0(uStack_338,0);
    uVar6 = FUN_1800dc480(&uStack_338);
    FUN_18048bfa0(uVar6,0);
    FUN_1800de7f0(&uStack_338,&ppuStack_508);
    FUN_1804f4a50(&uStack_338);
    ppuVar4 = ppuStack_508;
    lVar11 = lStack_520;
    uStack_528 = uVar22 | 0x380;
    uStack_338 = (undefined8 *)(CONCAT44(uStack_338._4_4_,uVar22) | 0x380);
    ppuStack_508 = (undefined8 ***)0x0;
    pppuStack_348 = (undefined8 ***)ppuVar4;
    puStack_340 = (undefined8 *)((ulonglong)puStack_340 & 0xffffffffffffff00);
    bVar1 = false;
    while (((undefined8 ***)ppuVar4 != (undefined8 ***)0x0 && (!bVar1))) {
      uVar6 = FUN_1800daf20(auStack_398,&DAT_182582de0,0x5ae,2);
      uVar6 = FUN_1800e2f70(uVar6);
      uVar6 = FUN_18047c970(uVar6,&DAT_1825829c0);
      uVar6 = FUN_18047c970(uVar6,&DAT_182583128);
      puVar9 = (undefined8 *)FUN_1804a17d0(&pppuStack_330,*(undefined4 *)(lVar11 + 0x80));
      puVar20 = puVar9 + 2;
      if (0xf < (ulonglong)puVar9[3]) {
        puVar9 = (undefined8 *)*puVar9;
      }
      FUN_180480360(uVar6,puVar9,*puVar20);
      if (0xf < uStack_318) {
        if ((0xfff < uStack_318 + 1) &&
           (0x1f < (ulonglong)((longlong)pppuStack_330 + (-8 - (longlong)pppuStack_330[-1]))))
        goto LAB_180498918;
        FUN_1803eb2c8();
      }
      uStack_320 = uVar10;
      uStack_318 = uVar3;
      pppuStack_330 = (undefined8 ***)((ulonglong)pppuStack_330 & 0xffffffffffffff00);
      FUN_1800db4f0(auStack_398);
      puStack_340 = (undefined8 *)CONCAT71(puStack_340._1_7_,1);
      bVar1 = true;
    }
    if ((undefined8 ***)ppuVar4 != (undefined8 ***)0x0) goto LAB_180498931;
  }
  FUN_180485630(*param_3,param_3[1],param_3[1] - *param_3 >> 3,pcVar7);
  pppuStack_2f8 = (undefined8 ****)0x0;
  lStack_2f0 = 0;
  uStack_2e8 = 0;
  puStack_3d8 = &DAT_182572258;
  ppuStack_3a0 = &puStack_3d8;
  plStack_3d0 = param_3;
  FUN_1804a4c90(&pppuStack_2f8,param_3,&puStack_3d8);
  if (ppuStack_3a0 != (undefined **)0x0) {
    (*(code *)PTR__guard_dispatch_icall_1806db690)
              (ppuStack_3a0,
               CONCAT71((int7)((ulonglong)&puStack_3d8 >> 8),ppuStack_3a0 != &puStack_3d8));
  }
  pcVar7 = (code *)0x0;
  lStack_310 = 0;
  lStack_308 = 0;
  lStack_300 = 0;
  pppuStack_330 = (undefined8 ***)0x0;
  uStack_328 = 0;
  uStack_320 = 0;
  uStack_318 = 0;
  pppuStack_330 = (undefined8 ***)FUN_180440bd0(0x20);
  pppuVar21 = _UNK_1825830f8;
  uStack_320 = _DAT_182590130;
  uStack_318 = _UNK_182590138;
  *pppuStack_330 = _DAT_1825830f0;
  ((undefined8 ****)pppuStack_330)[1] = pppuVar21;
  ((undefined8 ****)pppuStack_330)[2] = DAT_182583100;
  *(undefined4 *)((undefined8 ****)pppuStack_330 + 3) = DAT_182583108;
  *(undefined2 *)((longlong)pppuStack_330 + 0x1c) = DAT_18258310c;
  *(undefined1 *)((longlong)pppuStack_330 + 0x1e) = DAT_18258310e;
  *(undefined1 *)((longlong)pppuStack_330 + 0x1f) = 0;
  dVar24 = (double)FUN_18063e480();
  if (0xf < uStack_318) {
    if ((0xfff < uStack_318 + 1) &&
       (0x1f < (ulonglong)((longlong)pppuStack_330 + (-8 - (longlong)pppuStack_330[-1]))))
    goto LAB_180498960;
    FUN_1803eb2c8();
  }
  uStack_320 = uVar10;
  uStack_318 = uVar3;
  pppuStack_330 = (undefined8 ***)((ulonglong)pppuStack_330 & 0xffffffffffffff00);
  dVar25 = (double)FUN_180414b10((double)uVar17 * dVar24);
  dVar2 = DAT_18258fc98;
  lVar11 = 0;
  if ((DAT_18258fc98 <= dVar25) && (dVar25 = dVar25 - DAT_18258fc98, dVar25 < DAT_18258fc98)) {
    lVar11 = -0x8000000000000000;
  }
  uVar10 = 4;
  if (4 < (ulonglong)((longlong)dVar25 + lVar11)) {
    uVar10 = (longlong)dVar25 + lVar11;
  }
  uVar6 = FUN_1800db1f0(&lStack_4d8,1);
  FUN_18045d4c0(uVar6);
  FUN_18048a6b0(auStack_4d0);
  *(undefined **)(auStack_4d0 + (longlong)*(int *)(lStack_4d8 + 4) + -8) = &DAT_1806e7c60;
  *(int *)((longlong)&iStack_4dc + (longlong)*(int *)(lStack_4d8 + 4)) =
       *(int *)(lStack_4d8 + 4) + -0x10;
  apuStack_448[0] = &DAT_1806e62b0;
  FUN_1803e4954(apuStack_448);
  if ((ulonglong)(lStack_300 - lStack_310 >> 3) < uVar10) {
    if (0x1fffffffffffffff < uVar10) {
      FUN_180444e60();
      pcVar7 = (code *)swi(3);
      (*pcVar7)();
      return;
    }
    lVar15 = lStack_308 - lStack_310;
    lVar11 = FUN_180440bd0(uVar10 * 8);
    FUN_1806a7f00(lVar11,lStack_310,lStack_308 - lStack_310);
    if (lStack_310 != 0) {
      if ((0xfff < (lStack_300 - lStack_310 & 0xfffffffffffffff8U)) &&
         (0x1f < (lStack_310 - *(longlong *)(lStack_310 + -8)) - 8U)) {
        pppuStack_558 = (undefined8 ***)0x0;
        FUN_18040e3e4(0,0,0,0);
LAB_180498990:
        pppuStack_558 = (undefined8 ***)0x0;
        FUN_18040e3e4(0,0,0,0);
        pcVar7 = (code *)swi(3);
        (*pcVar7)();
        return;
      }
      FUN_1803eb2c8();
    }
    lStack_308 = lVar11 + (lVar15 >> 3) * 8;
    lStack_300 = lVar11 + uVar10 * 8;
    uVar17 = uStack_500;
    lStack_310 = lVar11;
  }
  ppuVar19 = ppuStack_518;
  param_4 = lStack_520;
  uVar10 = uStack_510;
  if ((ulonglong)((lStack_288 - lStack_290 >> 4) * -0x71c71c71c71c71c7) < uVar17) {
LAB_1804980e3:
    lVar11 = lStack_310;
    if (lStack_310 != lStack_308) {
      lStack_308 = lStack_310;
    }
    lVar15 = lStack_308;
    pppuStack_330 = (undefined8 ****)0x0;
    uStack_328 = 0;
    uStack_320 = 0;
    lStack_2d8 = 0;
    lStack_2d0 = 0;
    lStack_2c8 = 0;
    uStack_278 = 0;
    uStack_270 = 0;
    uStack_268 = 0;
    dVar25 = (double)FUN_180414b10((double)(uVar17 + (lStack_288 - lStack_290 >> 4) *
                                                     0x71c71c71c71c71c7));
    dVar25 = dVar25 * dVar24;
    lVar12 = 0;
    if ((dVar2 <= dVar25) && (dVar25 = dVar25 - dVar2, dVar25 < dVar2)) {
      lVar12 = -0x8000000000000000;
    }
    pppuVar21 = (undefined8 ***)0x4;
    if ((undefined8 ***)0x4 < (undefined8 ***)((longlong)dVar25 + lVar12)) {
      pppuVar21 = (undefined8 ***)((longlong)dVar25 + lVar12);
    }
    pppuStack_348 = pppuVar21;
    if (pcVar7 < (undefined8 ****)
                 ((lStack_2f0 - (longlong)pppuStack_2f8 >> 3) * -0x5555555555555555)) {
      lVar18 = (longlong)pcVar7 * 0x18;
      ppppuVar13 = (undefined8 ****)pppuStack_2f8;
      lVar12 = lStack_2f0;
      do {
        uVar17 = uStack_500;
        if (pppuVar21 <= (undefined8 ***)(lVar15 - lVar11 >> 3)) break;
        plVar8 = *(longlong **)((longlong)ppppuVar13 + lVar18 + 8);
        plVar16 = *(longlong **)((longlong)ppppuVar13 + lVar18);
        for (; uStack_500 = uVar17, plVar16 != plVar8; plVar16 = plVar16 + 1) {
          ppuStack_508 = *(undefined8 ***)(*param_3 + *plVar16 * 8);
          FUN_18047d7f0(&lStack_310,&ppuStack_508);
          ppppuVar13 = (undefined8 ****)pppuStack_2f8;
          lVar12 = lStack_2f0;
          pppuVar21 = pppuStack_348;
          lVar11 = lStack_310;
          lVar15 = lStack_308;
          uVar17 = uStack_500;
        }
        pcVar7 = (code *)((longlong)pcVar7 + 1);
        lVar18 = lVar18 + 0x18;
      } while (pcVar7 < (undefined8 ****)
                        ((lVar12 - (longlong)ppppuVar13 >> 3) * -0x5555555555555555));
    }
    lVar12 = lStack_2d0;
    uVar10 = uStack_510;
    ppuVar19 = ppuStack_518;
    param_4 = lStack_520;
    if (lVar11 != lVar15) goto code_r0x000180498285;
    if (lStack_2d8 != 0) {
      if (lStack_2d8 != lStack_2d0) {
        lVar11 = lStack_2d8 + 0x40;
        do {
          if (*(longlong *)(lVar11 + 0x40) != 0) {
            (*(code *)PTR__guard_dispatch_icall_1806db690)(*(longlong *)(lVar11 + 0x40),1);
          }
          FUN_1804446b0(lVar11);
          FUN_180444580(lVar11 + -0x18);
          lVar15 = lVar11 + 0x50;
          lVar11 = lVar11 + 0x90;
        } while (lVar15 != lVar12);
      }
      if ((0xfff < (ulonglong)((lStack_2c8 - lStack_2d8 >> 4) * 0x10)) &&
         (0x1f < (lStack_2d8 - *(longlong *)(lStack_2d8 + -8)) - 8U)) goto LAB_180498990;
      FUN_1803eb2c8();
      ppuVar19 = ppuStack_518;
      param_4 = lStack_520;
      uVar10 = uStack_510;
    }
  }
LAB_1804984c4:
  FUN_180442d40(&lStack_310);
  FUN_18048abb0(&pppuStack_2f8);
LAB_1804985c3:
  ppuVar5 = ppuStack_4f0;
  pppuStack_558 = (undefined8 ***)ppuStack_4f0;
  uVar6 = (*(code *)PTR__guard_dispatch_icall_1806db690)(uVar10,param_3,&lStack_290,param_4);
  plVar8 = plStack_4f8;
  FUN_1804943d0(uVar6,plStack_4f8,&lStack_290);
  uStack_528 = (uint)uStack_338 | 1;
  *(int *)((longlong)ppuVar19 + 0x104) =
       (int)(*(longlong *)(*plVar8 + 0x28) - *(longlong *)(*plVar8 + 0x20) >> 4) * 0x38e38e39;
  uVar6 = (*(code *)PTR__guard_dispatch_icall_1806db690)();
  FUN_180494230(uVar10,*plVar8 + 0x20,uVar6);
  if (1 < *(int *)(param_4 + 0x68)) {
    lVar11 = *plVar8;
    puVar20 = (undefined8 *)(lVar11 + 0x50);
    if (puVar20 != &uStack_2a8) {
      FUN_1804444e0(puVar20);
      *puVar20 = uStack_2a8;
      *(undefined8 *)(lVar11 + 0x58) = uStack_2a0;
      *(undefined8 *)(lVar11 + 0x60) = uStack_298;
      uStack_2a8 = 0;
      uStack_2a0 = 0;
      uStack_298 = 0;
    }
    lVar11 = *plVar8;
    puVar20 = (undefined8 *)(lVar11 + 0x38);
    if (puVar20 != &uStack_2c0) {
      FUN_1804444e0(puVar20);
      *puVar20 = uStack_2c0;
      *(undefined8 *)(lVar11 + 0x40) = uStack_2b8;
      *(undefined8 *)(lVar11 + 0x48) = uStack_2b0;
      uStack_2c0 = 0;
      uStack_2b8 = 0;
      uStack_2b0 = 0;
    }
  }
  uVar6 = FUN_1800db1f0(&lStack_4d8,1);
  FUN_18045d4c0(uVar6);
  FUN_18048a6b0(auStack_4d0);
  *(undefined **)(auStack_4d0 + (longlong)*(int *)(lStack_4d8 + 4) + -8) = &DAT_1806e7c60;
  *(int *)((longlong)&iStack_4dc + (longlong)*(int *)(lStack_4d8 + 4)) =
       *(int *)(lStack_4d8 + 4) + -0x10;
  apuStack_448[0] = &DAT_1806e62b0;
  FUN_1803e4954(apuStack_448);
  if (ppuVar5 != (undefined **)0x0) {
    plVar8 = (longlong *)FUN_1804a0860(&plStack_4f8);
    *(float *)(ppuVar19 + 0x21) = (float)(*plVar8 - lStack_4e8) / DAT_18258fcac;
    lVar11 = FUN_1804a0810(ppuVar19);
    *(undefined4 *)(lVar11 + 0x10) = *(undefined4 *)(ppuVar19 + 0x21);
    uVar6 = FUN_1804a0810(ppuVar19);
    FUN_180490ec0(uVar6,&uStack_258);
  }
  FUN_1804444e0(&uStack_2c0);
  FUN_1804444e0(&uStack_2a8);
  FUN_1804444e0(&lStack_290);
  puStack_198 = &DAT_182570680;
  if ((uStack_190 & 1) == 0) {
    uVar10 = uStack_190 & 0xfffffffffffffffc;
  }
  else {
    uVar10 = FUN_18044e950(&uStack_190);
  }
  if (uVar10 == 0) {
    FUN_180466130(&puStack_198);
  }
  else {
    FUN_18045d3d0(auStack_188);
  }
  if (((uStack_190 & 2) != 0) && (lVar11 = uStack_190 - 2, lVar11 != 0)) {
    FUN_18009de70(lVar11);
    FUN_1803eb2c8(lVar11,0x28);
  }
  FUN_1803eae20(uStack_78 ^ (ulonglong)auStack_578);
  return;
code_r0x000180498285:
  ppuStack_540 = ppuStack_518;
  puStack_548 = &uStack_258;
  lStack_550 = lStack_520;
  pppuStack_558 = (undefined8 ***)&uStack_278;
  FUN_1804902d0(uStack_510,&lStack_310,&pppuStack_330,&lStack_2d8);
  FUN_18047fbc0(&lStack_290,lStack_288);
  if (1 < *(int *)(param_4 + 0x68)) {
    FUN_18047fbc0(&uStack_2c0,uStack_2b8,lStack_2d8,
                  (lStack_2d0 - lStack_2d8 >> 4) * -0x71c71c71c71c71c7);
    FUN_18047fbc0(&uStack_2a8,uStack_2a0);
  }
  FUN_1804444e0(&uStack_278);
  FUN_1804444e0(&lStack_2d8);
  FUN_1804444e0(&pppuStack_330);
  if (uVar17 <= (ulonglong)((lStack_288 - lStack_290 >> 4) * -0x71c71c71c71c71c7))
  goto LAB_1804984c4;
  goto LAB_1804980e3;
}

