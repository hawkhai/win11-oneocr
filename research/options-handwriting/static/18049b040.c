// 18049b040 FUN_18049b040

ulonglong FUN_18049b040(longlong param_1,longlong *param_2,longlong *param_3)

{
  int *piVar1;
  longlong lVar2;
  undefined8 uVar3;
  longlong lVar4;
  longlong lVar5;
  ulonglong uVar6;
  float fVar7;
  
  lVar2 = *(longlong *)(**(longlong **)(param_1 + 8) + *param_2 * 8);
  if (*(longlong *)(lVar2 + 0x150) != 0) {
    LOCK();
    piVar1 = (int *)(*(longlong *)(lVar2 + 0x150) + 8);
    *piVar1 = *piVar1 + 1;
    UNLOCK();
  }
  uVar3 = *(undefined8 *)(lVar2 + 0x148);
  lVar2 = *(longlong *)(lVar2 + 0x150);
  lVar4 = *(longlong *)(**(longlong **)(param_1 + 8) + *param_3 * 8);
  if (*(longlong *)(lVar4 + 0x150) != 0) {
    LOCK();
    piVar1 = (int *)(*(longlong *)(lVar4 + 0x150) + 8);
    *piVar1 = *piVar1 + 1;
    UNLOCK();
  }
  lVar5 = *(longlong *)(lVar4 + 0x150);
  uVar6 = FUN_1805841e0(uVar3,*(undefined8 *)(lVar4 + 0x148),1,0,uVar3,lVar2,
                        *(undefined8 *)(lVar4 + 0x148),lVar5);
  if ((char)uVar6 != '\0') {
    fVar7 = (float)FUN_180584560();
    if (lVar5 != 0) {
      FUN_1804474f0(lVar5);
    }
    if (lVar2 != 0) {
      FUN_1804474f0(lVar2);
    }
    return (ulonglong)(0.0 < fVar7);
  }
  if (lVar5 != 0) {
    uVar6 = FUN_1804474f0(lVar5);
  }
  if (lVar2 != 0) {
    uVar6 = FUN_1804474f0(lVar2);
  }
  return uVar6 & 0xffffffffffffff00;
}

