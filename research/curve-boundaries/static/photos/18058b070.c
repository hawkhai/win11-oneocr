// 18058b070 FUN_18058b070

undefined8
FUN_18058b070(char param_1,longlong *param_2,undefined8 *param_3,longlong *param_4,longlong *param_5
             ,undefined4 param_6,longlong *param_7)

{
  code *pcVar1;
  bool bVar2;
  undefined4 *puVar3;
  longlong *plVar4;
  char cVar5;
  char cVar6;
  undefined8 uVar7;
  longlong lVar8;
  ulonglong uVar9;
  undefined4 *puVar10;
  undefined4 *puVar11;
  longlong lVar12;
  undefined4 uVar13;
  double dVar14;
  double dVar15;
  float fVar16;
  undefined8 uStack_1b0;
  undefined4 *puStack_1a8;
  undefined8 uStack_1a0;
  longlong lStack_198;
  undefined4 *puStack_190;
  undefined8 uStack_188;
  longlong lStack_180;
  undefined4 *puStack_178;
  undefined8 uStack_170;
  longlong lStack_168;
  undefined4 *puStack_160;
  undefined8 uStack_158;
  longlong lStack_150;
  longlong *plStack_148;
  int iStack_13c;
  longlong lStack_138;
  undefined1 auStack_130 [136];
  undefined *apuStack_a8 [14];
  
  plStack_148 = param_7;
  puStack_190 = (undefined4 *)0x0;
  uStack_188 = 0;
  lStack_180 = 0;
  puStack_1a8 = (undefined4 *)0x0;
  uStack_1a0 = 0;
  lStack_198 = 0;
  uVar7 = *param_3;
  lVar12 = *param_2;
  uVar9 = param_2[1] - lVar12 >> 3;
  if (param_1 == '\0') {
    cVar5 = FUN_18058a380(uVar9 & 0xffffffff,uVar7,lVar12,2,&puStack_190);
    if (cVar5 == '\0') {
      cVar5 = FUN_18058b580(uVar9 & 0xffffffff,uVar7,lVar12,2,&puStack_190);
    }
    else {
      cVar5 = '\x01';
    }
    lVar12 = *param_4;
    lVar8 = *param_5;
    uVar9 = param_4[1] - lVar12 >> 3;
    cVar6 = FUN_18058a380(uVar9 & 0xffffffff,lVar8,lVar12,2,&puStack_1a8);
    uStack_1b0 = lVar8;
    if (cVar6 != '\0') goto LAB_18058b1fe;
LAB_18058b1e5:
    cVar6 = FUN_18058b580(uVar9 & 0xffffffff,lVar8,lVar12,2,&puStack_1a8);
  }
  else {
    cVar5 = FUN_18058a380(uVar9 & 0xffffffff,lVar12,uVar7,2,&puStack_190);
    if (cVar5 == '\0') {
      cVar5 = FUN_18058b580(uVar9 & 0xffffffff,lVar12,uVar7,2,&puStack_190);
    }
    else {
      cVar5 = '\x01';
    }
    lVar12 = *param_5;
    lVar8 = *param_4;
    uVar9 = param_4[1] - lVar8 >> 3;
    cVar6 = FUN_18058a380(uVar9 & 0xffffffff,lVar8,lVar12,2,&puStack_1a8);
    uStack_1b0 = lVar12;
    if (cVar6 == '\0') goto LAB_18058b1e5;
LAB_18058b1fe:
    cVar6 = '\x01';
  }
  if ((cVar5 == '\0') || (cVar6 == '\0')) {
    uVar7 = FUN_1800db1f0(&lStack_138,1);
    FUN_18045d4c0(uVar7);
    FUN_18048a6b0(auStack_130);
    *(undefined **)(auStack_130 + (longlong)*(int *)(lStack_138 + 4) + -8) = &DAT_1806e7c60;
    *(int *)((longlong)&iStack_13c + (longlong)*(int *)(lStack_138 + 4)) =
         *(int *)(lStack_138 + 4) + -0x10;
    apuStack_a8[0] = &DAT_1806e62b0;
    FUN_1803e4954(apuStack_a8);
  }
  puStack_160 = (undefined4 *)0x0;
  uStack_158 = 0;
  lStack_150 = 0;
  puStack_178 = (undefined4 *)0x0;
  uStack_170 = 0;
  lStack_168 = 0;
  FUN_180589e30(param_1,param_2,param_3,param_4,param_5,&puStack_190,&puStack_1a8,&puStack_160,
                &puStack_178);
  plVar4 = plStack_148;
  FUN_1804c46c0(plStack_148,0x12);
  puVar3 = puStack_160;
  puVar10 = puStack_178;
  uVar13 = DAT_18258fae8;
  if (param_1 == '\0') {
    uVar13 = 0;
  }
  puVar11 = (undefined4 *)*plVar4;
  *puVar11 = uVar13;
  puVar11[1] = 0;
  puVar11[2] = puStack_190[2];
  puVar11[3] = puStack_190[1];
  puVar11[4] = *puStack_190;
  puVar11[5] = 0;
  puVar11[6] = puStack_1a8[2];
  puVar11[7] = puStack_1a8[1];
  puVar11[8] = *puStack_1a8;
  puVar11[9] = *puStack_160;
  puVar11[10] = *puStack_178;
  puVar11[0xb] = puStack_160[1];
  puVar11[0xc] = puStack_178[1];
  puVar11[0xd] = puStack_160[2];
  puVar11[0xe] = puStack_178[2];
  puVar11[0xf] = puStack_160[3];
  puVar11[0x10] = puStack_178[3];
  puVar11[0x11] = param_6;
  fVar16 = (float)(puStack_1a8[2] & DAT_182590480);
  if ((float)(puStack_1a8[2] & DAT_182590480) <= (float)(puStack_190[2] & DAT_182590480)) {
    fVar16 = (float)(puStack_190[2] & DAT_182590480);
  }
  uStack_1b0 = CONCAT44(fVar16,(undefined4)uStack_1b0);
  bVar2 = DAT_18258fa90 < fVar16;
  dVar14 = (double)FUN_1805898f0(param_1,param_4,param_5,&puStack_1a8);
  dVar15 = (double)FUN_1805898f0(param_1,param_2,param_3,&puStack_190);
  if (dVar14 <= dVar15) {
    dVar14 = dVar15;
  }
  uStack_1b0 = CONCAT71(CONCAT61(uStack_1b0._2_6_,bVar2),DAT_18258fbd0 < dVar14);
  puVar11 = puVar10;
  if (((lStack_168 - (longlong)puVar10 & 0xfffffffffffffffcU) < 0x1000) ||
     (puVar11 = *(undefined4 **)(puVar10 + -2),
     (ulonglong)((longlong)puVar10 + (-8 - (longlong)puVar11)) < 0x20)) {
    FUN_1803eb2c8(puVar11);
    puVar10 = puVar3;
    if (((lStack_150 - (longlong)puVar3 & 0xfffffffffffffffcU) < 0x1000) ||
       (puVar10 = *(undefined4 **)(puVar3 + -2),
       (ulonglong)((longlong)puVar3 + (-8 - (longlong)puVar10)) < 0x20)) {
      FUN_1803eb2c8(puVar10);
      if (puStack_1a8 != (undefined4 *)0x0) {
        if ((0xfff < (lStack_198 - (longlong)puStack_1a8 & 0xfffffffffffffffcU)) &&
           (0x1f < (ulonglong)((longlong)puStack_1a8 + (-8 - *(longlong *)(puStack_1a8 + -2)))))
        goto LAB_18058b565;
        FUN_1803eb2c8();
        puStack_1a8 = (undefined4 *)0x0;
        uStack_1a0 = 0;
        lStack_198 = 0;
      }
      if (puStack_190 == (undefined4 *)0x0) {
        return uStack_1b0;
      }
      if (((lStack_180 - (longlong)puStack_190 & 0xfffffffffffffffcU) < 0x1000) ||
         ((ulonglong)((longlong)puStack_190 + (-8 - *(longlong *)(puStack_190 + -2))) < 0x20)) {
        FUN_1803eb2c8();
        return uStack_1b0;
      }
      FUN_18040e3e4(0,0,0,0,0);
      goto LAB_18058b533;
    }
  }
  else {
LAB_18058b533:
    FUN_18040e3e4(0,0,0,0,0);
  }
  FUN_18040e3e4(0,0,0,0,0);
LAB_18058b565:
  FUN_18040e3e4(0,0,0,0,0);
  pcVar1 = (code *)swi(3);
  uVar7 = (*pcVar1)();
  return uVar7;
}

