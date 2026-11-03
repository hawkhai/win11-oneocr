// 1800ddd60 FUN_1800ddd60

void FUN_1800ddd60(longlong param_1,longlong *param_2,undefined8 *param_3)

{
  float fVar1;
  undefined8 uVar2;
  undefined8 uVar3;
  code *pcVar4;
  undefined4 uVar5;
  undefined4 uVar6;
  undefined4 uVar7;
  bool bVar8;
  int iVar9;
  undefined8 uVar10;
  undefined8 *puVar11;
  undefined8 *puVar12;
  uint uVar13;
  ulonglong uVar14;
  longlong lVar15;
  undefined8 *puVar16;
  longlong lVar17;
  undefined1 auStack_f8 [32];
  undefined8 *local_d8;
  undefined8 local_d0;
  undefined8 uStack_c8;
  undefined8 local_c0;
  undefined8 uStack_b8;
  undefined4 local_b0;
  undefined1 local_a8 [80];
  undefined8 local_58;
  undefined8 *local_50;
  undefined1 uStack_48;
  undefined7 uStack_47;
  ulonglong local_40;
  
  local_40 = DAT_182625200 ^ (ulonglong)auStack_f8;
  puVar16 = (undefined8 *)0x0;
  local_50 = (undefined8 *)0x0;
  uStack_48 = 0;
  uStack_47 = 0;
  lVar17 = *param_2;
  lVar15 = param_2[1];
  puVar11 = puVar16;
  if (lVar17 == lVar15) {
    local_58 = 0;
    FUN_1802a5570(&local_58,"!vTransformData.empty()");
    FUN_1800765c0(local_58,lVar17 != lVar15);
    uVar10 = FUN_1802a6c20(&local_58);
    FUN_1800765c0(uVar10,1);
    FUN_1802a9110(&local_58,&local_d8);
    FUN_1800b3690(&local_58);
    puVar11 = local_d8;
  }
  local_d8 = (undefined8 *)0x0;
  bVar8 = false;
  uStack_48 = 0;
  local_50 = puVar11;
  if (puVar11 != (undefined8 *)0x0) {
    while (!bVar8) {
      uVar10 = FUN_1802a55d0(local_a8,
                             "C:\\__w\\1\\s\\CoreEngine\\Native\\TextNormalizer\\TextLocationMapper.cpp"
                             ,0x11,2);
      uVar10 = FUN_1802ad3d0(uVar10);
      uVar10 = FUN_18006bfb0(uVar10,"Check failed: ");
      puVar12 = puVar11;
      if (0xf < (ulonglong)puVar11[3]) {
        puVar12 = (undefined8 *)*puVar11;
      }
      uVar10 = FUN_180070120(uVar10,puVar12,puVar11[2]);
      uVar10 = FUN_18006bfb0(uVar10,&DAT_1825674fc);
      FUN_18006bfb0(uVar10,"TransformData is empty!");
      FUN_1802a5ca0(local_a8);
      uStack_48 = 1;
      bVar8 = true;
    }
  }
  if (puVar11 == (undefined8 *)0x0) {
    fVar1 = *(float *)*param_2;
    *(bool *)param_1 = fVar1 != 0.0;
    uVar10 = param_3[1];
    *(undefined8 *)(param_1 + 4) = *param_3;
    *(undefined8 *)(param_1 + 0xc) = uVar10;
    uVar5 = *(undefined4 *)((longlong)param_3 + 0x14);
    uVar6 = *(undefined4 *)(param_3 + 3);
    uVar7 = *(undefined4 *)((longlong)param_3 + 0x1c);
    *(undefined4 *)(param_1 + 0x14) = *(undefined4 *)(param_3 + 2);
    *(undefined4 *)(param_1 + 0x18) = uVar5;
    *(undefined4 *)(param_1 + 0x1c) = uVar6;
    *(undefined4 *)(param_1 + 0x20) = uVar7;
    local_50 = (undefined8 *)0x0;
    uStack_48 = 0;
    uStack_47 = 0;
    if (fVar1 == 0.0) {
      uVar14 = (ulonglong)(param_2[1] - *param_2 >> 2) % 10;
      puVar11 = puVar16;
      if (uVar14 != 0) {
        local_58 = 0;
        FUN_1802a5570(&local_58,"0 == vTransformData.size() % 10");
        FUN_1800682e0(local_58,0);
        uVar10 = FUN_1802a6c20(&local_58);
        FUN_180067e80(uVar10,uVar14);
        FUN_1802a9110(&local_58,&local_d8);
        FUN_1800b3690(&local_58);
        puVar11 = local_d8;
      }
      local_d8 = (undefined8 *)0x0;
      bVar8 = false;
      uStack_48 = 0;
      local_50 = puVar11;
      if (puVar11 != (undefined8 *)0x0) {
        while (!bVar8) {
          uVar10 = FUN_1802a55d0(local_a8,
                                 "C:\\__w\\1\\s\\CoreEngine\\Native\\TextNormalizer\\TextLocationMapper.cpp"
                                 ,0x18,2);
          uVar10 = FUN_1802ad3d0(uVar10);
          uVar10 = FUN_18006bfb0(uVar10,"Check failed: ");
          puVar12 = puVar11;
          if (0xf < (ulonglong)puVar11[3]) {
            puVar12 = (undefined8 *)*puVar11;
          }
          uVar10 = FUN_180070120(uVar10,puVar12,puVar11[2]);
          uVar10 = FUN_18006bfb0(uVar10,&DAT_1825674fc);
          FUN_18006bfb0(uVar10,"Size of TransformData is not correct!");
          FUN_1802a5ca0(local_a8);
          uStack_48 = 1;
          bVar8 = true;
        }
      }
      if (puVar11 == (undefined8 *)0x0) {
        lVar17 = *param_2;
        if (param_2[1] - lVar17 >> 2 != 0) {
          do {
            local_d8 = (undefined8 *)(double)(int)*(float *)(lVar17 + (longlong)puVar16 * 4);
            puVar11 = *(undefined8 **)(param_1 + 0x48);
            if (puVar11 == *(undefined8 **)(param_1 + 0x50)) {
              FUN_180073100(param_1 + 0x40,puVar11,&local_d8);
            }
            else {
              *puVar11 = local_d8;
              *(undefined8 **)(param_1 + 0x48) = puVar11 + 1;
            }
            lVar17 = *param_2;
            puVar11 = (undefined8 *)(lVar17 + 4 + (longlong)puVar16 * 4);
            local_d0 = *puVar11;
            uStack_c8 = puVar11[1];
            puVar11 = (undefined8 *)(lVar17 + 0x14 + (longlong)puVar16 * 4);
            local_c0 = *puVar11;
            uStack_b8 = puVar11[1];
            local_b0 = *(undefined4 *)(lVar17 + 0x24 + (longlong)puVar16 * 4);
            puVar11 = *(undefined8 **)(param_1 + 0x30);
            if (puVar11 == *(undefined8 **)(param_1 + 0x38)) {
              FUN_1800de700(param_1 + 0x28,puVar11,&local_d0);
            }
            else {
              *puVar11 = local_d0;
              puVar11[1] = uStack_c8;
              puVar11[2] = local_c0;
              puVar11[3] = uStack_b8;
              *(undefined4 *)(puVar11 + 4) = local_b0;
              *(longlong *)(param_1 + 0x30) = *(longlong *)(param_1 + 0x30) + 0x24;
            }
            puVar16 = (undefined8 *)((longlong)puVar16 + 10);
            lVar17 = *param_2;
          } while (puVar16 < (undefined8 *)(param_2[1] - lVar17 >> 2));
        }
        goto LAB_1800de2a6;
      }
      goto LAB_1800de319;
    }
    uVar13 = (uint)((longlong)(ulonglong)(uint)((int)param_2[1] - (int)*param_2) >> 2) & 0xf;
    if (uVar13 != 1) {
      local_58 = 0;
      FUN_1802a5570(&local_58,"1 == vTransformData.size() % step");
      FUN_1800682e0(local_58,1);
      uVar10 = FUN_1802a6c20(&local_58);
      FUN_180067e80(uVar10,uVar13);
      FUN_1802a9110(&local_58,&local_d8);
      FUN_1800b3690(&local_58);
      puVar16 = local_d8;
    }
    local_d8 = (undefined8 *)0x0;
    bVar8 = false;
    uStack_48 = 0;
    local_50 = puVar16;
    if (puVar16 != (undefined8 *)0x0) {
      while (!bVar8) {
        uVar10 = FUN_1802a55d0(local_a8,
                               "C:\\__w\\1\\s\\CoreEngine\\Native\\TextNormalizer\\TextLocationMapper.cpp"
                               ,0x24,2);
        uVar10 = FUN_1802ad3d0(uVar10);
        uVar10 = FUN_18006bfb0(uVar10,"Check failed: ");
        puVar11 = puVar16;
        if (0xf < (ulonglong)puVar16[3]) {
          puVar11 = (undefined8 *)*puVar16;
        }
        uVar10 = FUN_180070120(uVar10,puVar11,puVar16[2]);
        uVar10 = FUN_18006bfb0(uVar10,&DAT_1825674fc);
        FUN_18006bfb0(uVar10,"Size of TransformData is not correct!");
        FUN_1802a5ca0(local_a8);
        uStack_48 = 1;
        bVar8 = true;
      }
    }
    if (puVar16 == (undefined8 *)0x0) {
      iVar9 = (int)((ulonglong)(param_2[1] - *param_2 >> 2) >> 4);
      lVar17 = (longlong)iVar9;
      if (iVar9 != 0) {
        lVar15 = 7;
        do {
          puVar11 = (undefined8 *)FUN_1800dd900(&local_d0,param_2,lVar15 + -6);
          puVar16 = *(undefined8 **)(param_1 + 0x60);
          if (puVar16 == *(undefined8 **)(param_1 + 0x68)) {
            FUN_1800de8e0(param_1 + 0x58,puVar16);
          }
          else {
            uVar10 = puVar11[2];
            puVar11[2] = 0;
            uVar2 = puVar11[1];
            puVar11[1] = 0;
            uVar3 = *puVar11;
            *puVar11 = 0;
            *puVar16 = uVar3;
            puVar16[1] = uVar2;
            puVar16[2] = uVar10;
            *(longlong *)(param_1 + 0x60) = *(longlong *)(param_1 + 0x60) + 0x18;
          }
          FUN_18002ab40(&local_d0);
          puVar11 = (undefined8 *)FUN_1800ddb40(&local_d0,param_2,lVar15);
          puVar16 = *(undefined8 **)(param_1 + 0x78);
          if (puVar16 == *(undefined8 **)(param_1 + 0x80)) {
            FUN_1800deb90(param_1 + 0x70,puVar16);
          }
          else {
            uVar10 = puVar11[2];
            puVar11[2] = 0;
            uVar2 = puVar11[1];
            puVar11[1] = 0;
            uVar3 = *puVar11;
            *puVar11 = 0;
            *puVar16 = uVar3;
            puVar16[1] = uVar2;
            puVar16[2] = uVar10;
            *(longlong *)(param_1 + 0x78) = *(longlong *)(param_1 + 0x78) + 0x18;
          }
          FUN_180069f30(&local_d0);
          lVar15 = lVar15 + 0x10;
          lVar17 = lVar17 + -1;
        } while (lVar17 != 0);
      }
LAB_1800de2a6:
      FUN_180642b80(local_40 ^ (ulonglong)auStack_f8);
      return;
    }
    uVar10 = FUN_180068600(&local_50);
    FUN_1801b7650(&local_d0,uVar10);
    FUN_180661808(&local_d0,&DAT_1826195a8);
  }
  uVar10 = FUN_180068600(&local_50);
  FUN_1801b7650(&local_d0,uVar10);
  FUN_180661808(&local_d0,&DAT_1826195a8);
LAB_1800de319:
  uVar10 = FUN_180068600(&local_50);
  FUN_1801b7650(&local_d0,uVar10);
  FUN_180661808(&local_d0,&DAT_1826195a8);
  pcVar4 = (code *)swi(3);
  (*pcVar4)();
  return;
}

