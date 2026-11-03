// 18052fad0 FUN_18052fad0

void FUN_18052fad0(longlong param_1,longlong *param_2,undefined8 *param_3)

{
  longlong *plVar1;
  undefined8 *puVar2;
  float fVar3;
  double *pdVar4;
  undefined4 *puVar5;
  undefined8 uVar6;
  undefined8 uVar7;
  code *pcVar8;
  undefined4 uVar9;
  undefined4 uVar10;
  undefined4 uVar11;
  bool bVar12;
  longlong lVar13;
  int iVar14;
  undefined8 uVar15;
  longlong lVar16;
  undefined8 *puVar17;
  double *pdVar18;
  ulonglong *puVar19;
  uint uVar20;
  ulonglong uVar21;
  ulonglong *puVar22;
  longlong lVar23;
  longlong lVar24;
  longlong lVar25;
  ulonglong *puVar26;
  double dVar27;
  undefined1 auStack_118 [32];
  ulonglong *puStack_f8;
  longlong lStack_f0;
  undefined8 uStack_e8;
  undefined8 uStack_e0;
  undefined4 uStack_d8;
  undefined4 uStack_d4;
  undefined4 uStack_d0;
  undefined4 uStack_cc;
  undefined4 uStack_c8;
  undefined1 auStack_b8 [80];
  undefined8 uStack_68;
  ulonglong *puStack_60;
  undefined1 uStack_58;
  undefined7 uStack_57;
  ulonglong uStack_50;
  
  uStack_50 = DAT_18264bfc0 ^ (ulonglong)auStack_118;
  puVar22 = (ulonglong *)0x0;
  puStack_60 = (ulonglong *)0x0;
  uStack_58 = 0;
  uStack_57 = 0;
  puVar19 = puVar22;
  lStack_f0 = param_1;
  if (*param_2 == param_2[1]) {
    uStack_68 = 0;
    FUN_1800daec0(&uStack_68,&DAT_182587350);
    FUN_18048c680(uStack_68,0);
    uVar15 = FUN_1800dc480(&uStack_68);
    FUN_18048c680(uVar15,1);
    FUN_1800de7f0(&uStack_68,&puStack_f8);
    FUN_1804f4a50(&uStack_68);
    puVar19 = puStack_f8;
  }
  puStack_f8 = (ulonglong *)0x0;
  uStack_58 = 0;
  bVar12 = false;
  puStack_60 = puVar19;
  while ((puVar19 != (ulonglong *)0x0 && (!bVar12))) {
    uVar15 = FUN_1800daf20(auStack_b8,&DAT_182587280,0x11,2);
    uVar15 = FUN_1800e2f70(uVar15);
    uVar15 = FUN_18047c970(uVar15,&DAT_1825829c0);
    puVar26 = puVar19;
    if (0xf < puVar19[3]) {
      puVar26 = (ulonglong *)*puVar19;
    }
    uVar15 = FUN_180480360(uVar15,puVar26,puVar19[2]);
    uVar15 = FUN_18047c970(uVar15,&DAT_1825829b8);
    FUN_18047c970(uVar15,&DAT_182587268);
    FUN_1800db4f0(auStack_b8);
    uStack_58 = 1;
    bVar12 = true;
  }
  if (puVar19 == (ulonglong *)0x0) {
    fVar3 = *(float *)*param_2;
    *(bool *)lStack_f0 = fVar3 != 0.0;
    uVar15 = param_3[1];
    *(undefined8 *)(lStack_f0 + 4) = *param_3;
    *(undefined8 *)(lStack_f0 + 0xc) = uVar15;
    uVar9 = *(undefined4 *)((longlong)param_3 + 0x14);
    uVar10 = *(undefined4 *)(param_3 + 3);
    uVar11 = *(undefined4 *)((longlong)param_3 + 0x1c);
    *(undefined4 *)(lStack_f0 + 0x14) = *(undefined4 *)(param_3 + 2);
    *(undefined4 *)(lStack_f0 + 0x18) = uVar9;
    *(undefined4 *)(lStack_f0 + 0x1c) = uVar10;
    *(undefined4 *)(lStack_f0 + 0x20) = uVar11;
    uVar21 = param_2[1] - *param_2 >> 2;
    puStack_60 = (ulonglong *)0x0;
    uStack_58 = 0;
    uStack_57 = 0;
    if (fVar3 == 0.0) {
      uVar21 = uVar21 % 10;
      puVar19 = puVar22;
      if (uVar21 != 0) {
        uStack_68 = 0;
        FUN_1800daec0(&uStack_68,&DAT_182587220);
        FUN_18048bfa0(uStack_68,0);
        uVar15 = FUN_1800dc480(&uStack_68);
        FUN_18048c440(uVar15,uVar21);
        FUN_1800de7f0(&uStack_68,&puStack_f8);
        FUN_1804f4a50(&uStack_68);
        puVar19 = puStack_f8;
      }
      puStack_f8 = (ulonglong *)0x0;
      uStack_58 = 0;
      bVar12 = false;
      puStack_60 = puVar19;
      while ((lVar13 = lStack_f0, puVar19 != (ulonglong *)0x0 && (!bVar12))) {
        uVar15 = FUN_1800daf20(auStack_b8,&DAT_182587280,0x18,2);
        uVar15 = FUN_1800e2f70(uVar15);
        uVar15 = FUN_18047c970(uVar15,&DAT_1825829c0);
        puVar26 = puVar19;
        if (0xf < puVar19[3]) {
          puVar26 = (ulonglong *)*puVar19;
        }
        uVar15 = FUN_180480360(uVar15,puVar26,puVar19[2]);
        uVar15 = FUN_18047c970(uVar15,&DAT_1825829b8);
        FUN_18047c970(uVar15,&DAT_182587240);
        FUN_1800db4f0(auStack_b8);
        uStack_58 = 1;
        bVar12 = true;
      }
      if (puVar19 == (ulonglong *)0x0) {
        lVar24 = *param_2;
        if (param_2[1] - lVar24 >> 2 != 0) {
          plVar1 = (longlong *)(lStack_f0 + 0x40);
          lVar16 = lStack_f0;
          do {
            dVar27 = (double)(int)*(float *)(lVar24 + (longlong)puVar22 * 4);
            pdVar18 = *(double **)(lVar13 + 0x48);
            if (pdVar18 == *(double **)(lVar13 + 0x50)) {
              lVar24 = (longlong)pdVar18 - *plVar1 >> 3;
              if (lVar24 == 0x1fffffffffffffff) {
                FUN_180444e60();
                pcVar8 = (code *)swi(3);
                (*pcVar8)();
                return;
              }
              puStack_f8 = (ulonglong *)(lVar24 + 1);
              uVar21 = (longlong)*(double **)(lVar13 + 0x50) - *plVar1 >> 3;
              if (0x1fffffffffffffff - (uVar21 >> 1) < uVar21) {
                puVar26 = (ulonglong *)0x1fffffffffffffff;
              }
              else {
                puVar19 = (ulonglong *)((uVar21 >> 1) + uVar21);
                puVar26 = puStack_f8;
                if (puStack_f8 <= puVar19) {
                  puVar26 = puVar19;
                }
                if ((ulonglong *)0x1fffffffffffffff < puVar26) goto LAB_1805301ae;
              }
              lVar16 = FUN_180440bd0((longlong)puVar26 * 8);
              *(double *)(lVar16 + lVar24 * 8) = dVar27;
              pdVar4 = (double *)*plVar1;
              if (pdVar18 == *(double **)(lVar13 + 0x48)) {
                lVar23 = (longlong)*(double **)(lVar13 + 0x48) - (longlong)pdVar4;
                lVar24 = lVar16;
                pdVar18 = pdVar4;
              }
              else {
                FUN_1806a7f00(lVar16,pdVar4,(longlong)pdVar18 - (longlong)pdVar4);
                lVar23 = *(longlong *)(lVar13 + 0x48) - (longlong)pdVar18;
                lVar24 = lVar16 + (lVar24 + 1) * 8;
              }
              FUN_1806a7f00(lVar24,pdVar18,lVar23);
              FUN_18049a970(plVar1,lVar16,puStack_f8,puVar26);
              lVar16 = lStack_f0;
            }
            else {
              *pdVar18 = dVar27;
              *(double **)(lVar13 + 0x48) = pdVar18 + 1;
            }
            lVar24 = *param_2;
            puVar2 = (undefined8 *)(lVar24 + 4 + (longlong)puVar22 * 4);
            uStack_e8 = *puVar2;
            uStack_e0 = puVar2[1];
            puVar2 = (undefined8 *)(lVar24 + 0x14 + (longlong)puVar22 * 4);
            uStack_d8 = *(undefined4 *)puVar2;
            uStack_d4 = *(undefined4 *)((longlong)puVar2 + 4);
            uVar15 = *puVar2;
            uStack_d0 = *(undefined4 *)(puVar2 + 1);
            uStack_cc = *(undefined4 *)((longlong)puVar2 + 0xc);
            uVar6 = puVar2[1];
            uStack_c8 = *(undefined4 *)(lVar24 + 0x24 + (longlong)puVar22 * 4);
            puVar5 = *(undefined4 **)(lVar16 + 0x30);
            if (puVar5 == *(undefined4 **)(lVar16 + 0x38)) {
              FUN_18052f020(lVar16 + 0x28,puVar5,&uStack_e8);
              lVar16 = lStack_f0;
            }
            else {
              uStack_e8._4_4_ = (undefined4)((ulonglong)uStack_e8 >> 0x20);
              uStack_e0._4_4_ = (undefined4)((ulonglong)uStack_e0 >> 0x20);
              *puVar5 = (undefined4)uStack_e8;
              puVar5[1] = uStack_e8._4_4_;
              puVar5[2] = (undefined4)uStack_e0;
              puVar5[3] = uStack_e0._4_4_;
              *(undefined8 *)(puVar5 + 4) = uVar15;
              *(undefined8 *)(puVar5 + 6) = uVar6;
              puVar5[8] = uStack_c8;
              *(longlong *)(lVar16 + 0x30) = *(longlong *)(lVar16 + 0x30) + 0x24;
            }
            puVar22 = (ulonglong *)((longlong)puVar22 + 10);
            lVar24 = *param_2;
          } while (puVar22 < (ulonglong *)(param_2[1] - lVar24 >> 2));
        }
        goto LAB_180530107;
      }
      goto LAB_180530186;
    }
    uVar20 = (uint)uVar21 & 0xf;
    if (uVar20 != 1) {
      uStack_68 = 0;
      FUN_1800daec0(&uStack_68,&DAT_1825872c8);
      FUN_18048bfa0(uStack_68,1);
      uVar15 = FUN_1800dc480(&uStack_68);
      FUN_18048c440(uVar15,uVar20);
      FUN_1800de7f0(&uStack_68,&puStack_f8);
      FUN_1804f4a50(&uStack_68);
      puVar22 = puStack_f8;
    }
    puStack_f8 = (ulonglong *)0x0;
    uStack_58 = 0;
    bVar12 = false;
    puStack_60 = puVar22;
    while ((lVar13 = lStack_f0, puVar22 != (ulonglong *)0x0 && (!bVar12))) {
      uVar15 = FUN_1800daf20(auStack_b8,&DAT_182587280,0x24,2);
      uVar15 = FUN_1800e2f70(uVar15);
      uVar15 = FUN_18047c970(uVar15,&DAT_1825829c0);
      puVar19 = puVar22;
      if (0xf < puVar22[3]) {
        puVar19 = (ulonglong *)*puVar22;
      }
      uVar15 = FUN_180480360(uVar15,puVar19,puVar22[2]);
      uVar15 = FUN_18047c970(uVar15,&DAT_1825829b8);
      FUN_18047c970(uVar15,&DAT_182587240);
      FUN_1800db4f0(auStack_b8);
      uStack_58 = 1;
      bVar12 = true;
    }
    if (puVar22 == (ulonglong *)0x0) {
      iVar14 = (int)((ulonglong)(param_2[1] - *param_2 >> 2) >> 4);
      lVar24 = (longlong)iVar14;
      if (iVar14 != 0) {
        lVar16 = lStack_f0 + 0x58;
        lVar23 = lStack_f0 + 0x70;
        lVar25 = 7;
        do {
          puVar17 = (undefined8 *)FUN_18052f440(&uStack_e8,param_2,lVar25 + -6);
          puVar2 = *(undefined8 **)(lVar13 + 0x60);
          if (puVar2 == *(undefined8 **)(lVar13 + 0x68)) {
            FUN_18052ee20(lVar16,puVar2,puVar17);
          }
          else {
            uVar15 = puVar17[2];
            puVar17[2] = 0;
            uVar6 = puVar17[1];
            puVar17[1] = 0;
            uVar7 = *puVar17;
            *puVar17 = 0;
            *puVar2 = uVar7;
            puVar2[1] = uVar6;
            puVar2[2] = uVar15;
            *(longlong *)(lVar13 + 0x60) = *(longlong *)(lVar13 + 0x60) + 0x18;
          }
          FUN_180442d40(&uStack_e8);
          puVar17 = (undefined8 *)FUN_18052f220(&uStack_e8,param_2,lVar25);
          puVar2 = *(undefined8 **)(lVar13 + 0x78);
          if (puVar2 == *(undefined8 **)(lVar13 + 0x80)) {
            FUN_18052ec20(lVar23,puVar2,puVar17);
          }
          else {
            uVar15 = puVar17[2];
            puVar17[2] = 0;
            uVar6 = puVar17[1];
            puVar17[1] = 0;
            uVar7 = *puVar17;
            *puVar17 = 0;
            *puVar2 = uVar7;
            puVar2[1] = uVar6;
            puVar2[2] = uVar15;
            *(longlong *)(lVar13 + 0x78) = *(longlong *)(lVar13 + 0x78) + 0x18;
          }
          FUN_1804442f0(&uStack_e8);
          lVar25 = lVar25 + 0x10;
          lVar24 = lVar24 + -1;
        } while (lVar24 != 0);
      }
LAB_180530107:
      FUN_1803eae20(uStack_50 ^ (ulonglong)auStack_118);
      return;
    }
    uVar15 = FUN_18048c900(&puStack_60);
    FUN_18064aef0(&uStack_e8,uVar15);
    FUN_180409bb8(&uStack_e8,&DAT_182640400);
  }
  uVar15 = FUN_18048c900(&puStack_60);
  FUN_18064aef0(&uStack_e8,uVar15);
  FUN_180409bb8(&uStack_e8,&DAT_182640400);
LAB_180530186:
  uVar15 = FUN_18048c900(&puStack_60);
  FUN_18064aef0(&uStack_e8,uVar15);
  FUN_180409bb8(&uStack_e8,&DAT_182640400);
LAB_1805301ae:
  FUN_1804441f0();
  pcVar8 = (code *)swi(3);
  (*pcVar8)();
  return;
}

