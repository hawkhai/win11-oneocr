// 180524d80 FUN_180524d80

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_180524d80(longlong param_1,float param_2,ulonglong param_3,longlong *param_4,
                  longlong param_5,longlong *param_6,undefined4 *param_7,longlong *param_8)

{
  undefined4 *puVar1;
  longlong lVar2;
  longlong lVar3;
  int iVar4;
  longlong lVar5;
  int iVar6;
  ulonglong uVar7;
  int iVar8;
  ulonglong uVar9;
  longlong lVar10;
  float fVar11;
  double dVar12;
  undefined1 auVar13 [16];
  float fVar14;
  float fVar15;
  float fVar16;
  float afStack_78 [20];
  
  *param_7 = *(undefined4 *)(param_1 + 0x10);
  iVar8 = *(int *)(param_1 + 0x10);
  iVar6 = *(int *)(param_5 + 4);
  if (*(int *)(param_5 + 4) < 1) {
    iVar6 = 0x600;
  }
  iVar4 = *(int *)(param_5 + 8);
  if (*(int *)(param_5 + 8) < 1) {
    iVar4 = 0x300;
  }
  auVar13._0_8_ = (double)(iVar6 * iVar6 + iVar4 * iVar4);
  auVar13._8_8_ = 0;
  if (auVar13._0_8_ < 0.0) {
    dVar12 = (double)FUN_180423490();
  }
  else {
    auVar13 = sqrtpd(ZEXT816(0),auVar13);
    dVar12 = auVar13._0_8_;
  }
  lVar5 = (longlong)(dVar12 * _DAT_18258fbb0 * (double)(iVar8 / 2) + DAT_18258fb60);
  iVar6 = 1000000;
  if (lVar5 < 1000000) {
    iVar6 = (int)lVar5;
  }
  uVar7 = 0;
  if (param_3 != 1) {
    fVar15 = 0.0;
    fVar16 = (float)iVar6 - DAT_18258fae8;
    do {
      lVar5 = *param_4;
      fVar11 = *(float *)(lVar5 + uVar7 * 8) - *(float *)(lVar5 + 8 + uVar7 * 8);
      fVar14 = *(float *)(lVar5 + 4 + uVar7 * 8) - *(float *)(lVar5 + 0xc + uVar7 * 8);
      fVar11 = fVar14 * fVar14 + fVar11 * fVar11;
      if (fVar11 < 0.0) {
        fVar11 = (float)FUN_180425010();
      }
      else {
        fVar11 = SQRT(fVar11);
      }
      uVar7 = uVar7 + 1;
      fVar15 = fVar11 * ((float)iVar8 / param_2) + fVar15;
      if (fVar16 < fVar15) {
        afStack_78[0] = fVar16;
        FUN_180446990(param_6,afStack_78);
        param_3 = uVar7;
        break;
      }
      afStack_78[0] = fVar15;
      FUN_180446990(param_6,afStack_78);
    } while (uVar7 < param_3 - 1);
  }
  uVar7 = param_3 - 1;
  uVar9 = 0;
  puVar1 = (undefined4 *)*param_8;
  iVar8 = (int)(param_4[1] - *param_4 >> 3) + -1;
  *puVar1 = 0;
  puVar1[1] = 0;
  if (uVar7 < 4) {
    if (param_3 == 1) goto LAB_1805250b7;
  }
  else {
    lVar10 = 0x18;
    lVar5 = (longlong)iVar8 * 8 + -0x18;
    do {
      lVar5 = lVar5 + -0x20;
      lVar2 = *param_8;
      lVar10 = lVar10 + 0x20;
      *(undefined4 *)(lVar2 + -0x30 + lVar10) = *(undefined4 *)(*param_6 + uVar9 * 4);
      *(undefined4 *)(lVar2 + -0x2c + lVar10) = 0;
      iVar6 = *(int *)(param_1 + 0x10);
      lVar2 = *param_8;
      *(undefined4 *)(lVar2 + 0x30 + lVar5) = *(undefined4 *)(*param_6 + uVar9 * 4);
      *(float *)(lVar2 + 0x34 + lVar5) = (float)(iVar6 + -1);
      lVar2 = *param_8;
      *(undefined4 *)(lVar2 + -0x28 + lVar10) = *(undefined4 *)(*param_6 + 4 + uVar9 * 4);
      *(undefined4 *)(lVar2 + -0x24 + lVar10) = 0;
      iVar6 = *(int *)(param_1 + 0x10);
      lVar2 = *param_8;
      *(undefined4 *)(lVar2 + 0x28 + lVar5) = *(undefined4 *)(*param_6 + 4 + uVar9 * 4);
      *(float *)(lVar2 + 0x2c + lVar5) = (float)(iVar6 + -1);
      lVar2 = *param_8;
      *(undefined4 *)(lVar2 + -0x20 + lVar10) = *(undefined4 *)(*param_6 + 8 + uVar9 * 4);
      *(undefined4 *)(lVar2 + -0x1c + lVar10) = 0;
      iVar6 = *(int *)(param_1 + 0x10);
      lVar2 = *param_8;
      *(undefined4 *)(lVar2 + 0x20 + lVar5) = *(undefined4 *)(*param_6 + 8 + uVar9 * 4);
      *(float *)(lVar2 + 0x24 + lVar5) = (float)(iVar6 + -1);
      lVar2 = *param_8;
      *(undefined4 *)(lVar2 + -0x18 + lVar10) = *(undefined4 *)(*param_6 + 0xc + uVar9 * 4);
      *(undefined4 *)(lVar2 + -0x14 + lVar10) = 0;
      iVar6 = *(int *)(param_1 + 0x10);
      lVar3 = *param_8;
      lVar2 = uVar9 * 4;
      uVar9 = uVar9 + 4;
      *(undefined4 *)(lVar3 + 0x18 + lVar5) = *(undefined4 *)(*param_6 + 0xc + lVar2);
      *(float *)(lVar3 + 0x1c + lVar5) = (float)(iVar6 + -1);
    } while (uVar9 < param_3 - 4);
    if (uVar7 <= uVar9) goto LAB_1805250b7;
  }
  lVar5 = ((longlong)iVar8 - uVar9) * 8;
  do {
    lVar10 = *param_8;
    *(undefined4 *)(lVar10 + 8 + uVar9 * 8) = *(undefined4 *)(*param_6 + uVar9 * 4);
    *(undefined4 *)(lVar10 + 0xc + uVar9 * 8) = 0;
    iVar6 = *(int *)(param_1 + 0x10);
    lVar2 = *param_8;
    lVar10 = uVar9 * 4;
    uVar9 = uVar9 + 1;
    *(undefined4 *)(lVar5 + -8 + lVar2) = *(undefined4 *)(*param_6 + lVar10);
    *(float *)(lVar5 + -4 + lVar2) = (float)(iVar6 + -1);
    lVar5 = lVar5 + -8;
  } while (uVar9 < uVar7);
LAB_1805250b7:
  iVar6 = *(int *)(param_1 + 0x10);
  lVar5 = *param_8;
  *(undefined4 *)(lVar5 + (longlong)iVar8 * 8) = 0;
  *(float *)(lVar5 + 4 + (longlong)iVar8 * 8) = (float)(iVar6 + -1);
  return;
}

