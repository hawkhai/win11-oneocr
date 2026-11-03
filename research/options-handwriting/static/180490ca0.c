// 180490ca0 FUN_180490ca0

undefined8 FUN_180490ca0(longlong *param_1,longlong *param_2)

{
  longlong lVar1;
  float fStackX_8;
  float fStackX_c;
  float fStackX_10;
  float fStackX_14;
  undefined4 uStack_28;
  undefined4 uStack_24;
  undefined4 uStack_20;
  undefined4 uStack_1c;
  undefined4 uStack_18;
  undefined4 uStack_14;
  undefined4 uStack_10;
  undefined4 uStack_c;
  
  lVar1 = *param_1;
  uStack_28 = *(undefined4 *)(lVar1 + 0xf0);
  uStack_24 = *(undefined4 *)(lVar1 + 0xf4);
  uStack_20 = *(undefined4 *)(lVar1 + 0xf8);
  uStack_1c = *(undefined4 *)(lVar1 + 0xfc);
  uStack_18 = *(undefined4 *)(lVar1 + 0x100);
  uStack_14 = *(undefined4 *)(lVar1 + 0x104);
  uStack_10 = *(undefined4 *)(lVar1 + 0x108);
  uStack_c = *(undefined4 *)(lVar1 + 0x10c);
  FUN_18048cc10(uStack_28,&fStackX_10,&uStack_28);
  lVar1 = *param_2;
  uStack_28 = *(undefined4 *)(lVar1 + 0xf0);
  uStack_24 = *(undefined4 *)(lVar1 + 0xf4);
  uStack_20 = *(undefined4 *)(lVar1 + 0xf8);
  uStack_1c = *(undefined4 *)(lVar1 + 0xfc);
  uStack_18 = *(undefined4 *)(lVar1 + 0x100);
  uStack_14 = *(undefined4 *)(lVar1 + 0x104);
  uStack_10 = *(undefined4 *)(lVar1 + 0x108);
  uStack_c = *(undefined4 *)(lVar1 + 0x10c);
  FUN_18048cc10(uStack_28,&fStackX_8,&uStack_28);
  if ((fStackX_c <= fStackX_14) && ((fStackX_14 != fStackX_c || (fStackX_8 <= fStackX_10)))) {
    return 0;
  }
  return 1;
}

