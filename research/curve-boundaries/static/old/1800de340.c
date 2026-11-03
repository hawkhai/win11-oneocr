// 1800de340 FUN_1800de340

float * FUN_1800de340(char *param_1,float *param_2,float *param_3)

{
  float fVar1;
  float *pfVar2;
  double *pdVar3;
  int iVar4;
  double *pdVar5;
  int iVar6;
  undefined8 uVar7;
  int iVar8;
  ulonglong uVar9;
  longlong lVar10;
  longlong *plVar11;
  int iVar12;
  float fVar13;
  float fVar14;
  float fVar15;
  float fVar16;
  int iStack_fc;
  longlong local_f8;
  undefined1 local_f0 [136];
  undefined **local_68 [12];
  
  if ((NAN(*param_3)) || (NAN(param_3[1]))) {
    uVar7 = FUN_1802a58b0(&local_f8,1);
    FUN_18002c0a0(uVar7);
    FUN_1800679c0(local_f0);
    *(undefined ***)(local_f0 + (longlong)*(int *)(local_f8 + 4) + -8) =
         std::basic_ostream<char,struct_std::char_traits<char>_>::vftable;
    *(int *)((longlong)&iStack_fc + (longlong)*(int *)(local_f8 + 4)) =
         *(int *)(local_f8 + 4) + -0x10;
    local_68[0] = std::ios_base::vftable;
    std::ios_base::_Ios_base_dtor((ios_base *)local_68);
  }
  if (*param_1 == '\0') {
    uVar9 = 0;
    pdVar3 = *(double **)(param_1 + 0x40);
    pdVar5 = pdVar3;
    if (*(longlong *)(param_1 + 0x48) - (longlong)pdVar3 >> 3 != 1) {
      do {
        if ((double)*param_3 < pdVar5[1]) break;
        uVar9 = uVar9 + 1;
        pdVar5 = pdVar5 + 1;
      } while (uVar9 < (*(longlong *)(param_1 + 0x48) - (longlong)pdVar3 >> 3) - 1U);
    }
    lVar10 = *(longlong *)(param_1 + 0x28);
    fVar13 = param_3[1];
    fVar14 = *param_3;
    fVar16 = fVar13 * *(float *)(lVar10 + 4 + uVar9 * 0x24) +
             fVar14 * *(float *)(lVar10 + uVar9 * 0x24) + *(float *)(lVar10 + 8 + uVar9 * 0x24);
    fVar15 = fVar13 * *(float *)(lVar10 + 0x10 + uVar9 * 0x24) +
             fVar14 * *(float *)(lVar10 + 0xc + uVar9 * 0x24) +
             *(float *)(lVar10 + 0x14 + uVar9 * 0x24);
    fVar1 = *(float *)(lVar10 + 0x18 + uVar9 * 0x24);
    fVar13 = fVar1 * fVar14 + fVar1 * fVar13 + *(float *)(lVar10 + 0x20 + uVar9 * 0x24);
    if (fVar13 != 0.0) {
      fVar16 = fVar16 / fVar13;
      fVar15 = fVar15 / fVar13;
    }
  }
  else {
    plVar11 = *(longlong **)(param_1 + 0x58);
    iVar6 = (int)((*(longlong *)(param_1 + 0x60) - (longlong)plVar11) / 0x18);
    iVar8 = 0;
    iVar4 = 0;
    if (0 < iVar6 + -2) {
      iVar4 = iVar6 + -2;
    }
    iVar12 = iVar4;
    if (0 < iVar6) {
      lVar10 = 0;
      do {
        iVar12 = iVar8;
        if (*param_3 <= *(float *)(*plVar11 + 8)) break;
        iVar8 = iVar8 + 1;
        lVar10 = lVar10 + 1;
        plVar11 = plVar11 + 3;
        iVar12 = iVar4;
      } while (lVar10 < iVar6);
    }
    pfVar2 = *(float **)(*(longlong *)(param_1 + 0x58) + (longlong)iVar12 * 0x18);
    iVar6 = iVar12 + 1;
    if (param_3[1] <= ((pfVar2[5] - pfVar2[1]) * (pfVar2[2] - *param_3)) / (pfVar2[2] - *pfVar2)) {
      iVar6 = iVar12;
    }
    pfVar2 = *(float **)(*(longlong *)(param_1 + 0x70) + (longlong)iVar6 * 0x18);
    fVar13 = (*param_3 - pfVar2[8]) - pfVar2[4];
    fVar14 = (param_3[1] - pfVar2[9]) - pfVar2[5];
    fVar16 = fVar14 * pfVar2[1] + fVar13 * *pfVar2 + pfVar2[6];
    fVar15 = fVar14 * pfVar2[3] + fVar13 * pfVar2[2] + pfVar2[7];
  }
  fVar13 = *(float *)(param_1 + 8);
  *param_2 = fVar16 + *(float *)(param_1 + 4);
  param_2[1] = fVar15 + fVar13;
  return param_2;
}

