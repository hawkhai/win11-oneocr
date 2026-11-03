// 1804a4c90 FUN_1804a4c90

/* WARNING: Removing unreachable block (ram,0x0001804a582b) */
/* WARNING: Removing unreachable block (ram,0x0001804a5289) */
/* WARNING: Removing unreachable block (ram,0x0001804a523f) */
/* WARNING: Removing unreachable block (ram,0x0001804a5298) */
/* WARNING: Removing unreachable block (ram,0x0001804a57bd) */

void FUN_1804a4c90(longlong *param_1,longlong *param_2,longlong param_3)

{
  uint *puVar1;
  code *pcVar2;
  char cVar3;
  uint uVar4;
  ulonglong *puVar5;
  ulonglong *puVar6;
  ulonglong *puVar7;
  ulonglong uVar8;
  ulonglong uVar9;
  ulonglong *puVar10;
  ulonglong *puVar11;
  ulonglong uVar12;
  ulonglong uVar13;
  longlong lVar14;
  longlong lVar15;
  ulonglong uVar16;
  undefined1 auStack_158 [32];
  undefined8 uStack_138;
  ulonglong *puStack_128;
  ulonglong *puStack_120;
  ulonglong *puStack_118;
  ulonglong *puStack_110;
  ulonglong *puStack_108;
  ulonglong *puStack_100;
  undefined1 uStack_f8;
  longlong lStack_f0;
  undefined4 uStack_e8;
  longlong *plStack_e0;
  longlong *plStack_d8;
  ulonglong uStack_d0;
  ulonglong uStack_c8;
  ulonglong **ppuStack_c0;
  undefined8 uStack_b8;
  ulonglong uStack_b0;
  ulonglong *puStack_a8;
  ulonglong *puStack_a0;
  ulonglong uStack_98;
  longlong *plStack_90;
  ulonglong uStack_88;
  ulonglong uStack_80;
  ulonglong *puStack_78;
  ulonglong *puStack_70;
  ulonglong *puStack_68;
  longlong lStack_60;
  longlong lStack_58;
  undefined8 uStack_50;
  ulonglong uStack_48;
  ulonglong uStack_40;
  
  uStack_40 = DAT_18264bfc0 ^ (ulonglong)auStack_158;
  *param_1 = 0;
  param_1[1] = 0;
  *param_1 = 0;
  param_1[1] = 0;
  param_1[2] = 0;
  uStack_e8 = 1;
  uVar16 = param_2[1] - *param_2 >> 3;
  lStack_f0 = param_3;
  plStack_e0 = param_2;
  plStack_d8 = param_1;
  uStack_c8 = uVar16;
  plStack_90 = param_1;
  if (uVar16 != 0) {
    puStack_110 = (ulonglong *)0x0;
    puStack_108 = (ulonglong *)0x0;
    puStack_100 = (ulonglong *)0x0;
    puStack_128 = (ulonglong *)0x0;
    puStack_120 = (ulonglong *)0x0;
    puStack_118 = (ulonglong *)0x0;
    lStack_60 = 0;
    lStack_58 = 0;
    uStack_50 = 0;
    uStack_48 = 0;
    uStack_88 = uStack_88 & 0xffffffff00000000;
    uVar12 = uVar16 + 0x1f >> 5;
    FUN_18047c7f0(&lStack_60,uVar12,&uStack_88);
    uStack_48 = 0;
    if (0x7fffffffffffffff < uVar16) {
      FUN_18049db20();
LAB_1804a57c9:
      uStack_138 = 0;
      FUN_18040e3e4(0,0,0,0);
      pcVar2 = (code *)swi(3);
      (*pcVar2)();
      return;
    }
    if ((uVar12 < (ulonglong)(lStack_58 - lStack_60 >> 2)) &&
       (lVar14 = lStack_60 + uVar12 * 4, lVar14 != lStack_58)) {
      lStack_58 = lVar14;
    }
    if ((uVar16 & 0x1f) != 0) {
      puVar1 = (uint *)(lStack_60 + uVar12 * 4 + -4);
      *puVar1 = *puVar1 & (1 << ((byte)uVar16 & 0x1f)) - 1U;
    }
    uStack_48 = uVar16;
    if ((ulonglong)((longlong)puStack_100 - (longlong)puStack_110 >> 3) < uVar16) {
      if (0x1fffffffffffffff < uVar16) goto LAB_1804a57e8;
      lVar14 = (longlong)puStack_108 - (longlong)puStack_110;
      puVar5 = (ulonglong *)FUN_180440bd0(uVar16 * 8);
      FUN_1806a7f00(puVar5,puStack_110,(longlong)puStack_108 - (longlong)puStack_110);
      if (puStack_110 != (ulonglong *)0x0) {
        if ((0xfff < ((longlong)puStack_100 - (longlong)puStack_110 & 0xfffffffffffffff8U)) &&
           (0x1f < (longlong)puStack_110 + (-8 - puStack_110[-1]))) goto LAB_1804a57c9;
        FUN_1803eb2c8();
      }
      puStack_108 = puVar5 + (lVar14 >> 3);
      puStack_100 = puVar5 + uVar16;
      puStack_110 = puVar5;
    }
    if ((ulonglong)((longlong)puStack_118 - (longlong)puStack_128 >> 3) < uVar16) {
      if (0x1fffffffffffffff < uVar16) {
LAB_1804a57e8:
        FUN_180444e60();
LAB_1804a57ee:
        uStack_138 = 0;
        FUN_18040e3e4(0,0,0,0);
        pcVar2 = (code *)swi(3);
        (*pcVar2)();
        return;
      }
      lVar14 = (longlong)puStack_120 - (longlong)puStack_128;
      puVar5 = (ulonglong *)FUN_180440bd0(uVar16 * 8);
      FUN_1806a7f00(puVar5,puStack_128,(longlong)puStack_120 - (longlong)puStack_128);
      if (puStack_128 != (ulonglong *)0x0) {
        if ((0xfff < ((longlong)puStack_118 - (longlong)puStack_128 & 0xfffffffffffffff8U)) &&
           (0x1f < (longlong)puStack_128 + (-8 - puStack_128[-1]))) goto LAB_1804a57c9;
        FUN_1803eb2c8();
      }
      puStack_120 = puVar5 + (lVar14 >> 3);
      puStack_118 = puVar5 + uVar16;
      puStack_128 = puVar5;
    }
    uVar12 = 0;
    if (uVar16 != 0) {
      do {
        puVar6 = puStack_108;
        puVar5 = puStack_120;
        puVar1 = (uint *)(lStack_60 + (uVar12 >> 5) * 4);
        *puVar1 = *puVar1 & ~(1 << ((uint)uVar12 & 0x1f));
        if (*(char *)(*(longlong *)(*param_2 + uVar12 * 8) + 0x15a) == '\0') {
          if (puStack_120 == puStack_118) {
            lVar14 = (longlong)puStack_120 - (longlong)puStack_128 >> 3;
            if (lVar14 == 0x1fffffffffffffff) goto LAB_1804a57e8;
            uVar13 = lVar14 + 1;
            uVar8 = (longlong)puStack_118 - (longlong)puStack_128 >> 3;
            if (0x1fffffffffffffff - (uVar8 >> 1) < uVar8) {
              uVar9 = 0x1fffffffffffffff;
            }
            else {
              uVar8 = uVar8 + (uVar8 >> 1);
              uVar9 = uVar13;
              if (uVar13 <= uVar8) {
                uVar9 = uVar8;
              }
              if (0x1fffffffffffffff < uVar9) goto LAB_1804a57e2;
            }
            puVar6 = (ulonglong *)FUN_180440bd0(uVar9 * 8);
            puVar6[lVar14] = uVar12;
            if (puVar5 == puStack_120) {
              lVar15 = (longlong)puStack_120 - (longlong)puStack_128;
              puVar11 = puVar6;
              puVar5 = puStack_128;
            }
            else {
              FUN_1806a7f00(puVar6,puStack_128,(longlong)puVar5 - (longlong)puStack_128);
              lVar15 = (longlong)puStack_120 - (longlong)puVar5;
              puVar11 = puVar6 + lVar14 + 1;
            }
            FUN_1806a7f00(puVar11,puVar5,lVar15);
            if (puStack_128 != (ulonglong *)0x0) {
              if ((0xfff < ((longlong)puStack_118 - (longlong)puStack_128 & 0xfffffffffffffff8U)) &&
                 (0x1f < (longlong)puStack_128 + (-8 - puStack_128[-1]))) goto LAB_1804a57c9;
              FUN_1803eb2c8();
            }
            puStack_120 = puVar6 + uVar13;
            puStack_118 = puVar6 + uVar9;
            param_2 = plStack_e0;
            puStack_128 = puVar6;
          }
          else {
            *puStack_120 = uVar12;
            puStack_120 = puStack_120 + 1;
          }
        }
        else if (puStack_108 == puStack_100) {
          lVar14 = (longlong)puStack_108 - (longlong)puStack_110 >> 3;
          if (lVar14 == 0x1fffffffffffffff) goto LAB_1804a57e8;
          uVar13 = lVar14 + 1;
          uVar8 = (longlong)puStack_100 - (longlong)puStack_110 >> 3;
          if (0x1fffffffffffffff - (uVar8 >> 1) < uVar8) {
            uVar9 = 0x1fffffffffffffff;
          }
          else {
            uVar8 = uVar8 + (uVar8 >> 1);
            uVar9 = uVar13;
            if (uVar13 <= uVar8) {
              uVar9 = uVar8;
            }
            if (0x1fffffffffffffff < uVar9) {
LAB_1804a57e2:
              FUN_1804441f0();
              pcVar2 = (code *)swi(3);
              (*pcVar2)();
              return;
            }
          }
          puVar5 = (ulonglong *)FUN_180440bd0(uVar9 * 8);
          puVar5[lVar14] = uVar12;
          if (puVar6 == puStack_108) {
            lVar15 = (longlong)puStack_108 - (longlong)puStack_110;
            puVar11 = puVar5;
            puVar6 = puStack_110;
          }
          else {
            FUN_1806a7f00(puVar5,puStack_110,(longlong)puVar6 - (longlong)puStack_110);
            lVar15 = (longlong)puStack_108 - (longlong)puVar6;
            puVar11 = puVar5 + lVar14 + 1;
          }
          FUN_1806a7f00(puVar11,puVar6,lVar15);
          if (puStack_110 != (ulonglong *)0x0) {
            if ((0xfff < ((longlong)puStack_100 - (longlong)puStack_110 & 0xfffffffffffffff8U)) &&
               (0x1f < (longlong)puStack_110 + (-8 - puStack_110[-1]))) goto LAB_1804a57c9;
            FUN_1803eb2c8();
          }
          puStack_108 = puVar5 + uVar13;
          puStack_100 = puVar5 + uVar9;
          param_2 = plStack_e0;
          puStack_110 = puVar5;
        }
        else {
          *puStack_108 = uVar12;
          puStack_108 = puStack_108 + 1;
        }
        uVar12 = uVar12 + 1;
        param_3 = lStack_f0;
        param_1 = plStack_d8;
      } while (uVar12 < uVar16);
    }
    uStack_d0 = 0;
    if (uStack_c8 != 0) {
      do {
        uVar16 = uStack_d0;
        puVar1 = (uint *)(lStack_60 + (uStack_d0 >> 5) * 4);
        uVar4 = 1 << ((byte)uStack_d0 & 0x1f);
        if ((*puVar1 & uVar4) == 0) {
          puStack_78 = (ulonglong *)0x0;
          puStack_70 = (ulonglong *)0x0;
          puStack_68 = (ulonglong *)0x0;
          *puVar1 = *puVar1 | uVar4;
          puVar5 = (ulonglong *)0x0;
          puVar6 = (ulonglong *)FUN_180440bd0(8);
          *puVar6 = uVar16;
          if (puStack_70 == (ulonglong *)0x0) {
            puVar11 = puVar6;
            puVar5 = puStack_78;
            puVar10 = (ulonglong *)-(longlong)puStack_78;
          }
          else {
            FUN_1806a7f00(puVar6,puStack_78,-(longlong)puStack_78);
            puVar11 = puVar6 + 1;
            puVar10 = puStack_70;
          }
          FUN_1806a7f00(puVar11,puVar5,puVar10);
          if (puStack_78 != (ulonglong *)0x0) {
            if ((0xfff < ((longlong)puStack_68 - (longlong)puStack_78 & 0xfffffffffffffff8U)) &&
               (0x1f < (longlong)puStack_78 + (-8 - puStack_78[-1]))) goto LAB_1804a57ee;
            FUN_1803eb2c8();
          }
          puStack_70 = puVar6 + 1;
          puStack_68 = puVar6 + 1;
          uVar12 = 0;
          puStack_78 = puVar6;
          if ((longlong)puStack_70 - (longlong)puVar6 >> 3 != 0) {
            do {
              puVar10 = puStack_108;
              puVar11 = puStack_120;
              uStack_80 = puStack_78[uVar12];
              uStack_98 = uVar12 + 1;
              puVar5 = puStack_110;
              puVar6 = puStack_128;
              if (*(char *)(*(longlong *)(*plStack_e0 + uStack_80 * 8) + 0x15a) == '\0') {
                for (; puVar5 != puVar10; puVar5 = puVar5 + 1) {
                  uStack_88 = *puVar5;
                  if ((*(uint *)(lStack_60 + (uStack_88 >> 5) * 4) >> ((byte)uStack_88 & 0x1f) & 1)
                      == 0) {
                    if (*(longlong *)(param_3 + 0x38) == 0) {
                      FUN_1803e3fe0();
                      pcVar2 = (code *)swi(3);
                      (*pcVar2)();
                      return;
                    }
                    cVar3 = (*(code *)PTR__guard_dispatch_icall_1806db690)
                                      (*(longlong *)(param_3 + 0x38),&uStack_80,&uStack_88);
                    puVar6 = puStack_70;
                    if (cVar3 != '\0') {
                      puVar1 = (uint *)(lStack_60 + (uStack_88 >> 5) * 4);
                      *puVar1 = *puVar1 | 1 << ((uint)uStack_88 & 0x1f);
                      if (puStack_70 == puStack_68) {
                        lVar14 = (longlong)puStack_70 - (longlong)puStack_78 >> 3;
                        if (lVar14 == 0x1fffffffffffffff) {
                          FUN_180444e60();
                          pcVar2 = (code *)swi(3);
                          (*pcVar2)();
                          return;
                        }
                        uVar16 = lVar14 + 1;
                        uVar12 = (longlong)puStack_68 - (longlong)puStack_78 >> 3;
                        if (0x1fffffffffffffff - (uVar12 >> 1) < uVar12) {
                          uVar13 = 0x1fffffffffffffff;
                        }
                        else {
                          uVar12 = uVar12 + (uVar12 >> 1);
                          uVar13 = uVar16;
                          if (uVar16 <= uVar12) {
                            uVar13 = uVar12;
                          }
                          if (0x1fffffffffffffff < uVar13) {
                            FUN_1804441f0();
                            pcVar2 = (code *)swi(3);
                            (*pcVar2)();
                            return;
                          }
                        }
                        puVar7 = (ulonglong *)FUN_180440bd0(uVar13 * 8);
                        puStack_a8 = puVar7 + lVar14;
                        puVar11 = puStack_a8 + 1;
                        ppuStack_c0 = &puStack_78;
                        *puStack_a8 = uStack_88;
                        uStack_b0 = uVar13;
                        puStack_a0 = puVar11;
                        if (puVar6 == puStack_70) {
                          lVar14 = (longlong)puStack_70 - (longlong)puStack_78;
                          puVar11 = puVar7;
                          puVar6 = puStack_78;
                        }
                        else {
                          FUN_1806a7f00(puVar7,puStack_78,(longlong)puVar6 - (longlong)puStack_78);
                          lVar14 = (longlong)puStack_70 - (longlong)puVar6;
                          puStack_a8 = puVar7;
                        }
                        FUN_1806a7f00(puVar11,puVar6,lVar14);
                        uStack_b8 = 0;
                        FUN_18049a970(&puStack_78,puVar7,uVar16);
                        FUN_18048ba20(&ppuStack_c0);
                      }
                      else {
                        *puStack_70 = uStack_88;
                        puStack_70 = puStack_70 + 1;
                      }
                    }
                  }
                  param_3 = lStack_f0;
                }
              }
              else {
                for (; puVar6 != puVar11; puVar6 = puVar6 + 1) {
                  uStack_88 = *puVar6;
                  if ((*(uint *)(lStack_60 + (uStack_88 >> 5) * 4) >> ((byte)uStack_88 & 0x1f) & 1)
                      == 0) {
                    if (*(longlong *)(param_3 + 0x38) == 0) {
                      FUN_1803e3fe0();
                      pcVar2 = (code *)swi(3);
                      (*pcVar2)();
                      return;
                    }
                    cVar3 = (*(code *)PTR__guard_dispatch_icall_1806db690)
                                      (*(longlong *)(param_3 + 0x38),&uStack_80,&uStack_88);
                    puVar5 = puStack_70;
                    if (cVar3 != '\0') {
                      puVar1 = (uint *)(lStack_60 + (uStack_88 >> 5) * 4);
                      *puVar1 = *puVar1 | 1 << ((uint)uStack_88 & 0x1f);
                      if (puStack_70 == puStack_68) {
                        lVar14 = (longlong)puStack_70 - (longlong)puStack_78 >> 3;
                        if (lVar14 == 0x1fffffffffffffff) {
                          FUN_180444e60();
                          pcVar2 = (code *)swi(3);
                          (*pcVar2)();
                          return;
                        }
                        uVar16 = lVar14 + 1;
                        uVar12 = (longlong)puStack_68 - (longlong)puStack_78 >> 3;
                        if (0x1fffffffffffffff - (uVar12 >> 1) < uVar12) {
                          uVar13 = 0x1fffffffffffffff;
                        }
                        else {
                          uVar12 = uVar12 + (uVar12 >> 1);
                          uVar13 = uVar16;
                          if (uVar16 <= uVar12) {
                            uVar13 = uVar12;
                          }
                          if (0x1fffffffffffffff < uVar13) {
                            FUN_1804441f0();
                            pcVar2 = (code *)swi(3);
                            (*pcVar2)();
                            return;
                          }
                        }
                        puVar7 = (ulonglong *)FUN_180440bd0(uVar13 * 8);
                        puStack_a8 = puVar7 + lVar14;
                        puVar10 = puStack_a8 + 1;
                        ppuStack_c0 = &puStack_78;
                        *puStack_a8 = uStack_88;
                        uStack_b0 = uVar13;
                        puStack_a0 = puVar10;
                        if (puVar5 == puStack_70) {
                          lVar14 = (longlong)puStack_70 - (longlong)puStack_78;
                          puVar10 = puVar7;
                          puVar5 = puStack_78;
                        }
                        else {
                          FUN_1806a7f00(puVar7,puStack_78,(longlong)puVar5 - (longlong)puStack_78);
                          lVar14 = (longlong)puStack_70 - (longlong)puVar5;
                          puStack_a8 = puVar7;
                        }
                        FUN_1806a7f00(puVar10,puVar5,lVar14);
                        uStack_b8 = 0;
                        FUN_18049a970(&puStack_78,puVar7,uVar16);
                        FUN_18048ba20(&ppuStack_c0);
                      }
                      else {
                        *puStack_70 = uStack_88;
                        puStack_70 = puStack_70 + 1;
                      }
                    }
                  }
                  param_3 = lStack_f0;
                }
              }
              param_3 = lStack_f0;
              uVar16 = uStack_d0;
              uVar12 = uStack_98;
            } while (uStack_98 < (ulonglong)((longlong)puStack_70 - (longlong)puStack_78 >> 3));
          }
          param_1 = plStack_d8;
          lVar14 = plStack_d8[1];
          if (lVar14 == plStack_d8[2]) {
            FUN_1804a2b50(plStack_d8,lVar14,&puStack_78);
          }
          else {
            FUN_1804889a0(lVar14,&puStack_78);
            param_1[1] = param_1[1] + 0x18;
          }
          FUN_180442d40(&puStack_78);
        }
        uStack_d0 = uVar16 + 1;
        param_3 = lStack_f0;
      } while (uStack_d0 < uStack_c8);
    }
    FUN_1804a4490(*param_1,param_1[1],(param_1[1] - *param_1 >> 3) * -0x5555555555555555,uStack_f8);
    FUN_1804442f0(&lStack_60);
    FUN_180442d40(&puStack_128);
    FUN_180442d40(&puStack_110);
  }
  FUN_1803eae20(uStack_40 ^ (ulonglong)auStack_158);
  return;
}

