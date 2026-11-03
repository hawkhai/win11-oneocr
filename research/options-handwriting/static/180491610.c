// 180491610 FUN_180491610

/* WARNING: Type propagation algorithm not settling */

void FUN_180491610(ulonglong *param_1,longlong *param_2,longlong *param_3,ulonglong param_4)

{
  int *piVar1;
  uint *puVar2;
  float *pfVar3;
  undefined4 uVar4;
  code *pcVar5;
  bool bVar6;
  double dVar7;
  float fVar8;
  float fVar9;
  char cVar10;
  uint uVar11;
  longlong *plVar12;
  ulonglong *puVar13;
  undefined4 *puVar14;
  undefined8 uVar15;
  undefined8 *puVar16;
  ulonglong *puVar17;
  ulonglong *puVar18;
  longlong *plVar19;
  ulonglong *puVar20;
  longlong lVar21;
  undefined8 **ppuVar22;
  char cVar23;
  longlong *plVar24;
  undefined *puVar25;
  undefined8 *puVar26;
  ulonglong uVar27;
  uint uVar28;
  ulonglong uVar29;
  ulonglong *puVar30;
  ulonglong uVar31;
  uint uVar32;
  ulonglong uVar33;
  longlong lVar34;
  ulonglong uVar35;
  undefined4 extraout_XMM0_Da;
  float fVar36;
  float fVar37;
  float fVar38;
  float fVar39;
  float fVar40;
  float fVar41;
  float fVar42;
  float fVar43;
  float fVar44;
  undefined1 auStack_6a8 [32];
  ulonglong **ppuStack_688;
  ulonglong *puStack_680;
  ulonglong uStack_678;
  char cStack_668;
  char acStack_667 [3];
  uint uStack_664;
  char cStack_660;
  ulonglong uStack_658;
  ulonglong *puStack_650;
  ulonglong *puStack_648;
  ulonglong *puStack_640;
  ulonglong *puStack_638;
  ulonglong *puStack_630;
  ulonglong *puStack_628;
  ulonglong *puStack_620;
  ulonglong *puStack_618;
  ulonglong *puStack_610;
  longlong *plStack_608;
  ulonglong *puStack_600;
  ulonglong *apuStack_5f8 [2];
  ulonglong *apuStack_5e8 [2];
  ulonglong uStack_5d8;
  longlong lStack_5d0;
  undefined4 *puStack_5c8;
  undefined4 *puStack_5c0;
  ulonglong *apuStack_5b8 [2];
  longlong lStack_5a8;
  longlong lStack_5a0;
  longlong lStack_598;
  undefined8 uStack_590;
  undefined4 uStack_588;
  undefined4 uStack_584;
  undefined4 uStack_580;
  undefined4 uStack_57c;
  ulonglong *puStack_578;
  longlong *plStack_570;
  undefined8 **ppuStack_568;
  ulonglong *puStack_560;
  longlong lStack_558;
  uint uStack_540;
  ulonglong **ppuStack_538;
  undefined8 uStack_530;
  ulonglong uStack_528;
  ulonglong *puStack_520;
  ulonglong *puStack_518;
  ulonglong *apuStack_510 [2];
  ulonglong *puStack_500;
  ulonglong *apuStack_4f8 [2];
  ulonglong *puStack_4e8;
  ulonglong *puStack_4e0;
  ulonglong *puStack_4d8;
  ulonglong *puStack_4d0;
  ulonglong *puStack_4c8;
  ulonglong *apuStack_4c0 [2];
  char acStack_4b0 [8];
  ulonglong uStack_4a8;
  ulonglong *puStack_4a0;
  ulonglong *puStack_498;
  undefined8 *puStack_490;
  undefined8 *puStack_488;
  undefined8 *puStack_480;
  undefined8 uStack_478;
  undefined8 uStack_470;
  undefined4 uStack_468;
  ulonglong uStack_460;
  ulonglong *puStack_458;
  ulonglong *puStack_450;
  undefined8 uStack_448;
  undefined8 *puStack_440;
  undefined8 *puStack_438;
  undefined8 *puStack_430;
  undefined8 uStack_428;
  undefined8 uStack_420;
  undefined8 uStack_418;
  undefined8 uStack_410;
  undefined8 uStack_408;
  undefined8 uStack_400;
  longlong lStack_3f8;
  undefined8 uStack_3f0;
  undefined8 uStack_3e8;
  undefined8 uStack_3e0;
  longlong lStack_3d8;
  undefined8 uStack_3d0;
  undefined8 uStack_3c8;
  undefined8 uStack_3c0;
  undefined *puStack_3b8;
  undefined1 auStack_3b0 [8];
  uint auStack_3a8 [14];
  longlong alStack_370 [11];
  undefined4 uStack_314;
  undefined *puStack_310;
  undefined8 uStack_308;
  undefined8 uStack_300;
  undefined4 uStack_2f8;
  undefined8 uStack_2f0;
  undefined8 uStack_2e8;
  undefined8 uStack_2e0;
  undefined8 uStack_2d8;
  undefined8 uStack_2d0;
  undefined8 uStack_2c8;
  undefined8 uStack_2c0;
  undefined1 uStack_2b8;
  undefined1 auStack_2a8 [416];
  longlong lStack_108;
  longlong lStack_f8;
  ulonglong uStack_e8;
  
  uStack_e8 = DAT_18264bfc0 ^ (ulonglong)auStack_6a8;
  uVar31 = 0;
  uStack_664 = 0;
  uStack_540 = 0;
  puStack_650 = (ulonglong *)0x0;
  puStack_648 = (ulonglong *)0x0;
  puStack_640 = (ulonglong *)0x0;
  puStack_638 = (ulonglong *)0x0;
  puStack_630 = (ulonglong *)0x0;
  puStack_628 = (ulonglong *)0x0;
  puStack_620 = (ulonglong *)0x0;
  puStack_618 = (ulonglong *)0x0;
  puStack_610 = (ulonglong *)0x0;
  uStack_4a8 = 0;
  puStack_4a0 = (ulonglong *)0x0;
  puStack_498 = (ulonglong *)0x0;
  uVar33 = param_2[1] - *param_2 >> 3;
  puStack_440 = (undefined8 *)0x0;
  puStack_438 = (undefined8 *)0x0;
  puStack_430 = (undefined8 *)0x0;
  plStack_608 = param_2;
  uStack_5d8 = param_4;
  plStack_570 = param_3;
  puStack_560 = param_1;
  puStack_500 = param_1;
  if (uVar33 != 0) {
    if (uVar33 < 0xaaaaaaaaaaaaaab) {
      puStack_440 = (undefined8 *)FUN_180440bd0(uVar33 * 0x18);
      puStack_430 = puStack_440 + uVar33 * 3;
      puStack_438 = puStack_440;
      do {
        *puStack_438 = 0;
        puStack_438[1] = 0;
        puStack_438[2] = 0;
        puStack_438 = puStack_438 + 3;
        uVar33 = uVar33 - 1;
      } while (uVar33 != 0);
      goto LAB_18049175c;
    }
LAB_180493690:
    FUN_180444e60();
LAB_180493696:
    FUN_180444e60();
LAB_18049369c:
    FUN_180444e60();
LAB_1804936a2:
    ppuStack_688 = (ulonglong **)uVar31;
    FUN_18040e3e4(0,0,0,0);
    pcVar5 = (code *)swi(3);
    (*pcVar5)();
    return;
  }
LAB_18049175c:
  uStack_658 = uStack_658 & 0xffffffff00000000;
  uVar33 = param_2[1] - *param_2 >> 3;
  lStack_5a8 = 0;
  lStack_5a0 = 0;
  lStack_598 = 0;
  if (uVar33 != 0) {
    if (0x3fffffffffffffff < uVar33) goto LAB_180493696;
    lStack_5a8 = FUN_180440bd0(uVar33 * 4);
    lStack_598 = lStack_5a8 + uVar33 * 4;
    lStack_5a0 = lStack_5a8;
    lStack_5a0 = FUN_180485f70(lStack_5a8,uVar33,&uStack_658);
  }
  lVar34 = param_3[1] - *param_3 >> 4;
  uVar33 = lVar34 * -0x71c71c71c71c71c7;
  puStack_490 = (undefined8 *)0x0;
  puStack_488 = (undefined8 *)0x0;
  puStack_480 = (undefined8 *)0x0;
  if (uVar33 != 0) {
    if (0x7ffffffffffffff < uVar33) goto LAB_18049369c;
    puStack_490 = (undefined8 *)FUN_180440bd0(lVar34 * -0x38e38e38e38e38e0);
    puStack_480 = puStack_490 + lVar34 * -0x71c71c71c71c71c;
    puStack_488 = puStack_490;
    do {
      *puStack_488 = 0;
      puStack_488[1] = 0;
      puStack_488[2] = 0;
      puStack_488[3] = 0xf;
      *(undefined1 *)puStack_488 = 0;
      puStack_488 = puStack_488 + 4;
      uVar33 = uVar33 - 1;
    } while (uVar33 != 0);
  }
  plVar12 = plStack_570;
  uStack_658 = 0;
  lVar34 = *param_3;
  uVar33 = uVar31;
  uVar29 = uVar31;
  if ((param_3[1] - lVar34 >> 4) * -0x71c71c71c71c71c7 != 0) {
    do {
      lVar21 = *(longlong *)(lVar34 + 0x48 + uVar33);
      for (lVar34 = *(longlong *)(lVar34 + 0x40 + uVar33); lVar34 != lVar21; lVar34 = lVar34 + 0xe0)
      {
        plVar19 = (longlong *)(lVar34 + 0x80);
        if (0xf < *(ulonglong *)(lVar34 + 0x98)) {
          plVar19 = (longlong *)*plVar19;
        }
        FUN_180444e80((undefined1 *)((longlong)puStack_490 + uVar29),plVar19,
                      *(undefined8 *)(lVar34 + 0x90));
      }
      uStack_658 = uStack_658 + 1;
      lVar34 = *plVar12;
      uVar33 = uVar33 + 0x90;
      uVar29 = uVar29 + 0x20;
    } while (uStack_658 < (ulonglong)((plVar12[1] - lVar34 >> 4) * -0x71c71c71c71c71c7));
    uVar33 = (ulonglong)uStack_664;
  }
  uVar28 = (uint)uVar33;
  plVar12 = (longlong *)param_2[1];
  plVar19 = (longlong *)*param_2;
  uVar27 = uVar31;
  uVar29 = uVar31;
  plVar24 = plVar19;
  if ((longlong)plVar12 - (longlong)plVar19 >> 3 != 0) {
    do {
      puVar13 = puStack_630;
      puVar30 = puStack_648;
      uVar28 = (uint)uVar33;
      if (*(char *)(plVar19[uVar27] + 0x15a) == '\0') {
        if (puStack_630 == puStack_628) {
          lVar34 = (longlong)puStack_630 - (longlong)puStack_638 >> 3;
          if (lVar34 == 0x1fffffffffffffff) goto LAB_1804936d2;
          apuStack_5e8[0] = (ulonglong *)(lVar34 + 1);
          uVar33 = (longlong)puStack_628 - (longlong)puStack_638 >> 3;
          if (0x1fffffffffffffff - (uVar33 >> 1) < uVar33) {
            puVar20 = (ulonglong *)0x1fffffffffffffff;
          }
          else {
            puVar30 = (ulonglong *)(uVar33 + (uVar33 >> 1));
            puVar20 = apuStack_5e8[0];
            if (apuStack_5e8[0] <= puVar30) {
              puVar20 = puVar30;
            }
            if ((ulonglong *)0x1fffffffffffffff < puVar20) goto LAB_1804936cc;
          }
          apuStack_5f8[0] = (ulonglong *)((longlong)puVar20 * 8);
          puVar30 = (ulonglong *)FUN_180440bd0(apuStack_5f8[0]);
          puVar30[lVar34] = uVar27;
          if (puVar13 == puStack_630) {
            lVar21 = (longlong)puStack_630 - (longlong)puStack_638;
            puVar20 = puVar30;
            puVar13 = puStack_638;
          }
          else {
            FUN_1806a7f00(puVar30,puStack_638,(longlong)puVar13 - (longlong)puStack_638);
            lVar21 = (longlong)puStack_630 - (longlong)puVar13;
            puVar20 = puVar30 + lVar34 + 1;
          }
          FUN_1806a7f00(puVar20,puVar13,lVar21);
          if (puStack_638 != (ulonglong *)0x0) {
            if ((0xfff < ((longlong)puStack_628 - (longlong)puStack_638 & 0xfffffffffffffff8U)) &&
               (0x1f < (longlong)puStack_638 + (-8 - puStack_638[-1]))) goto LAB_1804936b7;
            FUN_1803eb2c8();
          }
          puStack_630 = puVar30 + (longlong)apuStack_5e8[0];
          puStack_628 = (ulonglong *)((longlong)apuStack_5f8[0] + (longlong)puVar30);
          puStack_638 = puVar30;
        }
        else {
          *puStack_630 = uVar27;
          puStack_630 = puStack_630 + 1;
        }
        plVar12 = (longlong *)FUN_18063bfd0(&puStack_4e8,puStack_490 + uVar27 * 4);
        uStack_664 = uVar28 | 8;
        if ((ulonglong)(plVar12[1] - *plVar12 >> 2) < 2) {
LAB_180491f3f:
          uVar33 = (ulonglong)uStack_664;
LAB_180491f43:
          bVar6 = false;
        }
        else {
          lVar34 = *(longlong *)(*param_2 + uVar27 * 8);
          if (*(longlong *)(lVar34 + 0x150) != 0) {
            LOCK();
            piVar1 = (int *)(*(longlong *)(lVar34 + 0x150) + 8);
            *piVar1 = *piVar1 + 1;
            UNLOCK();
          }
          apuStack_5b8[0] = *(ulonglong **)(lVar34 + 0x150);
          uStack_664 = uVar28 | 0x18;
          if ((ulonglong)
              (*(longlong *)(*(longlong *)(lVar34 + 0x148) + 0x48) -
               *(longlong *)(*(longlong *)(lVar34 + 0x148) + 0x40) >> 3) < 3) goto LAB_180491f3f;
          lVar34 = *(longlong *)(*param_2 + uVar27 * 8);
          if (*(longlong *)(lVar34 + 0x150) != 0) {
            LOCK();
            piVar1 = (int *)(*(longlong *)(lVar34 + 0x150) + 8);
            *piVar1 = *piVar1 + 1;
            UNLOCK();
          }
          puStack_600 = *(ulonglong **)(lVar34 + 0x150);
          uStack_664 = uVar28 | 0x38;
          uVar33 = (ulonglong)uStack_664;
          if (*(char *)(*(longlong *)(lVar34 + 0x148) + 1) != '\0') goto LAB_180491f43;
          bVar6 = true;
        }
        puVar30 = apuStack_5b8[0];
        if ((uVar33 & 0x20) != 0) {
          uStack_664 = (uint)uVar33 & 0xffffffdf;
          uVar33 = (ulonglong)uStack_664;
          if (puStack_600 != (ulonglong *)0x0) {
            FUN_1804474f0(puStack_600);
          }
        }
        if ((uVar33 & 0x10) != 0) {
          uStack_664 = (uint)uVar33 & 0xffffffef;
          uVar33 = (ulonglong)uStack_664;
          if (puVar30 != (ulonglong *)0x0) {
            FUN_1804474f0(puVar30);
          }
        }
        if ((uVar33 & 8) != 0) {
          uStack_664 = (uint)uVar33 & 0xfffffff7;
          uVar33 = (ulonglong)uStack_664;
          FUN_1804442f0(&puStack_4e8);
        }
        puVar30 = puStack_4a0;
        if (bVar6) {
          if (puStack_4a0 == puStack_498) {
            lVar34 = (longlong)((longlong)puStack_4a0 - uStack_4a8) >> 3;
            if (lVar34 == 0x1fffffffffffffff) goto LAB_1804936d2;
            apuStack_5f8[0] = (ulonglong *)(lVar34 + 1);
            uVar35 = (longlong)((longlong)puStack_498 - uStack_4a8) >> 3;
            if (0x1fffffffffffffff - (uVar35 >> 1) < uVar35) {
              puVar20 = (ulonglong *)0x1fffffffffffffff;
            }
            else {
              puVar13 = (ulonglong *)(uVar35 + (uVar35 >> 1));
              puVar20 = apuStack_5f8[0];
              if (apuStack_5f8[0] <= puVar13) {
                puVar20 = puVar13;
              }
              if ((ulonglong *)0x1fffffffffffffff < puVar20) goto LAB_1804936cc;
            }
            apuStack_4c0[0] = (ulonglong *)FUN_180440bd0((longlong)puVar20 * 8);
            puStack_4d0 = apuStack_4c0[0] + lVar34;
            apuStack_5e8[0] = puStack_4d0 + 1;
            puStack_4e8 = &uStack_4a8;
            *puStack_4d0 = uVar27;
            puStack_4d8 = puVar20;
            puStack_4c8 = apuStack_5e8[0];
            if (puVar30 == puStack_4a0) {
              FUN_1806a7f00(apuStack_4c0[0],uStack_4a8,(longlong)puStack_4a0 - uStack_4a8);
              puVar13 = apuStack_4c0[0];
            }
            else {
              FUN_1806a7f00(apuStack_4c0[0],uStack_4a8,(longlong)puVar30 - uStack_4a8);
              puVar13 = apuStack_4c0[0];
              puStack_4d0 = apuStack_4c0[0];
              FUN_1806a7f00(apuStack_5e8[0],puVar30,(longlong)puStack_4a0 - (longlong)puVar30);
            }
            puStack_4e0 = (ulonglong *)0x0;
            FUN_18049a970(&uStack_4a8,puVar13,apuStack_5f8[0],puVar20);
            FUN_18048ba20(&puStack_4e8);
          }
          else {
            *puStack_4a0 = uVar27;
            puStack_4a0 = puStack_4a0 + 1;
          }
        }
      }
      else {
        if (puStack_648 == puStack_640) {
          lVar34 = (longlong)puStack_648 - (longlong)puStack_650 >> 3;
          if (lVar34 != 0x1fffffffffffffff) {
            apuStack_5e8[0] = (ulonglong *)(lVar34 + 1);
            uVar33 = (longlong)puStack_640 - (longlong)puStack_650 >> 3;
            if (0x1fffffffffffffff - (uVar33 >> 1) < uVar33) {
              puVar20 = (ulonglong *)0x1fffffffffffffff;
            }
            else {
              puVar13 = (ulonglong *)((uVar33 >> 1) + uVar33);
              puVar20 = apuStack_5e8[0];
              if (apuStack_5e8[0] <= puVar13) {
                puVar20 = puVar13;
              }
              if ((ulonglong *)0x1fffffffffffffff < puVar20) goto LAB_1804936cc;
            }
            apuStack_5f8[0] = (ulonglong *)((longlong)puVar20 * 8);
            puVar13 = (ulonglong *)FUN_180440bd0(apuStack_5f8[0]);
            puVar13[lVar34] = uVar27;
            if (puVar30 == puStack_648) {
              lVar21 = (longlong)puStack_648 - (longlong)puStack_650;
              puVar20 = puVar13;
              puVar30 = puStack_650;
            }
            else {
              FUN_1806a7f00(puVar13,puStack_650,(longlong)puVar30 - (longlong)puStack_650);
              lVar21 = (longlong)puStack_648 - (longlong)puVar30;
              puVar20 = puVar13 + lVar34 + 1;
            }
            FUN_1806a7f00(puVar20,puVar30,lVar21);
            if (puStack_650 != (ulonglong *)0x0) {
              if ((0xfff < ((longlong)puStack_640 - (longlong)puStack_650 & 0xfffffffffffffff8U)) &&
                 (0x1f < (longlong)puStack_650 + (-8 - puStack_650[-1]))) goto LAB_1804936b7;
              FUN_1803eb2c8();
            }
            puStack_648 = puVar13 + (longlong)apuStack_5e8[0];
            puStack_640 = (ulonglong *)((longlong)apuStack_5f8[0] + (longlong)puVar13);
            puStack_650 = puVar13;
            goto LAB_180491aac;
          }
LAB_1804936d2:
          FUN_180444e60();
          goto LAB_1804936d8;
        }
        *puStack_648 = uVar27;
        puStack_648 = puStack_648 + 1;
LAB_180491aac:
        plVar12 = (longlong *)FUN_18063bfd0(&puStack_4e8,puStack_490 + uVar27 * 4);
        uStack_664 = uVar28 | 1;
        if ((ulonglong)(plVar12[1] - *plVar12 >> 2) < 2) {
LAB_180491b6a:
          uVar33 = (ulonglong)uStack_664;
LAB_180491b6f:
          bVar6 = false;
        }
        else {
          lVar34 = *(longlong *)(*param_2 + uVar27 * 8);
          if (*(longlong *)(lVar34 + 0x150) != 0) {
            LOCK();
            piVar1 = (int *)(*(longlong *)(lVar34 + 0x150) + 8);
            *piVar1 = *piVar1 + 1;
            UNLOCK();
          }
          apuStack_510[0] = *(ulonglong **)(lVar34 + 0x150);
          uStack_664 = uVar28 | 3;
          if ((ulonglong)
              (*(longlong *)(*(longlong *)(lVar34 + 0x148) + 0x48) -
               *(longlong *)(*(longlong *)(lVar34 + 0x148) + 0x40) >> 3) < 3) goto LAB_180491b6a;
          lVar34 = *(longlong *)(*param_2 + uVar27 * 8);
          if (*(longlong *)(lVar34 + 0x150) != 0) {
            LOCK();
            piVar1 = (int *)(*(longlong *)(lVar34 + 0x150) + 8);
            *piVar1 = *piVar1 + 1;
            UNLOCK();
          }
          uStack_658 = *(ulonglong *)(lVar34 + 0x150);
          uStack_664 = uVar28 | 7;
          uVar33 = (ulonglong)uStack_664;
          if (*(char *)(*(longlong *)(lVar34 + 0x148) + 1) != '\0') goto LAB_180491b6f;
          bVar6 = true;
        }
        puVar30 = apuStack_510[0];
        if ((uVar33 & 4) != 0) {
          uStack_664 = (uint)uVar33 & 0xfffffffb;
          uVar33 = (ulonglong)uStack_664;
          if (uStack_658 != 0) {
            FUN_1804474f0(uStack_658);
          }
        }
        if ((uVar33 & 2) != 0) {
          uStack_664 = (uint)uVar33 & 0xfffffffd;
          uVar33 = (ulonglong)uStack_664;
          if (puVar30 != (ulonglong *)0x0) {
            FUN_1804474f0(puVar30);
          }
        }
        if ((uVar33 & 1) != 0) {
          uStack_664 = (uint)uVar33 & 0xfffffffe;
          uVar33 = (ulonglong)uStack_664;
          if (puStack_4e8 != (ulonglong *)0x0) {
            if ((0xfff < ((longlong)puStack_4d8 - (longlong)puStack_4e8 & 0xfffffffffffffffcU)) &&
               (0x1f < (longlong)puStack_4e8 + (-8 - puStack_4e8[-1]))) goto LAB_1804936a2;
            FUN_1803eb2c8();
            puStack_4e8 = (ulonglong *)0x0;
            puStack_4e0 = (ulonglong *)0x0;
            puStack_4d8 = (ulonglong *)0x0;
          }
        }
        puVar30 = puStack_618;
        if (bVar6) {
          if (puStack_618 == puStack_610) {
            lVar34 = (longlong)puStack_618 - (longlong)puStack_620 >> 3;
            if (lVar34 == 0x1fffffffffffffff) goto LAB_1804936d2;
            apuStack_5e8[0] = (ulonglong *)(lVar34 + 1);
            uVar35 = (longlong)puStack_610 - (longlong)puStack_620 >> 3;
            if (0x1fffffffffffffff - (uVar35 >> 1) < uVar35) {
              puVar20 = (ulonglong *)0x1fffffffffffffff;
            }
            else {
              puVar13 = (ulonglong *)((uVar35 >> 1) + uVar35);
              puVar20 = apuStack_5e8[0];
              if (apuStack_5e8[0] <= puVar13) {
                puVar20 = puVar13;
              }
              if ((ulonglong *)0x1fffffffffffffff < puVar20) {
LAB_1804936cc:
                FUN_1804441f0();
                pcVar5 = (code *)swi(3);
                (*pcVar5)();
                return;
              }
            }
            apuStack_5f8[0] = (ulonglong *)((longlong)puVar20 * 8);
            puVar13 = (ulonglong *)FUN_180440bd0(apuStack_5f8[0]);
            puVar13[lVar34] = uVar27;
            if (puVar30 == puStack_618) {
              lVar21 = (longlong)puStack_618 - (longlong)puStack_620;
              puVar20 = puVar13;
              puVar30 = puStack_620;
            }
            else {
              FUN_1806a7f00(puVar13,puStack_620,(longlong)puVar30 - (longlong)puStack_620);
              lVar21 = (longlong)puStack_618 - (longlong)puVar30;
              puVar20 = puVar13 + lVar34 + 1;
            }
            FUN_1806a7f00(puVar20,puVar30,lVar21);
            if (puStack_620 != (ulonglong *)0x0) {
              if ((0xfff < ((longlong)puStack_610 - (longlong)puStack_620 & 0xfffffffffffffff8U)) &&
                 (0x1f < (longlong)puStack_620 + (-8 - puStack_620[-1]))) {
LAB_1804936b7:
                ppuStack_688 = (ulonglong **)0x0;
                FUN_18040e3e4(0,0,0,0);
                pcVar5 = (code *)swi(3);
                (*pcVar5)();
                return;
              }
              FUN_1803eb2c8();
            }
            puStack_618 = puVar13 + (longlong)apuStack_5e8[0];
            puStack_610 = (ulonglong *)((longlong)apuStack_5f8[0] + (longlong)puVar13);
            puStack_620 = puVar13;
          }
          else {
            *puStack_618 = uVar27;
            puStack_618 = puStack_618 + 1;
          }
        }
      }
      uVar28 = (uint)uVar33;
      uVar27 = uVar27 + 1;
      plVar12 = (longlong *)param_2[1];
      plVar19 = (longlong *)*param_2;
      plVar24 = plVar19;
    } while (uVar27 < (ulonglong)((longlong)plVar12 - (longlong)plVar19 >> 3));
  }
  for (; plVar19 != plVar12; plVar19 = plVar19 + 1) {
    if ((*(int *)(*plVar19 + 0x17c) == 2) && (*(int *)(*plVar19 + 0x170) == 2)) {
      uVar29 = uVar29 + 1;
    }
  }
  cStack_660 = (float)(ulonglong)((longlong)plVar12 - (longlong)plVar24 >> 3) * DAT_18258fa94 <=
               (float)uVar29;
  lStack_5d0 = 0;
  puStack_5c8 = (undefined4 *)0x0;
  puStack_5c0 = (undefined4 *)0x0;
  FUN_1804a1300(&lStack_5d0,(longlong)puStack_648 - (longlong)puStack_650 >> 3);
  puVar13 = puStack_648;
  for (puVar30 = puStack_650; puVar30 != puVar13; puVar30 = puVar30 + 1) {
    uVar33 = *puVar30;
    lVar34 = *(longlong *)(*param_2 + uVar33 * 8);
    if (*(longlong *)(lVar34 + 0x150) != 0) {
      LOCK();
      piVar1 = (int *)(*(longlong *)(lVar34 + 0x150) + 8);
      *piVar1 = *piVar1 + 1;
      UNLOCK();
    }
    puStack_560 = *(ulonglong **)(lVar34 + 0x148);
    lVar34 = *(longlong *)(lVar34 + 0x150);
    uVar4 = *(undefined4 *)((longlong)puStack_560 + 0xc);
    uStack_478 = (longlong *)CONCAT44(uVar4,*(undefined4 *)((longlong)puStack_560 + 4));
    uVar29 = puStack_560[1];
    uVar27 = puStack_560[2];
    uStack_470 = (undefined8 **)CONCAT44((int)uVar27,(int)uVar29);
    uStack_468 = (undefined4)uVar33;
    lStack_558 = lVar34;
    if (puStack_5c8 == puStack_5c0) {
      FUN_18047dfe0(&lStack_5d0,puStack_5c8,&uStack_478);
    }
    else {
      *puStack_5c8 = *(undefined4 *)((longlong)puStack_560 + 4);
      puStack_5c8[1] = uVar4;
      puStack_5c8[2] = (int)uVar29;
      puStack_5c8[3] = (int)uVar27;
      puStack_5c8[4] = uStack_468;
      puStack_5c8 = puStack_5c8 + 5;
    }
    if (lVar34 != 0) {
      FUN_1804474f0(lVar34);
    }
  }
  FUN_1806a85c0(auStack_2a8,0,0x1b8);
  FUN_180487ec0(auStack_2a8);
  uVar33 = uVar31;
  uVar29 = uVar31;
  if (((longlong)puStack_5c8 - lStack_5d0 >> 2) * -0x3333333333333333 != 0) {
    do {
      puStack_4e8 = (ulonglong *)(lStack_5d0 + uVar33);
      puStack_4d0 = (ulonglong *)0x0;
      puStack_4e0 = (ulonglong *)*puStack_4e8;
      puStack_4d8 = (ulonglong *)puStack_4e8[1];
      apuStack_4c0[0] = (ulonglong *)0x0;
      acStack_4b0[0] = '\x01';
      puStack_680 = (ulonglong *)acStack_4b0;
      ppuStack_688 = apuStack_4c0;
      cVar10 = FUN_180486b80(auStack_2a8,&puStack_4e8,acStack_667,lStack_f8);
      if (cVar10 != '\0') {
        lVar34 = FUN_1804865f0(extraout_XMM0_Da,*(int *)(lStack_f8 + 4) + 1);
        puVar14 = (undefined4 *)FUN_18049e790(lStack_f8,&uStack_478);
        uStack_588 = *puVar14;
        uStack_584 = puVar14[1];
        uStack_580 = puVar14[2];
        uStack_57c = puVar14[3];
        puStack_578 = (ulonglong *)lStack_f8;
        FUN_18049db70(auStack_2a8,&uStack_590,lVar34,0);
        puVar30 = apuStack_4c0[0];
        puVar14 = (undefined4 *)FUN_18049e790(apuStack_4c0[0],&puStack_560);
        uStack_588 = *puVar14;
        uStack_584 = puVar14[1];
        uStack_580 = puVar14[2];
        uStack_57c = puVar14[3];
        puStack_578 = puVar30;
        FUN_18049db70(auStack_2a8,&uStack_590,lVar34,0);
        lStack_f8 = lVar34;
      }
      lStack_108 = lStack_108 + 1;
      uVar29 = uVar29 + 1;
      uVar33 = uVar33 + 0x14;
    } while (uVar29 < (ulonglong)(((longlong)puStack_5c8 - lStack_5d0 >> 2) * -0x3333333333333333));
  }
  puStack_458 = (ulonglong *)0x0;
  puStack_450 = (ulonglong *)0x0;
  uStack_448 = 0;
  apuStack_5e8[0] = puStack_638;
  puStack_560 = puStack_630;
  puVar30 = puStack_638;
  puVar13 = puStack_630;
  if (puStack_638 != puStack_630) {
    do {
      uVar33 = *puVar30;
      lVar34 = *(longlong *)(*param_2 + uVar33 * 8);
      if (*(longlong *)(lVar34 + 0x150) != 0) {
        LOCK();
        piVar1 = (int *)(*(longlong *)(lVar34 + 0x150) + 8);
        *piVar1 = *piVar1 + 1;
        UNLOCK();
      }
      uStack_478 = *(longlong **)(lVar34 + 0x148);
      ppuVar22 = *(undefined8 ***)(lVar34 + 0x150);
      if (puStack_458 != puStack_450) {
        puStack_450 = puStack_458;
      }
      uStack_588 = (undefined4)uStack_478[1];
      uStack_584 = (undefined4)uStack_478[2];
      uStack_590 = CONCAT44(*(undefined4 *)((longlong)uStack_478 + 0xc),
                            *(undefined4 *)((longlong)uStack_478 + 4));
      apuStack_5b8[0] = (ulonglong *)0x0;
      ppuStack_688 = &puStack_458;
      apuStack_5e8[0] = puVar30;
      ppuStack_568 = ppuVar22;
      uStack_470 = ppuVar22;
      FUN_180487990(auStack_2a8,lStack_f8,&uStack_590,apuStack_5b8);
      apuStack_5f8[0] = puStack_458;
      apuStack_4f8[0] = puStack_450;
      if (puStack_458 != puStack_450) {
        uStack_590 = uVar33 << 5;
        puVar20 = puStack_458;
        do {
          puStack_600 = (ulonglong *)((ulonglong)puStack_600 & 0xffffffff00000000);
          uStack_658 = uStack_658 & 0xffffffff00000000;
          puVar30 = (ulonglong *)(longlong)*(int *)(*puVar20 + 0x10);
          uStack_678 = uStack_5d8;
          puStack_680 = &uStack_658;
          ppuStack_688 = &puStack_600;
          apuStack_5f8[0] = puVar20;
          cVar10 = FUN_180494f00(*(undefined8 *)(*param_2 + (longlong)puVar30 * 8),
                                 *(undefined8 *)(*param_2 + uVar33 * 8),
                                 puStack_490 + (longlong)puVar30 * 4,
                                 (undefined1 *)(uStack_590 + (longlong)puStack_490));
          *(float *)(lStack_5a8 + (longlong)puVar30 * 4) =
               puStack_600._0_4_ + *(float *)(lStack_5a8 + (longlong)puVar30 * 4);
          *(float *)(lStack_5a8 + uVar33 * 4) =
               (float)uStack_658 + *(float *)(lStack_5a8 + uVar33 * 4);
          if (cVar10 != '\0') {
            puVar13 = puStack_440 + (longlong)puVar30 * 3;
            puVar18 = (ulonglong *)puVar13[1];
            if (puVar18 == (ulonglong *)puVar13[2]) {
              lVar34 = (longlong)((longlong)puVar18 - *puVar13) >> 3;
              if (lVar34 == 0x1fffffffffffffff) {
                FUN_180444e60();
                pcVar5 = (code *)swi(3);
                (*pcVar5)();
                return;
              }
              apuStack_510[0] = (ulonglong *)(lVar34 + 1);
              uVar29 = (longlong)((longlong)puVar13[2] - *puVar13) >> 3;
              if (0x1fffffffffffffff - (uVar29 >> 1) < uVar29) {
                puVar17 = (ulonglong *)0x1fffffffffffffff;
              }
              else {
                puVar20 = (ulonglong *)((uVar29 >> 1) + uVar29);
                puVar17 = apuStack_510[0];
                if (apuStack_510[0] <= puVar20) {
                  puVar17 = puVar20;
                }
                if ((ulonglong *)0x1fffffffffffffff < puVar17) goto LAB_1804936de;
              }
              apuStack_4c0[0] = (ulonglong *)FUN_180440bd0((longlong)puVar17 * 8);
              puStack_4d0 = apuStack_4c0[0] + lVar34;
              apuStack_5b8[0] = puStack_4d0 + 1;
              *puStack_4d0 = uVar33;
              uVar29 = *puVar13;
              puStack_4e8 = puVar13;
              puStack_4d8 = puVar17;
              puStack_4c8 = apuStack_5b8[0];
              if (puVar18 == (ulonglong *)puVar13[1]) {
                FUN_1806a7f00(apuStack_4c0[0],uVar29,(longlong)puVar13[1] - uVar29);
                puVar20 = apuStack_4c0[0];
              }
              else {
                FUN_1806a7f00(apuStack_4c0[0],uVar29,(longlong)puVar18 - uVar29);
                puVar20 = apuStack_4c0[0];
                puStack_4d0 = apuStack_4c0[0];
                FUN_1806a7f00(apuStack_5b8[0],puVar18,puVar13[1] - (longlong)puVar18);
              }
              puStack_4e0 = (ulonglong *)0x0;
              FUN_18049a970(puVar13,puVar20,apuStack_510[0],puVar17);
              FUN_18048ba20(&puStack_4e8);
              puVar20 = apuStack_5f8[0];
              param_2 = plStack_608;
            }
            else {
              *puVar18 = uVar33;
              puVar13[1] = puVar13[1] + 8;
            }
            apuStack_5b8[0] = puVar30;
            FUN_18047d7f0(puStack_440 + uVar33 * 3,apuStack_5b8);
          }
          puVar20 = puVar20 + 1;
          puVar30 = apuStack_5e8[0];
          ppuVar22 = ppuStack_568;
          puVar13 = puStack_560;
          apuStack_5f8[0] = puVar20;
        } while (puVar20 != apuStack_4f8[0]);
      }
      if (ppuVar22 != (undefined8 **)0x0) {
        FUN_1804474f0(ppuVar22);
      }
      apuStack_5e8[0] = puVar30 + 1;
      puVar30 = apuStack_5e8[0];
      uVar28 = uStack_664;
    } while (apuStack_5e8[0] != puVar13);
  }
  lStack_3d8 = 0;
  uStack_3d0 = 0;
  uStack_3c8 = 0;
  uStack_3c0 = 0;
  acStack_4b0[0] = 0;
  FUN_180488dd0(&lStack_3d8,param_2[1] - *param_2 >> 3,acStack_4b0);
  lStack_3f8 = 0;
  uStack_3f0 = 0;
  uStack_3e8 = 0;
  uStack_3e0 = 0;
  acStack_4b0[0] = 0;
  FUN_180488dd0(&lStack_3f8,param_2[1] - *param_2 >> 3,acStack_4b0);
  FUN_1806a85c0(&puStack_3b8,0,0x108);
  puStack_3b8 = &DAT_1825825c8;
  uStack_308 = 0;
  uStack_300 = 0;
  uStack_2f8 = 0;
  uStack_2f0 = 0;
  uStack_2e8 = 0;
  uStack_2e0 = 0;
  uStack_2d8 = 0;
  uStack_2d0 = 0;
  uStack_2c8 = 0;
  uStack_2c0 = 0;
  uStack_2b8 = 0;
  uStack_540 = uVar28 | 0x80;
  puStack_310 = &DAT_1806e7c60;
  uStack_314 = 0x98;
  FUN_1804a0360(&puStack_310,auStack_3b0,0);
  *(undefined **)(auStack_3b0 + (longlong)*(int *)(puStack_3b8 + 4) + -8) = &DAT_1807383c0;
  *(int *)((longlong)&uStack_3c0 + (longlong)*(int *)(puStack_3b8 + 4) + 4) =
       *(int *)(puStack_3b8 + 4) + -0xa8;
  FUN_180488140(auStack_3b0);
  uVar33 = uStack_5d8;
  if ((*(int *)(uStack_5d8 + 0x68) < 3) || (*(char *)(uStack_5d8 + 0x6c) == '\0')) {
    acStack_4b0[0] = '\0';
  }
  else {
    acStack_4b0[0] = '\x01';
    plVar12 = (longlong *)(uStack_5d8 + 0x30);
    if (0x7fffffffffffffffU - (longlong)*(ulonglong ***)(uStack_5d8 + 0x40) < 0xb) {
LAB_1804936d8:
      FUN_180444e40();
LAB_1804936de:
      FUN_1804441f0();
      pcVar5 = (code *)swi(3);
      (*pcVar5)();
      return;
    }
    plVar19 = plVar12;
    if (0xf < *(ulonglong *)(uStack_5d8 + 0x48)) {
      plVar19 = (longlong *)*plVar12;
    }
    uStack_678 = 0xb;
    puStack_680 = &DAT_182583018;
    ppuStack_688 = *(ulonglong ***)(uStack_5d8 + 0x40);
    FUN_180488350(&puStack_560,acStack_667[0],plVar12,plVar19);
    uVar15 = FUN_18047cc70(&uStack_590,&puStack_560,uVar33 + 0x10);
    puVar16 = (undefined8 *)FUN_180444e80(uVar15,&DAT_182583038,0x12);
    puStack_4e8 = (ulonglong *)*puVar16;
    puStack_4e0 = (ulonglong *)puVar16[1];
    puStack_4d8 = (ulonglong *)puVar16[2];
    puStack_4d0 = (ulonglong *)puVar16[3];
    puVar16[2] = 0;
    puVar16[3] = 0xf;
    *(undefined1 *)puVar16 = 0;
    FUN_18047d550(&uStack_478,&puStack_4e8,0);
    FUN_1804448d0(&puStack_4e8);
    FUN_1804448d0(&uStack_590);
    FUN_1804448d0(&puStack_560);
    plVar12 = &uStack_478;
    if (7 < uStack_460) {
      plVar12 = uStack_478;
    }
    lVar34 = FUN_1804a0940(auStack_3b0,plVar12,6);
    lVar21 = (longlong)*(int *)(puStack_3b8 + 4);
    if (lVar34 == 0) {
      uVar28 = 2;
      if (*(longlong *)((longlong)alStack_370 + lVar21) == 0) {
        uVar28 = 6;
      }
      uVar31 = (ulonglong)(uVar28 | *(uint *)((longlong)auStack_3a8 + lVar21));
    }
    else if (*(longlong *)((longlong)alStack_370 + lVar21) == 0) {
      uVar31 = 4;
    }
    FUN_18049e590(auStack_3b0 + lVar21 + -8,uVar31,0);
    FUN_180444950(&uStack_478);
  }
  uVar28 = DAT_1825904b0;
  fVar9 = DAT_18258fac8;
  fVar8 = DAT_18258fab8;
  dVar7 = DAT_18258fab0;
  uStack_5d8 = 0;
  if (param_2[1] - *param_2 >> 3 != 0) {
    do {
      uVar31 = uStack_5d8;
      ppuVar22 = (undefined8 **)0x0;
      uVar33 = uStack_5d8 >> 5;
      uVar32 = 1 << ((byte)uStack_5d8 & 0x1f);
      if ((*(uint *)(lStack_3f8 + uVar33 * 4) & uVar32) == 0) {
        puStack_4e8 = (ulonglong *)0x0;
        puStack_4e0 = (ulonglong *)0x0;
        puStack_4d8 = (ulonglong *)0x0;
        puVar17 = (ulonglong *)FUN_180440bd0(8);
        puVar30 = puVar17 + 1;
        ppuStack_538 = &puStack_4e8;
        uStack_528 = 1;
        *puVar17 = uVar31;
        puVar13 = puVar17;
        puVar20 = puStack_4e8;
        puVar18 = (ulonglong *)-(longlong)puStack_4e8;
        puStack_520 = puVar17;
        puStack_518 = puVar30;
        if (puStack_4e0 != (ulonglong *)0x0) {
          FUN_1806a7f00(puVar17,puStack_4e8,(ulonglong *)-(longlong)puStack_4e8);
          puVar20 = (ulonglong *)0x0;
          puVar13 = puVar30;
          puVar18 = puStack_4e0;
        }
        FUN_1806a7f00(puVar13,puVar20,puVar18);
        uStack_530 = 0;
        FUN_18049a970(&puStack_4e8,puVar17,1);
        FUN_18048ba20(&ppuStack_538);
        puVar2 = (uint *)(lStack_3f8 + uVar33 * 4);
        *puVar2 = *puVar2 | uVar32;
        puVar30 = puStack_4e0;
        puVar13 = puStack_4e8;
        if ((longlong)puStack_4e0 - (longlong)puStack_4e8 >> 3 != 0) {
          do {
            ppuStack_568 = (undefined8 **)((longlong)ppuVar22 + 1);
            puVar20 = (ulonglong *)puStack_440[puVar13[(longlong)ppuVar22] * 3];
            puVar18 = (ulonglong *)puStack_440[puVar13[(longlong)ppuVar22] * 3 + 1];
            puStack_560 = puVar18;
            if (puVar20 != puVar18) {
              do {
                uVar33 = *puVar20;
                if ((*(uint *)(lStack_3f8 + (uVar33 >> 5) * 4) >> ((byte)uVar33 & 0x1f) & 1) == 0) {
                  if (puVar30 == puStack_4d8) {
                    uVar31 = (longlong)puVar30 - (longlong)puVar13 >> 3;
                    if (uVar31 == 0x1fffffffffffffff) {
LAB_18049368a:
                      FUN_180444e60();
                      goto LAB_180493690;
                    }
                    uVar29 = uVar31 + 1;
                    uVar27 = (longlong)puStack_4d8 - (longlong)puVar13 >> 3;
                    if (0x1fffffffffffffff - (uVar27 >> 1) < uVar27) {
                      uVar35 = 0x1fffffffffffffff;
                    }
                    else {
                      uVar27 = (uVar27 >> 1) + uVar27;
                      uVar35 = uVar29;
                      if (uVar29 <= uVar27) {
                        uVar35 = uVar27;
                      }
                      if (0x1fffffffffffffff < uVar35) {
                        FUN_1804441f0();
                        goto LAB_18049368a;
                      }
                    }
                    puVar18 = (ulonglong *)FUN_180440bd0(uVar35 * 8);
                    puStack_520 = puVar18 + uVar31;
                    puVar13 = puStack_520 + 1;
                    ppuStack_538 = &puStack_4e8;
                    *puStack_520 = uVar33;
                    uStack_528 = uVar35;
                    puStack_518 = puVar13;
                    if (puVar30 == puStack_4e0) {
                      lVar34 = (longlong)puStack_4e0 - (longlong)puStack_4e8;
                      puVar13 = puVar18;
                      puVar30 = puStack_4e8;
                    }
                    else {
                      FUN_1806a7f00(puVar18,puStack_4e8,(longlong)puVar30 - (longlong)puStack_4e8);
                      lVar34 = (longlong)puStack_4e0 - (longlong)puVar30;
                      puStack_520 = puVar18;
                    }
                    FUN_1806a7f00(puVar13,puVar30,lVar34);
                    uStack_530 = 0;
                    FUN_18049a970(&puStack_4e8,puVar18,uVar29);
                    FUN_18048ba20(&ppuStack_538);
                    puVar18 = puStack_560;
                  }
                  else {
                    *puVar30 = uVar33;
                    puStack_4e0 = puStack_4e0 + 1;
                  }
                  *(uint *)(lStack_3f8 + (uVar33 >> 5) * 4) =
                       *(uint *)(lStack_3f8 + (uVar33 >> 5) * 4) | 1 << ((uint)uVar33 & 0x1f);
                  puVar30 = puStack_4e0;
                  puVar13 = puStack_4e8;
                }
                puVar20 = puVar20 + 1;
              } while (puVar20 != puVar18);
            }
            ppuVar22 = ppuStack_568;
          } while (ppuStack_568 < (undefined8 **)((longlong)puVar30 - (longlong)puVar13 >> 3));
        }
        plVar12 = plStack_608;
        uVar27 = 0;
        fVar41 = 0.0;
        fVar42 = 0.0;
        fVar43 = 0.0;
        fVar44 = 0.0;
        uVar31 = uVar27;
        uVar33 = uVar27;
        param_2 = plStack_608;
        uVar29 = uVar27;
        if (puVar13 != puVar30) {
          do {
            uVar35 = *puVar13;
            fVar37 = *(float *)(*plStack_570 + 0x60 + uVar35 * 0x90);
            fVar39 = *(float *)(*plStack_570 + 0x5c + uVar35 * 0x90);
            if (*(char *)(*(longlong *)(*plVar12 + uVar35 * 8) + 0x15a) == '\0') {
              fVar42 = fVar42 + fVar37;
              fVar44 = fVar44 + fVar39;
              uVar29 = uVar29 + 1;
              plVar19 = (longlong *)FUN_18063bfd0(&puStack_560,puStack_490 + uVar35 * 4);
              uVar33 = uVar33 + (plVar19[1] - *plVar19 >> 2);
            }
            else {
              fVar41 = fVar41 + fVar37;
              fVar43 = fVar43 + fVar39;
              uVar27 = uVar27 + 1;
              plVar19 = (longlong *)FUN_18063bfd0(&puStack_560,puStack_490 + uVar35 * 4);
              uVar31 = uVar31 + (plVar19[1] - *plVar19 >> 2);
            }
            FUN_1804442f0(&puStack_560);
            param_2 = plStack_608;
            puVar13 = puVar13 + 1;
          } while (puVar13 != puVar30);
          if ((uVar27 != 0) && (uVar29 != 0)) {
            fVar41 = fVar41 / (float)uVar27;
            fVar42 = fVar42 / (float)uVar29;
            fVar43 = fVar43 / (float)uVar27;
            fVar44 = fVar44 / (float)uVar29;
            uVar27 = (longlong)((longlong)puStack_4a0 - uStack_4a8) >> 3;
            uVar29 = (longlong)puStack_618 - (longlong)puStack_620 >> 3;
            cStack_668 = uVar27 <= uVar29;
            fVar40 = (float)uVar27;
            fVar38 = (float)uVar29;
            fVar39 = (float)uVar31;
            fVar36 = (float)uVar33;
            fVar37 = fVar39;
            if (fVar39 < fVar36) {
              fVar37 = fVar36;
            }
            if (dVar7 < (double)fVar37) {
              fVar37 = fVar39;
              if (fVar39 < fVar36) {
                fVar37 = fVar36;
              }
              fVar37 = (fVar39 - fVar36) / fVar37;
            }
            else {
              fVar37 = 0.0;
            }
            fVar39 = fVar41;
            if (fVar41 < fVar42) {
              fVar39 = fVar42;
            }
            if (dVar7 < (double)fVar39) {
              fVar39 = fVar41;
              if (fVar41 < fVar42) {
                fVar39 = fVar42;
              }
              fVar39 = (float)((uint)(fVar41 - fVar42) ^ uVar28) / fVar39;
            }
            else {
              fVar39 = 0.0;
            }
            fVar36 = fVar43;
            if (fVar43 < fVar44) {
              fVar36 = fVar44;
            }
            if (dVar7 < (double)fVar36) {
              fVar36 = fVar43;
              if (fVar43 < fVar44) {
                fVar36 = fVar44;
              }
              fVar36 = (fVar43 - fVar44) / fVar36;
            }
            else {
              fVar36 = 0.0;
            }
            fVar36 = fVar39 + fVar37 + fVar36;
            if (cStack_660 != '\0') {
              fVar37 = fVar38;
              if (fVar38 < fVar40) {
                fVar37 = fVar40;
              }
              if (dVar7 < (double)fVar37) {
                fVar37 = fVar38;
                if (fVar38 < fVar40) {
                  fVar37 = fVar40;
                }
                fVar37 = (fVar38 - fVar40) / fVar37;
              }
              else {
                fVar37 = 0.0;
              }
              fVar36 = fVar36 + fVar37 * fVar9;
            }
            acStack_667[0] = 0.0 < fVar36;
            lVar34 = (longlong)puStack_4e0 - (longlong)puStack_4e8;
            uStack_478 = plStack_608;
            uStack_470 = &puStack_490;
            uVar33 = FUN_180486750(puStack_4e8,puStack_4e0,&uStack_478);
            puVar20 = puStack_4e0;
            uVar31 = 0;
            puVar13 = puStack_4e0;
            cVar10 = acStack_4b0[0];
            for (puVar30 = puStack_4e8; puStack_4e0 = puVar13, acStack_4b0[0] = cVar10,
                puVar30 != puVar20; puVar30 = puVar30 + 1) {
              cVar10 = FUN_18063b400(puStack_490 + *puVar30 * 4,0);
              uVar29 = uVar31 + 1;
              if (cVar10 == '\0') {
                uVar29 = uVar31;
              }
              uVar31 = uVar29;
              puVar13 = puStack_4e0;
              cVar10 = acStack_4b0[0];
            }
            uVar29 = (ulonglong)(lVar34 >> 3) >> 1;
            lVar34 = lStack_3d8;
            puVar30 = puStack_4e8;
            if ((uVar29 <= uVar31) || (cVar23 = cStack_668, uVar33 <= uVar29)) {
              cVar23 = acStack_667[0];
              cStack_668 = acStack_667[0];
            }
            for (; puVar30 != puVar13; puVar30 = puVar30 + 1) {
              uVar31 = *puVar30;
              if (*(char *)(*(longlong *)(*param_2 + uVar31 * 8) + 0x15a) != cVar23) {
                uVar32 = 1 << ((byte)uVar31 & 0x1f);
                puVar2 = (uint *)(lVar34 + (uVar31 >> 5) * 4);
                pfVar3 = (float *)(lStack_5a8 + uVar31 * 4);
                if (fVar8 < *pfVar3 || fVar8 == *pfVar3) {
                  uVar11 = uVar32 | *puVar2;
                }
                else {
                  uVar11 = ~uVar32 & *puVar2;
                }
                *puVar2 = uVar11;
                lVar34 = lStack_3d8;
                cVar23 = cStack_668;
                if (((*(uint *)(lStack_3d8 + (uVar31 >> 5) * 4) & uVar32) != 0) && (cVar10 != '\0'))
                {
                  plVar12 = (longlong *)FUN_18047c970(&puStack_3b8,&DAT_182583028);
                  FUN_1804a0720((longlong)*(int *)(*plVar12 + 4) + (longlong)plVar12);
                  lVar34 = FUN_1803e4b34(&uStack_590,5);
                  (*(code *)PTR__guard_dispatch_icall_1806db690)
                            ((longlong)*(int *)(*plVar12 + 4) + (longlong)plVar12,
                             *(undefined8 *)(lVar34 + 8));
                  FUN_18048c440(plVar12,uVar31);
                  plVar12 = (longlong *)FUN_18047c970(&puStack_3b8,&DAT_182583050);
                  FUN_1804a0720((longlong)*(int *)(*plVar12 + 4) + (longlong)plVar12);
                  lVar34 = FUN_1803e4b34(apuStack_4f8,5);
                  (*(code *)PTR__guard_dispatch_icall_1806db690)
                            ((longlong)*(int *)(*plVar12 + 4) + (longlong)plVar12,
                             *(undefined8 *)(lVar34 + 8));
                  puVar25 = &DAT_182583074;
                  if (*(char *)(*(longlong *)(*param_2 + uVar31 * 8) + 0x15a) == '\0') {
                    puVar25 = &DAT_182583078;
                  }
                  FUN_18047c970(plVar12,puVar25);
                  plVar12 = (longlong *)FUN_18047c970(&puStack_3b8,&DAT_182583060);
                  FUN_1804a0720((longlong)*(int *)(*plVar12 + 4) + (longlong)plVar12);
                  lVar34 = FUN_1803e4b34(apuStack_5b8,10);
                  (*(code *)PTR__guard_dispatch_icall_1806db690)
                            ((longlong)*(int *)(*plVar12 + 4) + (longlong)plVar12,
                             *(undefined8 *)(lVar34 + 8));
                  FUN_18048c1e0(plVar12,fVar41);
                  plVar12 = (longlong *)FUN_18047c970(&puStack_3b8,&DAT_1825830a8);
                  FUN_1804a0720((longlong)*(int *)(*plVar12 + 4) + (longlong)plVar12);
                  lVar34 = FUN_1803e4b34(apuStack_510,10);
                  (*(code *)PTR__guard_dispatch_icall_1806db690)
                            ((longlong)*(int *)(*plVar12 + 4) + (longlong)plVar12,
                             *(undefined8 *)(lVar34 + 8));
                  FUN_18048c1e0(plVar12,fVar42);
                  plVar12 = (longlong *)FUN_18047c970(&puStack_3b8,&DAT_1825830c0);
                  FUN_1804a0720((longlong)*(int *)(*plVar12 + 4) + (longlong)plVar12);
                  lVar34 = FUN_1803e4b34(apuStack_5e8,10);
                  (*(code *)PTR__guard_dispatch_icall_1806db690)
                            ((longlong)*(int *)(*plVar12 + 4) + (longlong)plVar12,
                             *(undefined8 *)(lVar34 + 8));
                  FUN_18048c1e0(plVar12,fVar43);
                  plVar12 = (longlong *)FUN_18047c970(&puStack_3b8,&DAT_182583080);
                  FUN_1804a0720((longlong)*(int *)(*plVar12 + 4) + (longlong)plVar12);
                  lVar34 = FUN_1803e4b34(apuStack_5f8,10);
                  (*(code *)PTR__guard_dispatch_icall_1806db690)
                            ((longlong)*(int *)(*plVar12 + 4) + (longlong)plVar12,
                             *(undefined8 *)(lVar34 + 8));
                  FUN_18048c1e0(plVar12,fVar44);
                  plVar12 = (longlong *)FUN_18047c970(&puStack_3b8,&DAT_182583098);
                  FUN_1804a0720((longlong)*(int *)(*plVar12 + 4) + (longlong)plVar12);
                  lVar34 = FUN_1803e4b34(apuStack_4c0,10);
                  (*(code *)PTR__guard_dispatch_icall_1806db690)
                            ((longlong)*(int *)(*plVar12 + 4) + (longlong)plVar12,
                             *(undefined8 *)(lVar34 + 8));
                  FUN_18048c1e0(plVar12);
                  uVar15 = FUN_18047c970(&puStack_3b8,&DAT_1825830e4);
                  puVar26 = puStack_490 + uVar31 * 4;
                  puVar16 = puVar26 + 2;
                  if (0xf < (ulonglong)puVar26[3]) {
                    puVar26 = (undefined8 *)*puVar26;
                  }
                  FUN_180480360(uVar15,puVar26,*puVar16);
                  FUN_180486b40(&puStack_3b8);
                  lVar34 = lStack_3d8;
                  cVar23 = cStack_668;
                }
              }
            }
          }
        }
        FUN_180442d40(&puStack_4e8);
      }
      uStack_5d8 = uStack_5d8 + 1;
    } while (uStack_5d8 < (ulonglong)(param_2[1] - *param_2 >> 3));
  }
  plVar12 = plStack_570;
  uVar31 = 0;
  uStack_410 = 0;
  uStack_408 = 0;
  uStack_400 = 0;
  uStack_428 = 0;
  uStack_420 = 0;
  uStack_418 = 0;
  if ((plStack_570[1] - *plStack_570 >> 4) * -0x71c71c71c71c71c7 != 0) {
    do {
      uStack_478 = (longlong *)(lStack_3d8 + (uVar31 >> 5) * 4);
      uStack_470 = (undefined8 **)(ulonglong)((uint)uVar31 & 0x1f);
      uVar15 = FUN_18048c8d0(plVar12,uVar31);
      cVar10 = FUN_18048c8e0(&uStack_478);
      puVar16 = &uStack_410;
      if (cVar10 == '\0') {
        puVar16 = &uStack_428;
      }
      FUN_1804a0f30(puVar16,uVar15);
      uVar31 = uVar31 + 1;
    } while (uVar31 < (ulonglong)((plVar12[1] - *plVar12 >> 4) * -0x71c71c71c71c71c7));
  }
  FUN_18047c8f0(puStack_500,&uStack_428,&uStack_410);
  FUN_1804444e0(&uStack_428);
  FUN_1804444e0(&uStack_410);
  FUN_18048d270(&puStack_3b8);
  FUN_1804442f0(&lStack_3f8);
  FUN_1804442f0(&lStack_3d8);
  FUN_180442d40(&puStack_458);
  FUN_180489fd0(auStack_2a8);
  FUN_18048a930(&lStack_5d0);
  FUN_180444580(&puStack_490);
  FUN_1804442f0(&lStack_5a8);
  FUN_18048abb0(&puStack_440);
  FUN_180442d40(&uStack_4a8);
  FUN_180442d40(&puStack_620);
  FUN_180442d40(&puStack_638);
  FUN_180442d40(&puStack_650);
  FUN_1803eae20(uStack_e8 ^ (ulonglong)auStack_6a8);
  return;
}

