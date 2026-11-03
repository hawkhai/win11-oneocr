// 1805841e0 FUN_1805841e0

bool FUN_1805841e0(longlong param_1,longlong param_2,int param_3,double param_4)

{
  float fVar1;
  float fVar2;
  float fVar3;
  float fVar4;
  float fVar5;
  float fVar6;
  float fVar7;
  
  fVar4 = *(float *)(param_2 + 8);
  fVar6 = *(float *)(param_1 + 4);
  if (fVar6 <= fVar4) {
    fVar5 = *(float *)(param_2 + 0x10);
    fVar1 = *(float *)(param_1 + 0xc);
    if (fVar1 <= fVar5) {
      fVar3 = *(float *)(param_2 + 4);
      fVar2 = *(float *)(param_1 + 8);
      if ((fVar3 <= fVar2) &&
         (fVar7 = *(float *)(param_2 + 0xc),
         fVar7 < *(float *)(param_1 + 0x10) || fVar7 == *(float *)(param_1 + 0x10))) {
        if (fVar2 <= fVar4) {
          fVar4 = fVar2;
        }
        if (fVar3 <= fVar6) {
          fVar3 = fVar6;
        }
        if (fVar7 <= fVar1) {
          fVar7 = fVar1;
        }
        if (*(float *)(param_1 + 0x10) <= fVar5) {
          fVar5 = *(float *)(param_1 + 0x10);
        }
        fVar4 = (fVar4 - fVar3) + DAT_18258fae8;
        fVar6 = (fVar5 - fVar7) + DAT_18258fae8;
        if (fVar4 <= 0.0) {
          fVar4 = 0.0;
        }
        if (fVar6 <= 0.0) {
          fVar6 = 0.0;
        }
        if (param_3 == 0) {
          return ((double)(*(float *)(param_2 + 0x1c) + *(float *)(param_1 + 0x1c)) /
                 (param_4 + DAT_18258fb60)) * param_4 < (double)(fVar4 * fVar6);
        }
        if (param_3 == 1) {
          return 0.0 < fVar4 * fVar6;
        }
      }
    }
  }
  return false;
}

