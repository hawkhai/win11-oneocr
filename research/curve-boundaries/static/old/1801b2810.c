// 1801b2810 FUN_1801b2810

void FUN_1801b2810(uint *param_1,longlong param_2)

{
  int iVar1;
  int iVar2;
  code *pcVar3;
  bool bVar4;
  int iVar5;
  ulonglong uVar6;
  ulonglong uVar7;
  char cVar8;
  undefined8 uVar9;
  undefined8 *puVar10;
  undefined8 *puVar11;
  undefined8 *puVar12;
  undefined8 *puVar13;
  double dVar14;
  undefined4 in_stack_00000030;
  undefined1 in_stack_00000038;
  undefined1 in_stack_00000040;
  undefined8 in_stack_00000058;
  undefined1 auStack_2d8 [32];
  undefined8 local_2b8;
  undefined4 local_2b0;
  undefined1 local_2a8;
  undefined8 *local_298;
  undefined1 *puStack_290;
  undefined8 local_288;
  undefined4 local_278 [2];
  undefined8 *local_270;
  undefined8 local_268;
  undefined8 local_260;
  undefined4 local_258;
  undefined8 local_250;
  undefined4 local_248;
  undefined1 local_240 [8];
  undefined1 local_238 [92];
  int iStack_1dc;
  longlong local_1d8;
  undefined1 local_1d0 [136];
  undefined **local_148 [12];
  ulonglong local_e8;
  undefined8 *local_e0;
  undefined8 *puStack_d8;
  undefined8 local_d0;
  ulonglong local_c8 [2];
  undefined8 local_b8;
  undefined8 uStack_b0;
  undefined8 local_a8;
  undefined8 uStack_a0;
  undefined8 local_98;
  undefined8 uStack_90;
  undefined8 local_88;
  undefined8 uStack_80;
  undefined8 local_78;
  undefined8 uStack_70;
  undefined8 local_68;
  undefined8 uStack_60;
  ulonglong local_58;
  
  local_58 = DAT_182625200 ^ (ulonglong)auStack_2d8;
  puVar13 = (undefined8 *)0x0;
  local_c8[0] = CONCAT44(local_c8[0]._4_4_,param_1[3]);
  local_e8 = CONCAT44(local_e8._4_4_,param_1[2]);
  local_2a8 = in_stack_00000040;
  local_2b0 = CONCAT31(local_2b0._1_3_,in_stack_00000038);
  local_2b8 = CONCAT44(local_2b8._4_4_,in_stack_00000030);
  cVar8 = FUN_1801b2250(local_c8,&local_e8);
  local_b8 = 0;
  uStack_b0 = 0;
  local_a8 = 0;
  uStack_a0 = 0;
  local_98 = 0;
  uStack_90 = 0;
  local_88 = 0;
  uStack_80 = 0;
  local_78 = 0;
  uStack_70 = 0;
  local_68 = 0;
  uStack_60 = 0;
  FUN_180396ff0(&local_b8);
  if ((*param_1 & 0xfff) == 0x15) {
    FUN_180397240(&local_b8,param_1);
  }
  else {
    local_d0 = 0;
    local_e0 = (undefined8 *)CONCAT44(local_e0._4_4_,0x2010000);
    puStack_d8 = &local_b8;
    local_2b8 = 0;
    FUN_18039cd80(param_1,&local_e0,0x15,DAT_182576418);
  }
  uVar7 = local_c8[0];
  if (cVar8 == '\0') {
    uVar9 = FUN_1803978a0(&local_b8,local_238);
    FUN_1803970d0(param_2,uVar9);
    FUN_180397090(local_238);
LAB_1801b2ed5:
    uVar9 = FUN_1802a58b0(&local_1d8,1);
    FUN_18002c0a0(uVar9);
    FUN_1800679c0(local_1d0);
    *(undefined ***)((longlong)&local_1d8 + (longlong)*(int *)(local_1d8 + 4)) =
         std::basic_ostream<char,struct_std::char_traits<char>_>::vftable;
    *(int *)((longlong)&iStack_1dc + (longlong)*(int *)(local_1d8 + 4)) =
         *(int *)(local_1d8 + 4) + -0x10;
    local_148[0] = std::ios_base::vftable;
    std::ios_base::_Ios_base_dtor((ios_base *)local_148);
    FUN_180397090(&local_b8);
    FUN_180642b80(local_58 ^ (ulonglong)auStack_2d8);
    return;
  }
  local_e0 = (undefined8 *)0x0;
  puStack_d8 = (undefined8 *)0x0;
  iVar2 = (int)local_c8[0];
  puVar10 = puVar13;
  if ((int)local_c8[0] < 1) {
    local_c8[0] = 0;
    FUN_1802a5570(local_c8,"nImageWidth > 0");
    FUN_1800682e0(local_c8[0],uVar7 & 0xffffffff);
    uVar9 = FUN_1802a6c20(local_c8);
    FUN_1800682e0(uVar9,0);
    FUN_1802a9110(local_c8,&local_298);
    FUN_1800b3690(local_c8);
    puVar10 = local_298;
  }
  local_298 = (undefined8 *)0x0;
  bVar4 = false;
  puStack_d8 = (undefined8 *)((ulonglong)puStack_d8 & 0xffffffffffffff00);
  local_e0 = puVar10;
  if (puVar10 != (undefined8 *)0x0) {
    while (!bVar4) {
      uVar9 = FUN_1802a55d0(local_238,"C:\\__w\\1\\s\\CoreEngine\\Native\\Common\\ImageUtility.cpp",
                            0x147,2);
      uVar9 = FUN_1802ad3d0(uVar9);
      uVar9 = FUN_18006bfb0(uVar9,"Check failed: ");
      puVar12 = puVar10;
      if (0xf < (ulonglong)puVar10[3]) {
        puVar12 = (undefined8 *)*puVar10;
      }
      uVar9 = FUN_180070120(uVar9,puVar12,puVar10[2]);
      FUN_18006bfb0(uVar9,&DAT_1825674fc);
      FUN_1802a5ca0(local_238);
      puStack_d8 = (undefined8 *)CONCAT71(puStack_d8._1_7_,1);
      bVar4 = true;
    }
  }
  uVar6 = local_e8;
  if (puVar10 == (undefined8 *)0x0) {
    local_e0 = (undefined8 *)0x0;
    puStack_d8 = (undefined8 *)0x0;
    iVar5 = (int)local_e8;
    puVar10 = puVar13;
    if ((int)local_e8 < 1) {
      local_e8 = 0;
      FUN_1802a5570(&local_e8,"nImageHeight > 0");
      FUN_1800682e0(local_e8,uVar6 & 0xffffffff);
      uVar9 = FUN_1802a6c20(&local_e8);
      FUN_1800682e0(uVar9,0);
      FUN_1802a9110(&local_e8,&local_298);
      FUN_1800b3690(&local_e8);
      puVar10 = local_298;
    }
    local_298 = (undefined8 *)0x0;
    bVar4 = false;
    puStack_d8 = (undefined8 *)((ulonglong)puStack_d8 & 0xffffffffffffff00);
    local_e0 = puVar10;
    if (puVar10 != (undefined8 *)0x0) {
      while (!bVar4) {
        uVar9 = FUN_1802a55d0(local_238,
                              "C:\\__w\\1\\s\\CoreEngine\\Native\\Common\\ImageUtility.cpp",0x148,2)
        ;
        uVar9 = FUN_1802ad3d0(uVar9);
        uVar9 = FUN_18006bfb0(uVar9,"Check failed: ");
        puVar12 = puVar10;
        if (0xf < (ulonglong)puVar10[3]) {
          puVar12 = (undefined8 *)*puVar10;
        }
        uVar9 = FUN_180070120(uVar9,puVar12,puVar10[2]);
        FUN_18006bfb0(uVar9,&DAT_1825674fc);
        FUN_1802a5ca0(local_238);
        puStack_d8 = (undefined8 *)CONCAT71(puStack_d8._1_7_,1);
        bVar4 = true;
      }
    }
    if (puVar10 == (undefined8 *)0x0) {
      local_e0 = (undefined8 *)0x0;
      puStack_d8 = (undefined8 *)0x0;
      local_260 = in_stack_00000058;
      local_258 = 3;
      local_250 = DAT_182576550;
      local_248 = 1;
      local_298 = &local_260;
      puStack_290 = local_240;
      FUN_1801b4580(&local_e0,&local_298);
      dVar14 = ((double)iVar5 * (double)iVar2) / (double)(uStack_b0._4_4_ * (int)uStack_b0);
      cVar8 = *(char *)((longlong)local_e0[1] + 0x19);
      puVar10 = (undefined8 *)local_e0[1];
      puVar12 = local_e0;
      while (puVar11 = puVar10, cVar8 == '\0') {
        if (dVar14 < (double)puVar11[4] || dVar14 == (double)puVar11[4]) {
          puVar10 = (undefined8 *)*puVar11;
        }
        else {
          puVar10 = (undefined8 *)puVar11[2];
          puVar11 = puVar12;
        }
        cVar8 = *(char *)((longlong)puVar10 + 0x19);
        puVar12 = puVar11;
      }
      if (puVar12 == local_e0) {
        local_2b0 = 1;
      }
      else {
        local_2b0 = *(undefined4 *)(puVar12 + 5);
      }
      local_288 = 0;
      local_298 = (undefined8 *)CONCAT44(local_298._4_4_,0x2010000);
      local_268 = 0;
      local_278[0] = 0x1010000;
      local_270 = &local_b8;
      local_2b8 = 0;
      puStack_290 = (undefined1 *)param_2;
      FUN_180300300(local_278,&local_298,CONCAT44(iVar5,iVar2),0);
      FUN_1800822d0(&local_e0);
      local_e0 = (undefined8 *)0x0;
      puStack_d8 = (undefined8 *)0x0;
      iVar1 = *(int *)(param_2 + 0xc);
      puVar10 = puVar13;
      if (iVar1 != iVar2) {
        local_e8 = 0;
        FUN_1802a5570(&local_e8,"cvImgResized.cols == nImageWidth");
        FUN_1800682e0(local_e8,iVar1);
        uVar9 = FUN_1802a6c20(&local_e8);
        FUN_1800682e0(uVar9,uVar7 & 0xffffffff);
        FUN_1802a9110(&local_e8,&local_298);
        FUN_1800b3690(&local_e8);
        puVar10 = local_298;
      }
      local_298 = (undefined8 *)0x0;
      bVar4 = false;
      puStack_d8 = (undefined8 *)((ulonglong)puStack_d8 & 0xffffffffffffff00);
      local_e0 = puVar10;
      if (puVar10 != (undefined8 *)0x0) {
        while (!bVar4) {
          uVar9 = FUN_1802a55d0(local_238,
                                "C:\\__w\\1\\s\\CoreEngine\\Native\\Common\\ImageUtility.cpp",0x170,
                                2);
          uVar9 = FUN_1802ad3d0(uVar9);
          uVar9 = FUN_18006bfb0(uVar9,"Check failed: ");
          puVar12 = puVar10;
          if (0xf < (ulonglong)puVar10[3]) {
            puVar12 = (undefined8 *)*puVar10;
          }
          uVar9 = FUN_180070120(uVar9,puVar12,puVar10[2]);
          FUN_18006bfb0(uVar9,&DAT_1825674fc);
          FUN_1802a5ca0(local_238);
          puStack_d8 = (undefined8 *)CONCAT71(puStack_d8._1_7_,1);
          bVar4 = true;
        }
      }
      if (puVar10 != (undefined8 *)0x0) goto LAB_1801b2fe8;
      local_e0 = (undefined8 *)0x0;
      puStack_d8 = (undefined8 *)0x0;
      iVar2 = *(int *)(param_2 + 8);
      if (iVar2 != iVar5) {
        local_e8 = 0;
        FUN_1802a5570(&local_e8,"cvImgResized.rows == nImageHeight");
        FUN_1800682e0(local_e8,iVar2);
        uVar9 = FUN_1802a6c20(&local_e8);
        FUN_1800682e0(uVar9,uVar6 & 0xffffffff);
        FUN_1802a9110(&local_e8,&local_298);
        FUN_1800b3690(&local_e8);
        puVar13 = local_298;
      }
      local_298 = (undefined8 *)0x0;
      bVar4 = false;
      puStack_d8 = (undefined8 *)((ulonglong)puStack_d8 & 0xffffffffffffff00);
      local_e0 = puVar13;
      if (puVar13 != (undefined8 *)0x0) {
        while (!bVar4) {
          uVar9 = FUN_1802a55d0(local_238,
                                "C:\\__w\\1\\s\\CoreEngine\\Native\\Common\\ImageUtility.cpp",0x171,
                                2);
          uVar9 = FUN_1802ad3d0(uVar9);
          uVar9 = FUN_18006bfb0(uVar9,"Check failed: ");
          puVar10 = puVar13;
          if (0xf < (ulonglong)puVar13[3]) {
            puVar10 = (undefined8 *)*puVar13;
          }
          uVar9 = FUN_180070120(uVar9,puVar10,puVar13[2]);
          FUN_18006bfb0(uVar9,&DAT_1825674fc);
          FUN_1802a5ca0(local_238);
          puStack_d8 = (undefined8 *)CONCAT71(puStack_d8._1_7_,1);
          bVar4 = true;
        }
      }
      if (puVar13 == (undefined8 *)0x0) goto LAB_1801b2ed5;
      uVar9 = FUN_180068600(&local_e0);
      FUN_1801b7650(local_278,uVar9);
      FUN_180661808(local_278,&DAT_1826195a8);
      goto LAB_1801b2f92;
    }
  }
  else {
LAB_1801b2f92:
    uVar9 = FUN_180068600(&local_e0);
    FUN_1801b7650(local_278,uVar9);
    FUN_180661808(local_278,&DAT_1826195a8);
  }
  uVar9 = FUN_180068600(&local_e0);
  FUN_1801b7650(local_278,uVar9);
  FUN_180661808(local_278,&DAT_1826195a8);
LAB_1801b2fe8:
  uVar9 = FUN_180068600(&local_e0);
  FUN_1801b7650(local_278,uVar9);
  FUN_180661808(local_278,&DAT_1826195a8);
  pcVar3 = (code *)swi(3);
  (*pcVar3)();
  return;
}

