// 180445df0 CreateOcrProcessOptions

undefined8 CreateOcrProcessOptions(longlong *param_1)

{
  undefined1 *puVar1;
  undefined8 *puVar2;
  longlong lVar3;
  undefined1 auStack_a8 [4];
  undefined4 uStack_a4;
  undefined4 uStack_a0;
  undefined1 uStack_9c;
  undefined1 auStack_98 [32];
  undefined1 auStack_78 [32];
  undefined4 uStack_58;
  undefined4 uStack_54;
  undefined4 uStack_50;
  undefined4 uStack_4c;
  undefined1 uStack_48;
  undefined1 uStack_47;
  undefined1 uStack_46;
  undefined1 uStack_45;
  undefined1 uStack_44;
  undefined4 uStack_40;
  undefined1 uStack_3c;
  undefined1 uStack_3b;
  undefined4 uStack_38;
  undefined4 uStack_34;
  undefined1 uStack_30;
  undefined1 uStack_2f;
  undefined1 uStack_2e;
  undefined4 uStack_2c;
  undefined4 uStack_28;
  undefined1 uStack_24;
  undefined1 uStack_23;
  undefined8 uStack_20;
  undefined1 uStack_18;
  undefined4 uStack_14;
  
                    /* 0x445df0  4  CreateOcrProcessOptions */
  if (param_1 == (longlong *)0x0) {
    return 4;
  }
  puVar2 = (undefined8 *)FUN_1803eae7c(0x98,&DAT_182558904);
  if (puVar2 == (undefined8 *)0x0) {
    *param_1 = 0;
  }
  else {
    *puVar2 = 0;
    puVar2[1] = 0;
    puVar2[2] = 0;
    puVar2[3] = 0;
    puVar2[4] = 0;
    puVar2[5] = 0;
    puVar2[6] = 0;
    puVar2[7] = 0;
    puVar2[8] = 0;
    puVar2[9] = 0;
    puVar2[10] = 0;
    puVar2[0xb] = 0;
    puVar2[0xc] = 0;
    puVar2[0xd] = 0;
    puVar2[0xe] = 0;
    puVar2[0xf] = 0;
    puVar2[0x10] = 0;
    puVar2[0x11] = 0;
    puVar2[0x12] = 0;
    lVar3 = FUN_180442560(puVar2);
    *param_1 = lVar3;
    if (lVar3 != 0) {
      FUN_180442560(auStack_a8);
      puVar1 = (undefined1 *)*param_1;
      uStack_a4 = 0x480;
      uStack_a0 = 0x300;
      uStack_4c = 1;
      uStack_48 = 0;
      uStack_38 = 100;
      uStack_34 = 3000;
      uStack_3b = 1;
      *puVar1 = auStack_a8[0];
      *(undefined4 *)(puVar1 + 4) = 0x480;
      *(undefined4 *)(puVar1 + 8) = 0x300;
      puVar1[0xc] = uStack_9c;
      FUN_1804433b0(puVar1 + 0x10,auStack_98);
      FUN_1804433b0(puVar1 + 0x30,auStack_78);
      *(undefined4 *)(puVar1 + 0x50) = uStack_58;
      *(undefined4 *)(puVar1 + 0x54) = uStack_54;
      *(undefined4 *)(puVar1 + 0x58) = uStack_50;
      *(undefined4 *)(puVar1 + 0x5c) = uStack_4c;
      puVar1[0x60] = uStack_48;
      puVar1[0x61] = uStack_47;
      puVar1[0x62] = uStack_46;
      puVar1[99] = uStack_45;
      puVar1[100] = uStack_44;
      *(undefined4 *)(puVar1 + 0x68) = uStack_40;
      puVar1[0x6c] = uStack_3c;
      puVar1[0x6d] = uStack_3b;
      *(undefined4 *)(puVar1 + 0x70) = uStack_38;
      *(undefined4 *)(puVar1 + 0x74) = uStack_34;
      puVar1[0x78] = uStack_30;
      puVar1[0x79] = uStack_2f;
      puVar1[0x7a] = uStack_2e;
      *(undefined4 *)(puVar1 + 0x7c) = uStack_2c;
      *(undefined4 *)(puVar1 + 0x80) = uStack_28;
      puVar1[0x84] = uStack_24;
      puVar1[0x85] = uStack_23;
      *(undefined8 *)(puVar1 + 0x88) = uStack_20;
      puVar1[0x90] = uStack_18;
      *(undefined4 *)(puVar1 + 0x94) = uStack_14;
      FUN_1804448d0(auStack_78);
      FUN_1804448d0(auStack_98);
      return 0;
    }
  }
  return 1;
}

