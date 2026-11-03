// 1800d9b50 FUN_1800d9b50

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_1800d9b50(longlong param_1,float param_2,float param_3,longlong param_4,int *param_5,
                  int *param_6,double *param_7)

{
  int iVar1;
  longlong lVar2;
  int iVar3;
  int iVar4;
  undefined1 auVar5 [16];
  double dVar6;
  
  iVar4 = *(int *)(param_1 + 0x10);
  *param_6 = iVar4;
  dVar6 = (double)iVar4 / (double)param_3;
  *param_7 = dVar6;
  iVar4 = (int)((double)param_2 * dVar6);
  if (iVar4 < 1) {
    iVar4 = 1;
  }
  iVar3 = 0x600;
  if (0 < *(int *)(param_4 + 4)) {
    iVar3 = *(int *)(param_4 + 4);
  }
  iVar1 = 0x300;
  if (0 < *(int *)(param_4 + 8)) {
    iVar1 = *(int *)(param_4 + 8);
  }
  auVar5._0_8_ = (double)(iVar3 * iVar3 + iVar1 * iVar1);
  auVar5._8_8_ = 0;
  if (auVar5._0_8_ < 0.0) {
    dVar6 = (double)FUN_18067acc0();
  }
  else {
    auVar5 = sqrtpd(ZEXT816(0),auVar5);
    dVar6 = auVar5._0_8_;
  }
  lVar2 = (longlong)
          (dVar6 * _DAT_182576468 * (double)(*(int *)(param_1 + 0x10) / 2) + DAT_182576418);
  iVar3 = 1000000;
  if (lVar2 < 1000000) {
    iVar3 = (int)lVar2;
  }
  if (iVar4 < iVar3) {
    iVar3 = iVar4;
  }
  *param_5 = iVar3;
  return;
}

