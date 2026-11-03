// 18052f770 FUN_18052f770

float * FUN_18052f770(char *param_1,float *param_2,float *param_3)

{
  float fVar1;
  longlong lVar2;
  double *pdVar3;
  code *pcVar4;
  double *pdVar5;
  int iVar6;
  int iVar7;
  undefined8 uVar8;
  ulonglong *puVar9;
  float *pfVar10;
  int iVar11;
  longlong *plVar12;
  ulonglong uVar13;
  ulonglong uVar14;
  float fVar15;
  float fVar16;
  float fVar17;
  float fVar18;
  int iStack_fc;
  longlong lStack_f8;
  undefined1 auStack_f0 [136];
  undefined *apuStack_68 [12];
  
  if ((NAN(*param_3)) || (NAN(param_3[1]))) {
    uVar8 = FUN_1800db1f0(&lStack_f8,1);
    FUN_18045d4c0(uVar8);
    FUN_18048a6b0(auStack_f0);
    *(undefined **)(auStack_f0 + (longlong)*(int *)(lStack_f8 + 4) + -8) = &DAT_1806e7c60;
    *(int *)((longlong)&iStack_fc + (longlong)*(int *)(lStack_f8 + 4)) =
         *(int *)(lStack_f8 + 4) + -0x10;
    apuStack_68[0] = &DAT_1806e62b0;
    FUN_1803e4954(apuStack_68);
  }
  if (param_1[1] == '\0') {
    uVar13 = 0;
    if (*param_1 == '\0') {
      pdVar3 = *(double **)(param_1 + 0x40);
      pdVar5 = pdVar3;
      if (*(longlong *)(param_1 + 0x48) - (longlong)pdVar3 >> 3 != 1) {
        do {
          if ((double)*param_3 < pdVar5[1]) break;
          uVar13 = uVar13 + 1;
          pdVar5 = pdVar5 + 1;
        } while (uVar13 < (*(longlong *)(param_1 + 0x48) - (longlong)pdVar3 >> 3) - 1U);
      }
      fVar15 = *param_3;
      fVar16 = param_3[1];
      lVar2 = *(longlong *)(param_1 + 0x28);
      fVar18 = fVar16 * *(float *)(lVar2 + 4 + uVar13 * 0x24) +
               fVar15 * *(float *)(lVar2 + uVar13 * 0x24) + *(float *)(lVar2 + 8 + uVar13 * 0x24);
      fVar17 = fVar16 * *(float *)(lVar2 + 0x10 + uVar13 * 0x24) +
               fVar15 * *(float *)(lVar2 + 0xc + uVar13 * 0x24) +
               *(float *)(lVar2 + 0x14 + uVar13 * 0x24);
      fVar1 = *(float *)(lVar2 + 0x18 + uVar13 * 0x24);
      fVar15 = fVar1 * fVar16 + fVar1 * fVar15 + *(float *)(lVar2 + 0x20 + uVar13 * 0x24);
      if (fVar15 != 0.0) {
        fVar18 = fVar18 / fVar15;
        fVar17 = fVar17 / fVar15;
      }
    }
    else {
      plVar12 = *(longlong **)(param_1 + 0x58);
      iVar7 = (int)(*(longlong *)(param_1 + 0x60) - (longlong)plVar12 >> 3) * -0x55555555;
      iVar11 = iVar7 + -2;
      if (iVar7 + -2 < 1) {
        iVar11 = 0;
      }
      iVar6 = iVar11;
      if (0 < iVar7) {
        uVar14 = uVar13;
        do {
          iVar6 = (int)uVar14;
          if (*param_3 <= *(float *)(*plVar12 + 8)) break;
          uVar14 = (ulonglong)((int)uVar14 + 1);
          uVar13 = uVar13 + 1;
          plVar12 = plVar12 + 3;
          iVar6 = iVar11;
        } while ((longlong)uVar13 < (longlong)iVar7);
      }
      pfVar10 = *(float **)(*(longlong *)(param_1 + 0x58) + (longlong)iVar6 * 0x18);
      iVar7 = iVar6 + 1;
      if (param_3[1] <=
          ((pfVar10[5] - pfVar10[1]) * (pfVar10[2] - *param_3)) / (pfVar10[2] - *pfVar10)) {
        iVar7 = iVar6;
      }
      pfVar10 = *(float **)(*(longlong *)(param_1 + 0x70) + (longlong)iVar7 * 0x18);
      fVar15 = (*param_3 - pfVar10[8]) - pfVar10[4];
      fVar16 = (param_3[1] - pfVar10[9]) - pfVar10[5];
      fVar18 = fVar16 * pfVar10[1] + fVar15 * *pfVar10 + pfVar10[6];
      fVar17 = fVar16 * pfVar10[3] + fVar15 * pfVar10[2] + pfVar10[7];
    }
    fVar15 = *(float *)(param_1 + 8);
    *param_2 = fVar18 + *(float *)(param_1 + 4);
    param_2[1] = fVar17 + fVar15;
  }
  else {
    iVar7 = (int)(*(longlong *)(param_1 + 0xa0) - *(longlong *)(param_1 + 0x98) >> 3);
    iVar11 = iVar7 + -1;
    iVar6 = (int)*param_3;
    if (iVar6 <= iVar11) {
      iVar11 = iVar6;
    }
    iVar6 = 0;
    if (0 < iVar11) {
      iVar6 = iVar11;
    }
    fVar15 = *param_3 / (float)iVar7;
    if (param_3[1] == 0.0) {
      puVar9 = (ulonglong *)(*(longlong *)(param_1 + 0x98) + (longlong)iVar6 * 8);
      fVar16 = 0.0;
    }
    else {
      puVar9 = (ulonglong *)(*(longlong *)(param_1 + 0xb0) + (longlong)iVar6 * 8);
      fVar16 = DAT_18258fae8;
    }
    uVar13 = *puVar9;
    if (*(ulonglong *)(param_1 + 0x88) <= uVar13) {
      Unwind_1804b6dd0();
      pcVar4 = (code *)swi(3);
      pfVar10 = (float *)(*pcVar4)();
      return pfVar10;
    }
    lVar2 = *(longlong *)(param_1 + 0x90);
    param_2[0] = 0.0;
    param_2[1] = 0.0;
    *param_2 = fVar16 * *(float *)(lVar2 + 8 + uVar13 * 0x18) +
               fVar15 * *(float *)(lVar2 + uVar13 * 0x18) + *(float *)(lVar2 + 0x10 + uVar13 * 0x18)
    ;
    param_2[1] = fVar16 * *(float *)(lVar2 + 0xc + uVar13 * 0x18) +
                 fVar15 * *(float *)(lVar2 + 4 + uVar13 * 0x18) +
                 *(float *)(lVar2 + 0x14 + uVar13 * 0x18);
  }
  return param_2;
}

