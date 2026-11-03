// 18058a380 FUN_18058a380

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_18058a380(int param_1,double *param_2,longlong param_3,int param_4,longlong *param_5)

{
  double dVar1;
  undefined8 uVar2;
  code *pcVar3;
  double dVar4;
  ulonglong uVar5;
  float *pfVar6;
  ulonglong uVar7;
  ulonglong uVar8;
  undefined8 *puVar9;
  ulonglong uVar10;
  longlong *plVar11;
  undefined8 *puVar12;
  longlong lVar13;
  longlong lVar14;
  longlong lVar15;
  int iVar16;
  double dVar17;
  undefined1 auStack_148 [32];
  ulonglong *puStack_128;
  ulonglong *puStack_120;
  undefined8 uStack_118;
  ulonglong uStack_110;
  longlong lStack_108;
  ulonglong uStack_100;
  ulonglong uStack_f8;
  undefined8 uStack_f0;
  ulonglong uStack_e8;
  longlong lStack_e0;
  ulonglong uStack_d8;
  ulonglong uStack_d0;
  undefined8 uStack_c8;
  ulonglong uStack_c0;
  longlong lStack_b8;
  ulonglong uStack_b0;
  ulonglong uStack_a8;
  undefined8 uStack_a0;
  ulonglong uStack_98;
  longlong lStack_90;
  ulonglong uStack_88;
  ulonglong uStack_80;
  undefined8 uStack_78;
  ulonglong uStack_70;
  longlong lStack_68;
  ulonglong uStack_60;
  ulonglong uStack_58;
  undefined8 uStack_50;
  ulonglong uStack_48;
  undefined8 uStack_40;
  ulonglong uStack_38;
  
  uStack_38 = DAT_18264bfc0 ^ (ulonglong)auStack_148;
  uVar7 = (ulonglong)param_1;
  iVar16 = param_4;
  if (param_1 <= param_4) {
    iVar16 = param_1 + -1;
  }
  uStack_f0 = 0;
  lStack_e0 = 0;
  uStack_f8 = (ulonglong)(iVar16 + 1);
  uStack_e8 = uStack_f8 * uVar7;
  uVar10 = 0;
  uStack_100 = uVar7;
  if (uStack_e8 == 0) {
    lStack_e0 = 0;
LAB_18058a43f:
    uStack_78 = 0;
    lStack_68 = 0;
    uStack_80 = 1;
    uStack_88 = uVar7;
    uStack_70 = uVar7;
    if (uVar7 != 0) {
      if (uVar7 < 0x2000000000000000) {
        lStack_68 = FUN_180440bd0(uVar7 * 8);
        goto LAB_18058a48e;
      }
      goto LAB_18058abaa;
    }
    lStack_68 = 0;
LAB_18058a48e:
    uVar8 = uVar10;
    if (uVar7 < 4) {
      if (uVar7 != 0) goto LAB_18058a517;
LAB_18058a52f:
      if (uVar7 != 0) goto LAB_18058a541;
    }
    else {
      lVar15 = 2;
      puVar9 = (undefined8 *)(param_3 + 0x10);
      do {
        *(undefined8 *)(lStack_68 + uStack_80 * uVar8 * 8) = puVar9[-2];
        *(undefined8 *)(lStack_68 + (lVar15 + -1) * uStack_80 * 8) = puVar9[-1];
        *(undefined8 *)(lStack_68 + uStack_80 * lVar15 * 8) = *puVar9;
        *(undefined8 *)(lStack_68 + (lVar15 + 1) * uStack_80 * 8) = puVar9[1];
        uVar8 = uVar8 + 4;
        puVar9 = puVar9 + 4;
        lVar15 = lVar15 + 4;
      } while (uVar8 < uVar7 - 3);
      if (uVar8 < uVar7) {
LAB_18058a517:
        do {
          *(undefined8 *)(lStack_68 + uStack_80 * uVar8 * 8) = *(undefined8 *)(param_3 + uVar8 * 8);
          uVar8 = uVar8 + 1;
        } while (uVar8 < uVar7);
        goto LAB_18058a52f;
      }
LAB_18058a541:
      dVar4 = DAT_18258fb60;
      lVar15 = (longlong)iVar16;
      uVar8 = uVar10;
      do {
        if (-1 < lVar15) {
          uVar5 = uVar10;
          dVar17 = dVar4;
          if (2 < lVar15) {
            do {
              *(double *)(lStack_e0 + (uStack_f8 * uVar8 + uVar5) * 8) = dVar17;
              dVar1 = *param_2;
              *(double *)(lStack_e0 + 8 + (uStack_f8 * uVar8 + uVar5) * 8) = dVar17 * dVar1;
              dVar17 = dVar17 * dVar1 * *param_2;
              *(double *)(lStack_e0 + 0x10 + (uStack_f8 * uVar8 + uVar5) * 8) = dVar17;
              dVar17 = dVar17 * *param_2;
              *(double *)(lStack_e0 + 0x18 + (uStack_f8 * uVar8 + uVar5) * 8) = dVar17;
              dVar17 = dVar17 * *param_2;
              uVar5 = uVar5 + 4;
            } while ((longlong)uVar5 <= lVar15 + -3);
            if (lVar15 < (longlong)uVar5) goto LAB_18058a5f7;
          }
          do {
            *(double *)(lStack_e0 + (uStack_f8 * uVar8 + uVar5) * 8) = dVar17;
            dVar17 = dVar17 * *param_2;
            uVar5 = uVar5 + 1;
          } while ((longlong)uVar5 <= lVar15);
        }
LAB_18058a5f7:
        uVar8 = uVar8 + 1;
        param_2 = param_2 + 1;
      } while (uVar8 < uVar7);
    }
    uStack_c8 = 0;
    lStack_b8 = 0;
    uStack_d8 = uStack_f8;
    uStack_d0 = uStack_100;
    uStack_c0 = uStack_f8 * uStack_100;
    if (uStack_c0 != 0) {
      if (uStack_c0 < 0x2000000000000000) {
        lStack_b8 = FUN_180440bd0(uStack_c0 * 8);
        goto LAB_18058a675;
      }
      goto LAB_18058abb0;
    }
    lStack_b8 = 0;
LAB_18058a675:
    uVar7 = uVar10;
    if (uStack_d8 != 0) {
      do {
        uVar8 = uVar10;
        if (uStack_d0 < 4) {
          if (uStack_d0 != 0) goto LAB_18058a74f;
        }
        else {
          do {
            *(undefined8 *)(lStack_b8 + (uStack_d0 * uVar7 + uVar8) * 8) =
                 *(undefined8 *)(lStack_e0 + (uStack_f8 * uVar8 + uVar7) * 8);
            *(undefined8 *)(lStack_b8 + 8 + (uStack_d0 * uVar7 + uVar8) * 8) =
                 *(undefined8 *)(lStack_e0 + ((uVar8 + 1) * uStack_f8 + uVar7) * 8);
            *(undefined8 *)(lStack_b8 + 0x10 + (uStack_d0 * uVar7 + uVar8) * 8) =
                 *(undefined8 *)(lStack_e0 + ((uVar8 + 2) * uStack_f8 + uVar7) * 8);
            *(undefined8 *)(lStack_b8 + 0x18 + (uStack_d0 * uVar7 + uVar8) * 8) =
                 *(undefined8 *)(lStack_e0 + ((uVar8 + 3) * uStack_f8 + uVar7) * 8);
            uVar8 = uVar8 + 4;
          } while (uVar8 < uStack_d0 - 3);
          for (; uVar8 < uStack_d0; uVar8 = uVar8 + 1) {
LAB_18058a74f:
            *(undefined8 *)(lStack_b8 + (uStack_d0 * uVar7 + uVar8) * 8) =
                 *(undefined8 *)(lStack_e0 + (uStack_f8 * uVar8 + uVar7) * 8);
          }
        }
        uVar7 = uVar7 + 1;
      } while (uVar7 < uStack_d8);
    }
    uStack_50 = 0;
    uStack_40 = 0;
    puStack_128 = &uStack_d8;
    puStack_120 = &uStack_100;
    uStack_58 = uStack_f8;
    uStack_48 = uStack_f8 * uStack_d8;
    uStack_60 = uStack_d8;
    if (uStack_48 == 0) {
      uStack_40 = 0;
LAB_18058a7ee:
      FUN_180588c80(&uStack_60,&puStack_128);
      uStack_a0 = 0;
      lStack_90 = 0;
      puStack_128 = &uStack_d8;
      puStack_120 = &uStack_88;
      uStack_b0 = uStack_d8;
      uStack_a8 = uStack_80;
      uStack_98 = uStack_80 * uStack_d8;
      if (uStack_98 == 0) {
        lStack_90 = 0;
      }
      else {
        if (0x1fffffffffffffff < uStack_98) goto LAB_18058abbc;
        lStack_90 = FUN_180440bd0(uStack_98 * 8);
      }
      FUN_180588c80(&uStack_b0,&puStack_128);
      uVar7 = uStack_60;
      uStack_118 = 0;
      lStack_108 = 0;
      uStack_110 = uStack_60;
      if (uStack_60 == 0) {
        lStack_108 = 0;
      }
      else {
        if (0x1fffffffffffffff < uStack_60) {
          FUN_1804441f0();
          goto LAB_18058aba4;
        }
        lStack_108 = FUN_180440bd0(uStack_60 * 8);
      }
      lVar13 = _UNK_18258fdb8;
      lVar15 = _DAT_18258fdb0;
      if (uVar7 != 0) {
        uVar8 = uVar10;
        if (7 < uVar7) {
          lVar14 = 4;
          plVar11 = (longlong *)(lStack_108 + 0x20);
          do {
            plVar11[-4] = uVar8 + lVar15;
            plVar11[-3] = uVar8 + lVar13;
            plVar11[-2] = lVar14 + -2 + lVar15;
            plVar11[-1] = lVar14 + -2 + lVar13;
            *plVar11 = lVar14 + lVar15;
            plVar11[1] = lVar14 + lVar13;
            plVar11[2] = lVar14 + 2 + lVar15;
            plVar11[3] = lVar14 + 2 + lVar13;
            uVar8 = uVar8 + 8;
            plVar11 = plVar11 + 8;
            lVar14 = lVar14 + 8;
          } while (uVar8 < (uVar7 & 0xfffffffffffffff8));
          if (uVar7 <= uVar8) goto LAB_18058a96c;
        }
        do {
          *(ulonglong *)(lStack_108 + uVar8 * 8) = uVar8;
          uVar8 = uVar8 + 1;
        } while (uVar8 < uVar7);
      }
LAB_18058a96c:
      lVar15 = FUN_180589550(&uStack_60,&uStack_118);
      if (lVar15 == 0) {
        uVar7 = uVar10;
        if (uStack_110 != 0) {
          do {
            uVar8 = *(ulonglong *)(lStack_108 + uVar7 * 8);
            if (uVar7 != uVar8) {
              puVar9 = (undefined8 *)(lStack_90 + uStack_a8 * uVar7 * 8);
              puVar12 = (undefined8 *)(lStack_90 + uVar8 * uStack_a8 * 8);
              lVar15 = uStack_a8 - 1;
              if (-1 < lVar15) {
                if (3 < (longlong)uStack_a8) {
                  uVar8 = uStack_a8 >> 2;
                  lVar15 = lVar15 + uVar8 * -4;
                  do {
                    uVar2 = *puVar9;
                    *puVar9 = *puVar12;
                    *puVar12 = uVar2;
                    uVar2 = puVar9[1];
                    puVar9[1] = puVar12[1];
                    puVar12[1] = uVar2;
                    uVar2 = puVar9[2];
                    puVar9[2] = puVar12[2];
                    puVar12[2] = uVar2;
                    uVar2 = puVar9[3];
                    puVar9[3] = puVar12[3];
                    puVar12[3] = uVar2;
                    puVar12 = puVar12 + 4;
                    puVar9 = puVar9 + 4;
                    uVar8 = uVar8 - 1;
                  } while (uVar8 != 0);
                  if (lVar15 < 0) goto LAB_18058aaad;
                }
                lVar13 = (longlong)puVar12 - (longlong)puVar9;
                do {
                  uVar2 = *puVar9;
                  *puVar9 = *(undefined8 *)(lVar13 + (longlong)puVar9);
                  *(undefined8 *)(lVar13 + (longlong)puVar9) = uVar2;
                  puVar9 = puVar9 + 1;
                  lVar15 = lVar15 + -1;
                } while (-1 < lVar15);
              }
            }
LAB_18058aaad:
            uVar7 = uVar7 + 1;
          } while (uVar7 < uStack_110);
        }
        puStack_128 = &uStack_60;
        FUN_1805892d0(&puStack_128,&uStack_b0);
        FUN_180589100(&uStack_60,&uStack_b0);
        puStack_128 = (ulonglong *)((ulonglong)puStack_128 & 0xffffffff00000000);
        FUN_18051a4e0(param_5,(longlong)(param_4 + 1),&puStack_128);
        if (uStack_b0 != 0) {
          pfVar6 = (float *)*param_5;
          do {
            *pfVar6 = (float)*(double *)(lStack_90 + uStack_a8 * uVar10 * 8);
            uVar10 = uVar10 + 1;
            pfVar6 = pfVar6 + 1;
          } while (uVar10 < uStack_b0);
        }
        FUN_180589880(&uStack_118);
        FUN_180589880(&uStack_a0);
        FUN_180589880(&uStack_50);
        FUN_180589880(&uStack_c8);
        FUN_180589880(&uStack_78);
        FUN_180589880(&uStack_f0);
      }
      else {
        FUN_180589880(&uStack_118);
        FUN_180589880(&uStack_a0);
        FUN_180589880(&uStack_50);
        FUN_180589880(&uStack_c8);
        FUN_180589880(&uStack_78);
        FUN_180589880(&uStack_f0);
      }
      FUN_1803eae20(uStack_38 ^ (ulonglong)auStack_148);
      return;
    }
    if (uStack_48 < 0x2000000000000000) {
      uStack_40 = FUN_180440bd0(uStack_48 * 8);
      goto LAB_18058a7ee;
    }
  }
  else {
    if (uStack_e8 < 0x2000000000000000) {
      lStack_e0 = FUN_180440bd0(uStack_e8 * 8);
      goto LAB_18058a43f;
    }
LAB_18058aba4:
    FUN_1804441f0();
LAB_18058abaa:
    FUN_1804441f0();
LAB_18058abb0:
    FUN_1804441f0();
  }
  FUN_1804441f0();
LAB_18058abbc:
  FUN_1804441f0();
  pcVar3 = (code *)swi(3);
  (*pcVar3)();
  return;
}

