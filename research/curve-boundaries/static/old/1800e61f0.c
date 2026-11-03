// 1800e61f0 FUN_1800e61f0

void FUN_1800e61f0(char param_1,longlong *param_2,undefined8 *param_3,longlong *param_4,
                  longlong *param_5,undefined4 param_6,longlong *param_7)

{
  longlong lVar1;
  longlong lVar2;
  undefined4 *puVar3;
  longlong *plVar4;
  char cVar5;
  char cVar6;
  undefined8 uVar7;
  ulonglong uVar8;
  undefined4 uVar9;
  double dVar10;
  double dVar11;
  float fVar12;
  undefined1 auStack_218 [32];
  undefined4 **local_1f8;
  undefined4 **local_1f0;
  undefined4 **local_1e8;
  undefined4 **local_1e0;
  undefined4 **local_1d8;
  char local_1c8;
  undefined8 local_1c0;
  longlong *local_1b8;
  int iStack_1ac;
  longlong local_1a8;
  undefined1 local_1a0 [136];
  undefined **local_118 [12];
  undefined4 *local_b8;
  undefined8 uStack_b0;
  undefined8 local_a8;
  undefined4 *local_a0;
  undefined8 uStack_98;
  undefined8 local_90;
  undefined4 *local_88;
  undefined8 uStack_80;
  undefined8 local_78;
  undefined4 *local_70;
  undefined8 uStack_68;
  undefined8 local_60;
  ulonglong local_58;
  
  local_58 = DAT_182625200 ^ (ulonglong)auStack_218;
  local_1b8 = param_7;
  local_a0 = (undefined4 *)0x0;
  uStack_98 = 0;
  local_90 = 0;
  local_b8 = (undefined4 *)0x0;
  uStack_b0 = 0;
  local_a8 = 0;
  uVar7 = *param_3;
  lVar1 = *param_2;
  local_1f8 = &local_a0;
  uVar8 = param_2[1] - lVar1 >> 3;
  local_1c8 = param_1;
  if (param_1 == '\0') {
    cVar5 = FUN_1800e4ea0(uVar8 & 0xffffffff,uVar7,lVar1,2);
    if (cVar5 == '\0') {
      local_1f8 = &local_a0;
      cVar5 = FUN_1800e48d0(uVar8 & 0xffffffff,uVar7,lVar1,2);
    }
    else {
      cVar5 = '\x01';
    }
    lVar2 = *param_4;
    local_1c0 = *param_5;
    uVar8 = param_4[1] - lVar2 >> 3;
    local_1f8 = &local_b8;
    cVar6 = FUN_1800e4ea0(uVar8 & 0xffffffff,local_1c0,lVar2,2);
    lVar1 = local_1c0;
  }
  else {
    cVar5 = FUN_1800e4ea0(uVar8 & 0xffffffff,lVar1,uVar7,2);
    if (cVar5 == '\0') {
      local_1f8 = &local_a0;
      cVar5 = FUN_1800e48d0(uVar8 & 0xffffffff,lVar1,uVar7,2);
    }
    else {
      cVar5 = '\x01';
    }
    local_1c0 = *param_5;
    lVar1 = *param_4;
    uVar8 = param_4[1] - lVar1 >> 3;
    local_1f8 = &local_b8;
    cVar6 = FUN_1800e4ea0(uVar8 & 0xffffffff,lVar1,local_1c0,2);
    lVar2 = local_1c0;
  }
  if (cVar6 == '\0') {
    local_1f8 = &local_b8;
    cVar6 = FUN_1800e48d0(uVar8 & 0xffffffff,lVar1,lVar2,2);
  }
  else {
    cVar6 = '\x01';
  }
  if ((cVar5 == '\0') || (cVar6 == '\0')) {
    uVar7 = FUN_1802a58b0(&local_1a8,1);
    FUN_18002c0a0(uVar7);
    FUN_1800679c0(local_1a0);
    *(undefined ***)(local_1a0 + (longlong)*(int *)(local_1a8 + 4) + -8) =
         std::basic_ostream<char,struct_std::char_traits<char>_>::vftable;
    *(int *)((longlong)&iStack_1ac + (longlong)*(int *)(local_1a8 + 4)) =
         *(int *)(local_1a8 + 4) + -0x10;
    local_118[0] = std::ios_base::vftable;
    std::ios_base::_Ios_base_dtor((ios_base *)local_118);
  }
  cVar5 = local_1c8;
  local_70 = (undefined4 *)0x0;
  uStack_68 = 0;
  local_60 = 0;
  local_88 = (undefined4 *)0x0;
  uStack_80 = 0;
  local_78 = 0;
  local_1d8 = &local_88;
  local_1e0 = &local_70;
  local_1e8 = &local_b8;
  local_1f0 = &local_a0;
  local_1f8 = (undefined4 **)param_5;
  FUN_1800e57d0(local_1c8,param_2,param_3,param_4);
  plVar4 = local_1b8;
  FUN_1800c4a50(local_1b8,0x12);
  lVar1 = local_1c0;
  uVar9 = DAT_1825763b0;
  if (cVar5 == '\0') {
    uVar9 = 0;
  }
  puVar3 = (undefined4 *)*plVar4;
  *puVar3 = uVar9;
  puVar3[1] = 0;
  puVar3[2] = local_a0[2];
  puVar3[3] = local_a0[1];
  puVar3[4] = *local_a0;
  puVar3[5] = 0;
  puVar3[6] = local_b8[2];
  puVar3[7] = local_b8[1];
  puVar3[8] = *local_b8;
  puVar3[9] = *local_70;
  puVar3[10] = *local_88;
  puVar3[0xb] = local_70[1];
  puVar3[0xc] = local_88[1];
  puVar3[0xd] = local_70[2];
  puVar3[0xe] = local_88[2];
  puVar3[0xf] = local_70[3];
  puVar3[0x10] = local_88[3];
  puVar3[0x11] = param_6;
  fVar12 = (float)(local_b8[2] & DAT_1825769b0);
  if ((float)(local_b8[2] & DAT_1825769b0) <= (float)(local_a0[2] & DAT_1825769b0)) {
    fVar12 = (float)(local_a0[2] & DAT_1825769b0);
  }
  local_1c0 = CONCAT44(fVar12,(undefined4)local_1c0);
  local_1c0._0_2_ = CONCAT11(DAT_182576360 < fVar12,(char)lVar1);
  dVar10 = (double)FUN_1800e5d10(cVar5,param_4,param_5,&local_b8);
  dVar11 = (double)FUN_1800e5d10(cVar5,param_2,param_3,&local_a0);
  if (dVar10 <= dVar11) {
    dVar10 = dVar11;
  }
  local_1c0 = CONCAT71(local_1c0._1_7_,DAT_182576480 < dVar10);
  FUN_180069f30(&local_88);
  FUN_180069f30(&local_70);
  FUN_180069f30(&local_b8);
  FUN_180069f30(&local_a0);
  FUN_180642b80(local_58 ^ (ulonglong)auStack_218);
  return;
}

