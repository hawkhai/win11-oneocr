// 1800defd0 FUN_1800defd0

void FUN_1800defd0(undefined8 param_1,undefined8 param_2,longlong *param_3,longlong *param_4,
                  longlong param_5,char param_6,longlong param_7)

{
  undefined4 uVar1;
  undefined4 *puVar2;
  undefined4 *puVar3;
  undefined8 *puVar4;
  longlong lVar5;
  ulonglong *puVar6;
  undefined8 *puVar7;
  undefined4 *puVar8;
  undefined1 auStack_c8 [32];
  undefined4 **local_a8;
  undefined4 local_98 [2];
  undefined8 local_90;
  undefined8 uStack_88;
  undefined8 *local_80;
  undefined8 *local_78;
  undefined8 *puStack_70;
  undefined8 *local_68;
  undefined4 *local_60;
  undefined4 *puStack_58;
  undefined8 local_50;
  undefined8 local_48;
  undefined4 uStack_40;
  undefined4 uStack_3c;
  longlong local_38;
  ulonglong local_30;
  
  local_30 = DAT_182625200 ^ (ulonglong)auStack_c8;
  lVar5 = ((param_3[1] - *param_3 >> 3) + -1) - param_5;
  local_80 = (undefined8 *)(param_5 + 1);
  if (param_6 == '\0') {
    uStack_88 = (undefined8 *)(lVar5 + -1);
    local_90 = (undefined8 *)lVar5;
  }
  else {
    local_90 = (undefined8 *)param_5;
    uStack_88 = local_80;
    local_80 = (undefined8 *)lVar5;
  }
  local_48._0_4_ = (undefined4)local_90;
  local_48._4_4_ = local_90._4_4_;
  uStack_40 = (undefined4)uStack_88;
  uStack_3c = uStack_88._4_4_;
  local_38 = (longlong)local_80;
  local_90 = (undefined8 *)0x0;
  uStack_88 = (undefined8 *)0x0;
  local_80 = (undefined8 *)0x0;
  local_90 = (undefined8 *)FUN_18002b510(0x18);
  local_80 = local_90 + 3;
  *local_90 = 0;
  local_90[1] = 0;
  local_90[2] = 0;
  uStack_88 = local_90 + 3;
  puVar6 = &local_48;
  puVar8 = (undefined4 *)((longlong)local_90 + 4);
  do {
    uVar1 = *(undefined4 *)(*param_3 + 4 + *puVar6 * 8);
    puVar8[-1] = *(undefined4 *)(*param_3 + *puVar6 * 8);
    *puVar8 = uVar1;
    puVar8 = puVar8 + 2;
    puVar6 = puVar6 + 1;
  } while (puVar6 != &local_30);
  local_78 = (undefined8 *)0x0;
  puStack_70 = (undefined8 *)0x0;
  local_68 = (undefined8 *)0x0;
  local_78 = (undefined8 *)FUN_18002b510(0x18);
  local_68 = local_78 + 3;
  *local_78 = 0;
  local_78[1] = 0;
  local_78[2] = 0;
  puStack_70 = local_78 + 3;
  puVar6 = &local_48;
  puVar8 = (undefined4 *)((longlong)local_78 + 4);
  do {
    uVar1 = *(undefined4 *)(*param_4 + 4 + *puVar6 * 8);
    puVar8[-1] = *(undefined4 *)(*param_4 + *puVar6 * 8);
    *puVar8 = uVar1;
    puVar8 = puVar8 + 2;
    puVar6 = puVar6 + 1;
  } while (puVar6 != &local_30);
  local_60 = (undefined4 *)0x0;
  puStack_58 = (undefined4 *)0x0;
  local_50 = 0;
  local_a8 = &local_60;
  FUN_1801b1920(param_1,param_2,&local_90,&local_78);
  puVar4 = puStack_70;
  puVar8 = puStack_58;
  for (puVar7 = local_78; puStack_58 = puVar8, puVar3 = local_60, puVar7 != puVar4;
      puVar7 = puVar7 + 1) {
    puVar8 = *(undefined4 **)(param_7 + 8);
    if (puVar8 == *(undefined4 **)(param_7 + 0x10)) {
      FUN_180072fe0(param_7,puVar8,puVar7);
    }
    else {
      *puVar8 = *(undefined4 *)puVar7;
      *(undefined4 **)(param_7 + 8) = puVar8 + 1;
    }
    puVar8 = *(undefined4 **)(param_7 + 8);
    if (puVar8 == *(undefined4 **)(param_7 + 0x10)) {
      FUN_180072fe0(param_7);
    }
    else {
      *puVar8 = *(undefined4 *)((longlong)puVar7 + 4);
      *(undefined4 **)(param_7 + 8) = puVar8 + 1;
    }
    puVar8 = puStack_58;
  }
  for (; puVar3 != puVar8; puVar3 = puVar3 + 1) {
    local_98[0] = *puVar3;
    puVar2 = *(undefined4 **)(param_7 + 8);
    if (puVar2 == *(undefined4 **)(param_7 + 0x10)) {
      FUN_180072fe0(param_7,puVar2,local_98);
    }
    else {
      *puVar2 = local_98[0];
      *(undefined4 **)(param_7 + 8) = puVar2 + 1;
    }
  }
  FUN_180069f30(&local_60);
  FUN_18002ab40(&local_78);
  FUN_18002ab40(&local_90);
  FUN_180642b80(local_30 ^ (ulonglong)auStack_c8);
  return;
}

