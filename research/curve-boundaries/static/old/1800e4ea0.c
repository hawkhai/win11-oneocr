// 1800e4ea0 FUN_1800e4ea0

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_1800e4ea0(int param_1,double *param_2,longlong param_3,int param_4,longlong *param_5)

{
  double dVar1;
  code *pcVar2;
  double dVar3;
  undefined8 **ppuVar4;
  longlong lVar5;
  undefined8 ***pppuVar6;
  undefined8 ***pppuVar7;
  longlong *plVar8;
  float *pfVar9;
  undefined8 ***pppuVar10;
  undefined8 ***pppuVar11;
  longlong lVar12;
  undefined8 *puVar13;
  longlong lVar14;
  int iVar15;
  double dVar16;
  undefined1 auStack_158 [32];
  undefined8 **local_138;
  undefined8 **local_130;
  undefined8 **local_128;
  undefined8 **local_120;
  undefined8 **local_118;
  undefined8 **ppuStack_110;
  undefined8 local_108;
  ulonglong uStack_100;
  longlong local_f8;
  undefined8 **local_f0;
  undefined8 **ppuStack_e8;
  undefined8 local_e0;
  ulonglong uStack_d8;
  longlong local_d0;
  undefined8 **local_c8;
  longlong lStack_c0;
  undefined8 local_b8;
  undefined8 **ppuStack_b0;
  longlong local_a8;
  undefined8 **local_a0;
  longlong lStack_98;
  undefined8 local_90;
  ulonglong uStack_88;
  longlong local_80;
  undefined8 **local_78;
  undefined8 **ppuStack_70;
  undefined8 local_68;
  ulonglong uStack_60;
  longlong local_58;
  undefined8 local_50;
  undefined8 **ppuStack_48;
  longlong local_40;
  ulonglong local_38;
  
  local_38 = DAT_182625200 ^ (ulonglong)auStack_158;
  pppuVar6 = (undefined8 ***)(longlong)param_1;
  iVar15 = param_4;
  if (param_1 <= param_4) {
    iVar15 = param_1 + -1;
  }
  local_108 = 0;
  local_f8 = 0;
  ppuStack_110 = (undefined8 **)(longlong)(iVar15 + 1);
  uStack_100 = (longlong)pppuVar6 * (longlong)ppuStack_110;
  pppuVar10 = (undefined8 ***)0x0;
  local_118 = pppuVar6;
  if (uStack_100 != 0) {
    if (uStack_100 < 0x2000000000000000) {
      local_f8 = FUN_18002b510(uStack_100 * 8);
      goto LAB_1800e4f65;
    }
    FUN_180027fd0();
LAB_1800e573e:
    FUN_180027fd0();
LAB_1800e5744:
    FUN_180027fd0();
LAB_1800e574a:
    FUN_180027fd0();
LAB_1800e5750:
    FUN_180027fd0();
LAB_1800e5756:
    FUN_180027fd0();
    pcVar2 = (code *)swi(3);
    (*pcVar2)();
    return;
  }
  local_f8 = 0;
LAB_1800e4f65:
  local_b8 = 0;
  local_a8 = 0;
  lStack_c0 = 1;
  local_c8 = pppuVar6;
  ppuStack_b0 = pppuVar6;
  if (pppuVar6 == (undefined8 ***)0x0) {
    local_a8 = 0;
  }
  else {
    if ((undefined8 ***)0x1fffffffffffffff < pppuVar6) goto LAB_1800e573e;
    local_a8 = FUN_18002b510((longlong)pppuVar6 * 8);
  }
  pppuVar11 = pppuVar10;
  dVar3 = DAT_182576418;
  if ((undefined8 ***)0x3 < pppuVar6) {
    lVar14 = 2;
    puVar13 = (undefined8 *)(param_3 + 0x10);
    do {
      *(undefined8 *)(local_a8 + lStack_c0 * (longlong)pppuVar11 * 8) = puVar13[-2];
      *(undefined8 *)(local_a8 + (lVar14 + -1) * lStack_c0 * 8) = puVar13[-1];
      *(undefined8 *)(local_a8 + lVar14 * lStack_c0 * 8) = *puVar13;
      *(undefined8 *)(local_a8 + (lVar14 + 1) * lStack_c0 * 8) = puVar13[1];
      pppuVar11 = (undefined8 ***)((longlong)pppuVar11 + 4);
      puVar13 = puVar13 + 4;
      lVar14 = lVar14 + 4;
      dVar3 = DAT_182576418;
    } while (pppuVar11 < (undefined8 ***)((longlong)pppuVar6 - 3U));
  }
  for (; DAT_182576418 = dVar3, pppuVar11 < pppuVar6;
      pppuVar11 = (undefined8 ***)((longlong)pppuVar11 + 1)) {
    *(undefined8 *)(local_a8 + lStack_c0 * (longlong)pppuVar11 * 8) =
         *(undefined8 *)(param_3 + (longlong)pppuVar11 * 8);
    dVar3 = DAT_182576418;
  }
  lVar14 = (longlong)iVar15;
  pppuVar11 = pppuVar10;
  if (pppuVar6 != (undefined8 ***)0x0) {
    do {
      if (-1 < iVar15) {
        pppuVar7 = pppuVar10;
        dVar16 = dVar3;
        if (2 < lVar14) {
          do {
            *(double *)
             (local_f8 +
             (longlong)((longlong)ppuStack_110 * (longlong)pppuVar11 + (longlong)pppuVar7) * 8) =
                 dVar16;
            dVar1 = *param_2;
            *(double *)
             (local_f8 + 8 +
             (longlong)((longlong)ppuStack_110 * (longlong)pppuVar11 + (longlong)pppuVar7) * 8) =
                 dVar16 * dVar1;
            dVar16 = dVar16 * dVar1 * *param_2;
            *(double *)
             (local_f8 + 0x10 +
             (longlong)((longlong)ppuStack_110 * (longlong)pppuVar11 + (longlong)pppuVar7) * 8) =
                 dVar16;
            dVar16 = dVar16 * *param_2;
            *(double *)
             (local_f8 + 0x18 +
             (longlong)((longlong)ppuStack_110 * (longlong)pppuVar11 + (longlong)pppuVar7) * 8) =
                 dVar16;
            dVar16 = dVar16 * *param_2;
            pppuVar7 = (undefined8 ***)((longlong)pppuVar7 + 4);
          } while ((longlong)pppuVar7 <= lVar14 + -3);
        }
        for (; (longlong)pppuVar7 <= lVar14; pppuVar7 = (undefined8 ***)((longlong)pppuVar7 + 1)) {
          *(double *)
           (local_f8 +
           (longlong)((longlong)ppuStack_110 * (longlong)pppuVar11 + (longlong)pppuVar7) * 8) =
               dVar16;
          dVar16 = dVar16 * *param_2;
        }
      }
      pppuVar11 = (undefined8 ***)((longlong)pppuVar11 + 1);
      param_2 = param_2 + 1;
    } while (pppuVar11 < pppuVar6);
  }
  local_e0 = 0;
  local_d0 = 0;
  local_f0 = ppuStack_110;
  ppuStack_e8 = local_118;
  uStack_d8 = (longlong)ppuStack_110 * (longlong)local_118;
  if (uStack_d8 == 0) {
    local_d0 = 0;
  }
  else {
    if (0x1fffffffffffffff < uStack_d8) goto LAB_1800e5744;
    local_d0 = FUN_18002b510(uStack_d8 * 8);
  }
  pppuVar6 = pppuVar10;
  if ((undefined8 ***)local_f0 != (undefined8 ***)0x0) {
    do {
      pppuVar11 = pppuVar10;
      if ((undefined8 ***)0x3 < ppuStack_e8) {
        do {
          *(undefined8 *)
           (local_d0 +
           (longlong)((longlong)ppuStack_e8 * (longlong)pppuVar6 + (longlong)pppuVar11) * 8) =
               *(undefined8 *)
                (local_f8 +
                (longlong)((longlong)ppuStack_110 * (longlong)pppuVar11 + (longlong)pppuVar6) * 8);
          *(undefined8 *)
           (local_d0 + 8 +
           (longlong)((longlong)ppuStack_e8 * (longlong)pppuVar6 + (longlong)pppuVar11) * 8) =
               *(undefined8 *)
                (local_f8 +
                (longlong)(((longlong)pppuVar11 + 1U) * (longlong)ppuStack_110 + (longlong)pppuVar6)
                * 8);
          *(undefined8 *)
           (local_d0 + 0x10 +
           (longlong)((longlong)ppuStack_e8 * (longlong)pppuVar6 + (longlong)pppuVar11) * 8) =
               *(undefined8 *)
                (local_f8 +
                (longlong)((longlong)ppuStack_110 * ((longlong)pppuVar11 + 2U) + (longlong)pppuVar6)
                * 8);
          *(undefined8 *)
           (local_d0 + 0x18 +
           (longlong)((longlong)ppuStack_e8 * (longlong)pppuVar6 + (longlong)pppuVar11) * 8) =
               *(undefined8 *)
                (local_f8 +
                (longlong)(((longlong)pppuVar11 + 3U) * (longlong)ppuStack_110 + (longlong)pppuVar6)
                * 8);
          pppuVar11 = (undefined8 ***)((longlong)pppuVar11 + 4);
        } while (pppuVar11 < (undefined8 ***)((longlong)ppuStack_e8 - 3U));
      }
      for (; pppuVar11 < ppuStack_e8; pppuVar11 = (undefined8 ***)((longlong)pppuVar11 + 1)) {
        *(undefined8 *)
         (local_d0 +
         (longlong)((longlong)ppuStack_e8 * (longlong)pppuVar6 + (longlong)pppuVar11) * 8) =
             *(undefined8 *)
              (local_f8 +
              (longlong)((longlong)ppuStack_110 * (longlong)pppuVar11 + (longlong)pppuVar6) * 8);
      }
      pppuVar6 = (undefined8 ***)((longlong)pppuVar6 + 1);
    } while (pppuVar6 < local_f0);
  }
  local_68 = 0;
  local_58 = 0;
  local_138 = &local_f0;
  local_130 = &local_118;
  ppuStack_70 = ppuStack_110;
  uStack_60 = (longlong)ppuStack_110 * (longlong)local_f0;
  local_78 = local_f0;
  if (uStack_60 == 0) {
    local_58 = 0;
  }
  else {
    if (0x1fffffffffffffff < uStack_60) goto LAB_1800e574a;
    local_58 = FUN_18002b510(uStack_60 * 8);
  }
  FUN_1800e7000(&local_78,&local_138);
  local_90 = 0;
  local_80 = 0;
  local_138 = &local_f0;
  local_130 = &local_c8;
  local_a0 = local_f0;
  lStack_98 = lStack_c0;
  uStack_88 = lStack_c0 * (longlong)local_f0;
  if (uStack_88 == 0) {
    local_80 = 0;
  }
  else {
    if (0x1fffffffffffffff < uStack_88) goto LAB_1800e5750;
    local_80 = FUN_18002b510(uStack_88 * 8);
  }
  FUN_1800e7000(&local_a0,&local_138);
  ppuVar4 = local_78;
  local_50 = 0;
  local_40 = 0;
  ppuStack_48 = local_78;
  if ((undefined8 ***)local_78 == (undefined8 ***)0x0) {
    local_40 = 0;
  }
  else {
    if ((undefined8 ***)0x1fffffffffffffff < local_78) goto LAB_1800e5756;
    local_40 = FUN_18002b510((longlong)local_78 * 8);
  }
  lVar5 = _UNK_182576648;
  lVar14 = _DAT_182576640;
  if ((undefined8 ***)ppuVar4 != (undefined8 ***)0x0) {
    pppuVar6 = pppuVar10;
    if ((undefined8 ***)0x7 < ppuVar4) {
      lVar12 = 4;
      plVar8 = (longlong *)(local_40 + 0x20);
      do {
        plVar8[-4] = (longlong)pppuVar6 + lVar14;
        plVar8[-3] = (longlong)pppuVar6 + lVar5;
        plVar8[-2] = lVar12 + -2 + lVar14;
        plVar8[-1] = lVar12 + -2 + lVar5;
        *plVar8 = lVar12 + lVar14;
        plVar8[1] = lVar12 + lVar5;
        plVar8[2] = lVar12 + 2 + lVar14;
        plVar8[3] = lVar12 + 2 + lVar5;
        pppuVar6 = pppuVar6 + 1;
        plVar8 = plVar8 + 8;
        lVar12 = lVar12 + 8;
      } while (pppuVar6 < (undefined8 ***)((ulonglong)ppuVar4 & 0xfffffffffffffff8));
      if (ppuVar4 <= pppuVar6) goto LAB_1800e54ac;
    }
    do {
      *(undefined8 ****)(local_40 + (longlong)pppuVar6 * 8) = pppuVar6;
      pppuVar6 = (undefined8 ***)((longlong)pppuVar6 + 1);
    } while (pppuVar6 < ppuVar4);
  }
LAB_1800e54ac:
  lVar5 = FUN_1800e6a50(&local_78,&local_50);
  lVar14 = local_40;
  ppuVar4 = ppuStack_48;
  if (lVar5 == 0) {
    pppuVar6 = pppuVar10;
    if ((undefined8 ***)ppuStack_48 != (undefined8 ***)0x0) {
      do {
        pppuVar11 = *(undefined8 ****)(lVar14 + (longlong)pppuVar6 * 8);
        if (pppuVar6 != pppuVar11) {
          local_128 = &local_a0;
          local_138 = &local_a0;
          local_130 = pppuVar11;
          local_120 = pppuVar6;
          FUN_1800e6da0(&local_128,&local_138);
        }
        pppuVar6 = (undefined8 ***)((longlong)pppuVar6 + 1);
      } while (pppuVar6 < ppuVar4);
    }
    local_138 = &local_78;
    FUN_1800e7210(&local_138,&local_a0);
    FUN_1800e74a0(&local_78,&local_a0);
    local_138 = (undefined8 **)((ulonglong)local_138 & 0xffffffff00000000);
    FUN_1800cee30(param_5,(longlong)(param_4 + 1),&local_138);
    if ((undefined8 ***)local_a0 != (undefined8 ***)0x0) {
      pfVar9 = (float *)*param_5;
      do {
        *pfVar9 = (float)*(double *)(local_80 + lStack_98 * (longlong)pppuVar10 * 8);
        pppuVar10 = (undefined8 ***)((longlong)pppuVar10 + 1);
        pfVar9 = pfVar9 + 1;
      } while (pppuVar10 < local_a0);
    }
  }
  if ((undefined8 ***)ppuVar4 != (undefined8 ***)0x0) {
    lVar5 = lVar14;
    if ((0xfff < (ulonglong)((longlong)ppuVar4 * 8)) &&
       (lVar5 = *(longlong *)(lVar14 + -8), 0x1f < (lVar14 - lVar5) - 8U)) {
                    /* WARNING: Subroutine does not return */
      FUN_180665fe4();
    }
    thunk_FUN_180686960(lVar5);
  }
  if (uStack_88 != 0) {
    if (0xfff < uStack_88 * 8) {
      if (0x1f < (local_80 - *(longlong *)(local_80 + -8)) - 8U) {
                    /* WARNING: Subroutine does not return */
        FUN_180665fe4(*(longlong *)(local_80 + -8),uStack_88 * 8 + 0x27);
      }
    }
    thunk_FUN_180686960();
  }
  if (uStack_60 != 0) {
    if (0xfff < uStack_60 * 8) {
      if (0x1f < (local_58 - *(longlong *)(local_58 + -8)) - 8U) {
                    /* WARNING: Subroutine does not return */
        FUN_180665fe4(*(longlong *)(local_58 + -8),uStack_60 * 8 + 0x27);
      }
    }
    thunk_FUN_180686960();
  }
  if (uStack_d8 != 0) {
    if (0xfff < uStack_d8 * 8) {
      if (0x1f < (local_d0 - *(longlong *)(local_d0 + -8)) - 8U) {
                    /* WARNING: Subroutine does not return */
        FUN_180665fe4(*(longlong *)(local_d0 + -8),uStack_d8 * 8 + 0x27);
      }
    }
    thunk_FUN_180686960();
  }
  if ((undefined8 ***)ppuStack_b0 != (undefined8 ***)0x0) {
    if (0xfff < (ulonglong)((longlong)ppuStack_b0 * 8)) {
      if (0x1f < (local_a8 - *(longlong *)(local_a8 + -8)) - 8U) {
                    /* WARNING: Subroutine does not return */
        FUN_180665fe4(*(longlong *)(local_a8 + -8),(longlong)ppuStack_b0 * 8 + 0x27);
      }
    }
    thunk_FUN_180686960();
  }
  if (uStack_100 != 0) {
    if (0xfff < uStack_100 * 8) {
      if (0x1f < (local_f8 - *(longlong *)(local_f8 + -8)) - 8U) {
                    /* WARNING: Subroutine does not return */
        FUN_180665fe4(*(longlong *)(local_f8 + -8),uStack_100 * 8 + 0x27);
      }
    }
    thunk_FUN_180686960();
  }
  FUN_180642b80(local_38 ^ (ulonglong)auStack_158);
  return;
}

