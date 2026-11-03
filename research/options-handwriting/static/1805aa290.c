
undefined8 FUN_1805aa290(longlong param_1,undefined8 param_2,longlong *param_3,undefined8 param_4)

{
  longlong lVar1;
  longlong lVar2;
  longlong lVar3;
  undefined4 uVar4;
  undefined4 uVar5;
  undefined1 uVar6;
  undefined4 uVar7;
  undefined4 uVar8;
  undefined1 uStack_38;
  
  lVar1 = param_3[1];
  lVar2 = *param_3;
  lVar3 = lVar1 - lVar2 >> 4;
  if (*(char *)(param_1 + 0x218) == '\0') {
    FUN_180592a70(lVar2,lVar1,lVar3,uStack_38);
    uVar5 = *(undefined4 *)(param_1 + 0x74);
    uVar4 = *(undefined4 *)(param_1 + 0x78);
    uVar6 = 0;
    uVar7 = DAT_18258fac8;
    uVar8 = DAT_18258fab8;
  }
  else {
    FUN_180592a70(lVar2,lVar1,lVar3,uStack_38);
    uVar7 = *(undefined4 *)(param_1 + 0x224);
    uVar6 = *(undefined1 *)(param_1 + 0x218);
    uVar8 = *(undefined4 *)(param_1 + 0x228);
    uVar4 = *(undefined4 *)(param_1 + 0x220);
    uVar5 = *(undefined4 *)(param_1 + 0x21c);
  }
  FUN_180584ff0(param_2,param_4,param_3,param_1 + 0x80,uVar5,uVar4,uVar6,uVar7,uVar8,DAT_18258fae0,4
               );
  return param_2;
}

