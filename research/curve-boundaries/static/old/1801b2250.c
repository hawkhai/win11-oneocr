// 1801b2250 FUN_1801b2250

void FUN_1801b2250(int *param_1,int *param_2,int param_3,int param_4,int param_5,char param_6,
                  char param_7)

{
  code *pcVar1;
  int *piVar2;
  bool bVar3;
  int iVar4;
  undefined8 uVar5;
  undefined8 *puVar6;
  undefined8 *puVar7;
  undefined8 *puVar8;
  int *piVar9;
  int iVar10;
  undefined1 auStack_f8 [32];
  undefined8 *local_d8;
  undefined1 local_d0 [24];
  undefined1 local_b8 [80];
  undefined8 local_68;
  undefined8 *local_60;
  undefined1 uStack_58;
  undefined7 uStack_57;
  ulonglong local_50;
  
  local_50 = DAT_182625200 ^ (ulonglong)auStack_f8;
  puVar8 = (undefined8 *)0x0;
  local_60 = (undefined8 *)0x0;
  uStack_58 = 0;
  uStack_57 = 0;
  puVar7 = puVar8;
  if (param_3 < 1) {
    local_68 = 0;
    FUN_1802a5570(&local_68,"nMinImageLength > 0");
    FUN_1800682e0(local_68,param_3);
    uVar5 = FUN_1802a6c20(&local_68);
    FUN_1800682e0(uVar5,0);
    FUN_1802a9110(&local_68,&local_d8);
    FUN_1800b3690(&local_68);
    puVar7 = local_d8;
  }
  local_d8 = (undefined8 *)0x0;
  bVar3 = false;
  uStack_58 = 0;
  local_60 = puVar7;
  if (puVar7 != (undefined8 *)0x0) {
    while (!bVar3) {
      uVar5 = FUN_1802a55d0(local_b8,"C:\\__w\\1\\s\\CoreEngine\\Native\\Common\\ImageUtility.cpp",
                            0xed,2);
      uVar5 = FUN_1802ad3d0(uVar5);
      uVar5 = FUN_18006bfb0(uVar5,"Check failed: ");
      puVar6 = puVar7;
      if (0xf < (ulonglong)puVar7[3]) {
        puVar6 = (undefined8 *)*puVar7;
      }
      uVar5 = FUN_180070120(uVar5,puVar6,puVar7[2]);
      FUN_18006bfb0(uVar5,&DAT_1825674fc);
      FUN_1802a5ca0(local_b8);
      uStack_58 = 1;
      bVar3 = true;
    }
  }
  if (puVar7 == (undefined8 *)0x0) {
    local_60 = (undefined8 *)0x0;
    uStack_58 = 0;
    uStack_57 = 0;
    puVar7 = puVar8;
    if (param_4 < param_3) {
      local_68 = 0;
      FUN_1802a5570(&local_68,"nMinImageLength <= nMaxImageLength");
      FUN_1800682e0(local_68,param_3);
      uVar5 = FUN_1802a6c20(&local_68);
      FUN_1800682e0(uVar5,param_4);
      FUN_1802a9110(&local_68,&local_d8);
      FUN_1800b3690(&local_68);
      puVar7 = local_d8;
    }
    local_d8 = (undefined8 *)0x0;
    bVar3 = false;
    uStack_58 = 0;
    local_60 = puVar7;
    if (puVar7 != (undefined8 *)0x0) {
      while (!bVar3) {
        uVar5 = FUN_1802a55d0(local_b8,"C:\\__w\\1\\s\\CoreEngine\\Native\\Common\\ImageUtility.cpp"
                              ,0xee,2);
        uVar5 = FUN_1802ad3d0(uVar5);
        uVar5 = FUN_18006bfb0(uVar5,"Check failed: ");
        puVar6 = puVar7;
        if (0xf < (ulonglong)puVar7[3]) {
          puVar6 = (undefined8 *)*puVar7;
        }
        uVar5 = FUN_180070120(uVar5,puVar6,puVar7[2]);
        FUN_18006bfb0(uVar5,&DAT_1825674fc);
        FUN_1802a5ca0(local_b8);
        uStack_58 = 1;
        bVar3 = true;
      }
    }
    if (puVar7 == (undefined8 *)0x0) {
      local_60 = (undefined8 *)0x0;
      uStack_58 = 0;
      uStack_57 = 0;
      iVar10 = *param_1;
      puVar7 = puVar8;
      if (iVar10 < 1) {
        local_68 = 0;
        FUN_1802a5570(&local_68,"nImageWidth > 0");
        FUN_1800682e0(local_68,iVar10);
        uVar5 = FUN_1802a6c20(&local_68);
        FUN_1800682e0(uVar5,0);
        FUN_1802a9110(&local_68,&local_d8);
        FUN_1800b3690(&local_68);
        puVar7 = local_d8;
      }
      local_d8 = (undefined8 *)0x0;
      bVar3 = false;
      uStack_58 = 0;
      local_60 = puVar7;
      if (puVar7 != (undefined8 *)0x0) {
        while (!bVar3) {
          uVar5 = FUN_1802a55d0(local_b8,
                                "C:\\__w\\1\\s\\CoreEngine\\Native\\Common\\ImageUtility.cpp",0xef,2
                               );
          uVar5 = FUN_1802ad3d0(uVar5);
          uVar5 = FUN_18006bfb0(uVar5,"Check failed: ");
          puVar6 = puVar7;
          if (0xf < (ulonglong)puVar7[3]) {
            puVar6 = (undefined8 *)*puVar7;
          }
          uVar5 = FUN_180070120(uVar5,puVar6,puVar7[2]);
          FUN_18006bfb0(uVar5,&DAT_1825674fc);
          FUN_1802a5ca0(local_b8);
          uStack_58 = 1;
          bVar3 = true;
        }
      }
      if (puVar7 != (undefined8 *)0x0) goto LAB_1801b27e8;
      local_60 = (undefined8 *)0x0;
      uStack_58 = 0;
      uStack_57 = 0;
      iVar10 = *param_2;
      if (iVar10 < 1) {
        local_68 = 0;
        FUN_1802a5570(&local_68,"nImageHeight > 0");
        FUN_1800682e0(local_68,iVar10);
        uVar5 = FUN_1802a6c20(&local_68);
        FUN_1800682e0(uVar5,0);
        FUN_1802a9110(&local_68,&local_d8);
        FUN_1800b3690(&local_68);
        puVar8 = local_d8;
      }
      local_d8 = (undefined8 *)0x0;
      bVar3 = false;
      uStack_58 = 0;
      local_60 = puVar8;
      if (puVar8 != (undefined8 *)0x0) {
        while (!bVar3) {
          uVar5 = FUN_1802a55d0(local_b8,
                                "C:\\__w\\1\\s\\CoreEngine\\Native\\Common\\ImageUtility.cpp",0xf0,2
                               );
          uVar5 = FUN_1802ad3d0(uVar5);
          uVar5 = FUN_18006bfb0(uVar5,"Check failed: ");
          puVar7 = puVar8;
          if (0xf < (ulonglong)puVar8[3]) {
            puVar7 = (undefined8 *)*puVar8;
          }
          uVar5 = FUN_180070120(uVar5,puVar7,puVar8[2]);
          FUN_18006bfb0(uVar5,&DAT_1825674fc);
          FUN_1802a5ca0(local_b8);
          uStack_58 = 1;
          bVar3 = true;
        }
      }
      if (puVar8 == (undefined8 *)0x0) {
        if (param_6 != '\0') {
          piVar9 = param_2;
          piVar2 = param_1;
          if (*param_2 < *param_1) {
            piVar9 = param_1;
            piVar2 = param_2;
          }
          iVar10 = *piVar2;
          if (iVar10 < param_3) {
            *piVar9 = (param_3 * *piVar9) / iVar10;
            *piVar2 = param_3;
            iVar10 = param_3;
          }
          if ((0 < param_4) && (param_4 < *piVar9)) {
            *piVar2 = (param_4 * iVar10) / *piVar9;
            *piVar9 = param_4;
          }
          if ((0 < param_3) && (param_3 < *piVar2)) {
            *piVar9 = (param_3 * *piVar9) / *piVar2;
            *piVar2 = param_3;
          }
        }
        iVar10 = *param_2 / param_5;
        if (param_7 == '\0') {
          iVar4 = (param_5 + -1 + *param_2) / param_5;
          iVar10 = 1;
          if (1 < iVar4) {
            iVar10 = iVar4;
          }
          *param_2 = iVar10 * param_5;
          iVar10 = *param_1 + -1 + param_5;
        }
        else {
          iVar4 = 1;
          if (1 < iVar10) {
            iVar4 = iVar10;
          }
          *param_2 = iVar4 * param_5;
          iVar10 = *param_1;
        }
        iVar4 = 1;
        if (1 < iVar10 / param_5) {
          iVar4 = iVar10 / param_5;
        }
        *param_1 = iVar4 * param_5;
        FUN_180642b80(local_50 ^ (ulonglong)auStack_f8,iVar10 % param_5);
        return;
      }
      uVar5 = FUN_180068600(&local_60);
      FUN_1801b7650(local_d0,uVar5);
      FUN_180661808(local_d0,&DAT_1826195a8);
      goto LAB_1801b2798;
    }
  }
  else {
LAB_1801b2798:
    uVar5 = FUN_180068600(&local_60);
    FUN_1801b7650(local_d0,uVar5);
    FUN_180661808(local_d0,&DAT_1826195a8);
  }
  uVar5 = FUN_180068600(&local_60);
  FUN_1801b7650(local_d0,uVar5);
  FUN_180661808(local_d0,&DAT_1826195a8);
LAB_1801b27e8:
  uVar5 = FUN_180068600(&local_60);
  FUN_1801b7650(local_d0,uVar5);
  FUN_180661808(local_d0,&DAT_1826195a8);
  pcVar1 = (code *)swi(3);
  (*pcVar1)();
  return;
}

