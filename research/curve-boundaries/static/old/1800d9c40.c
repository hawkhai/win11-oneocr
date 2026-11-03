// 1800d9c40 FUN_1800d9c40

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_1800d9c40(longlong param_1,float param_2,longlong param_3,longlong *param_4,
                  longlong param_5,longlong *param_6,undefined4 *param_7,longlong *param_8)

{
  int iVar1;
  float *pfVar2;
  undefined4 *puVar3;
  longlong lVar4;
  longlong lVar5;
  longlong lVar6;
  int iVar7;
  longlong lVar8;
  longlong lVar9;
  ulonglong uVar10;
  int iVar11;
  ulonglong uVar12;
  float fVar13;
  double dVar14;
  undefined1 auVar15 [16];
  float fVar16;
  float fVar17;
  float fVar18;
  float local_78 [20];
  
  *param_7 = *(undefined4 *)(param_1 + 0x10);
  iVar1 = *(int *)(param_1 + 0x10);
  iVar11 = 0x600;
  if (0 < *(int *)(param_5 + 4)) {
    iVar11 = *(int *)(param_5 + 4);
  }
  iVar7 = 0x300;
  if (0 < *(int *)(param_5 + 8)) {
    iVar7 = *(int *)(param_5 + 8);
  }
  auVar15._0_8_ = (double)(iVar11 * iVar11 + iVar7 * iVar7);
  auVar15._8_8_ = 0;
  if (auVar15._0_8_ < 0.0) {
    dVar14 = (double)FUN_18067acc0();
  }
  else {
    auVar15 = sqrtpd(ZEXT816(0),auVar15);
    dVar14 = auVar15._0_8_;
  }
  lVar8 = (longlong)(dVar14 * _DAT_182576468 * (double)(iVar1 / 2) + DAT_182576418);
  iVar11 = 1000000;
  if (lVar8 < 1000000) {
    iVar11 = (int)lVar8;
  }
  uVar12 = 0;
  if (param_3 != 1) {
    fVar17 = 0.0;
    fVar18 = (float)iVar11 - DAT_1825763b0;
    uVar10 = uVar12;
    do {
      lVar8 = *param_4;
      fVar13 = *(float *)(lVar8 + 4 + uVar10 * 8) - *(float *)(lVar8 + 0xc + uVar10 * 8);
      fVar16 = *(float *)(lVar8 + uVar10 * 8) - *(float *)(lVar8 + 8 + uVar10 * 8);
      fVar13 = fVar16 * fVar16 + fVar13 * fVar13;
      if (fVar13 < 0.0) {
        fVar13 = (float)FUN_18067c840();
      }
      else {
        fVar13 = SQRT(fVar13);
      }
      pfVar2 = (float *)param_6[1];
      fVar17 = fVar13 * ((float)iVar1 / param_2) + fVar17;
      if (fVar18 < fVar17) {
        if (pfVar2 == (float *)param_6[2]) {
          local_78[0] = fVar18;
          FUN_180072fe0(param_6,pfVar2,local_78);
        }
        else {
          *pfVar2 = fVar18;
          param_6[1] = (longlong)(pfVar2 + 1);
        }
        param_3 = uVar10 + 1;
        break;
      }
      local_78[0] = fVar17;
      if (pfVar2 == (float *)param_6[2]) {
        FUN_180072fe0(param_6,pfVar2,local_78);
      }
      else {
        *pfVar2 = fVar17;
        param_6[1] = (longlong)(pfVar2 + 1);
      }
      uVar10 = uVar10 + 1;
    } while (uVar10 < param_3 - 1U);
  }
  lVar8 = param_4[1];
  uVar10 = param_3 - 1;
  lVar9 = *param_4;
  puVar3 = (undefined4 *)*param_8;
  *puVar3 = 0;
  puVar3[1] = 0;
  lVar8 = (longlong)(int)(lVar8 - lVar9 >> 3);
  if (uVar10 < 4) {
    if (param_3 == 1) goto LAB_1800d9fe5;
  }
  else {
    lVar9 = 0x18;
    lVar6 = lVar8 * 8;
    do {
      lVar9 = lVar9 + 0x20;
      lVar4 = *param_8;
      *(undefined4 *)(lVar4 + -0x30 + lVar9) = *(undefined4 *)(*param_6 + uVar12 * 4);
      *(undefined4 *)(lVar4 + -0x2c + lVar9) = 0;
      iVar1 = *(int *)(param_1 + 0x10);
      lVar4 = *param_8;
      *(undefined4 *)(lVar6 + -0x10 + lVar4) = *(undefined4 *)(*param_6 + uVar12 * 4);
      *(float *)(lVar6 + -0xc + lVar4) = (float)(iVar1 + -1);
      lVar4 = *param_8;
      *(undefined4 *)(lVar4 + -0x28 + lVar9) = *(undefined4 *)(*param_6 + 4 + uVar12 * 4);
      *(undefined4 *)(lVar4 + -0x24 + lVar9) = 0;
      iVar1 = *(int *)(param_1 + 0x10);
      lVar4 = *param_8;
      *(undefined4 *)(lVar6 + -0x18 + lVar4) = *(undefined4 *)(*param_6 + 4 + uVar12 * 4);
      *(float *)(lVar6 + -0x14 + lVar4) = (float)(iVar1 + -1);
      lVar4 = *param_8;
      *(undefined4 *)(lVar4 + -0x20 + lVar9) = *(undefined4 *)(*param_6 + 8 + uVar12 * 4);
      *(undefined4 *)(lVar4 + -0x1c + lVar9) = 0;
      iVar1 = *(int *)(param_1 + 0x10);
      lVar4 = *param_8;
      *(undefined4 *)(lVar6 + -0x20 + lVar4) = *(undefined4 *)(*param_6 + 8 + uVar12 * 4);
      *(float *)(lVar6 + -0x1c + lVar4) = (float)(iVar1 + -1);
      lVar4 = *param_8;
      *(undefined4 *)(lVar4 + -0x18 + lVar9) = *(undefined4 *)(*param_6 + 0xc + uVar12 * 4);
      *(undefined4 *)(lVar4 + -0x14 + lVar9) = 0;
      lVar4 = uVar12 * 4;
      uVar12 = uVar12 + 4;
      iVar1 = *(int *)(param_1 + 0x10);
      lVar5 = *param_8;
      *(undefined4 *)(lVar6 + -0x28 + lVar5) = *(undefined4 *)(*param_6 + 0xc + lVar4);
      *(float *)(lVar6 + -0x24 + lVar5) = (float)(iVar1 + -1);
      lVar6 = lVar6 + -0x20;
    } while (uVar12 < param_3 - 4U);
    if (uVar10 <= uVar12) goto LAB_1800d9fe5;
  }
  lVar9 = ((lVar8 + -1) - uVar12) * 8;
  do {
    lVar6 = *param_8;
    *(undefined4 *)(lVar6 + 8 + uVar12 * 8) = *(undefined4 *)(*param_6 + uVar12 * 4);
    *(undefined4 *)(lVar6 + 0xc + uVar12 * 8) = 0;
    lVar6 = uVar12 * 4;
    uVar12 = uVar12 + 1;
    iVar1 = *(int *)(param_1 + 0x10);
    lVar4 = *param_8;
    *(undefined4 *)(lVar9 + -8 + lVar4) = *(undefined4 *)(*param_6 + lVar6);
    *(float *)(lVar9 + -4 + lVar4) = (float)(iVar1 + -1);
    lVar9 = lVar9 + -8;
  } while (uVar12 < uVar10);
LAB_1800d9fe5:
  iVar1 = *(int *)(param_1 + 0x10);
  lVar9 = *param_8;
  *(undefined4 *)(lVar9 + -8 + lVar8 * 8) = 0;
  *(float *)(lVar9 + -4 + lVar8 * 8) = (float)(iVar1 + -1);
  return;
}

