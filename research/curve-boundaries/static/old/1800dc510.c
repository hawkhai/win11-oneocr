// 1800dc510 FUN_1800dc510

/* WARNING: Removing unreachable block (ram,0x0001800dca0c) */

void FUN_1800dc510(undefined8 param_1,ulonglong param_2,float *param_3,undefined8 param_4,
                  float *param_5,longlong *param_6,undefined8 param_7)

{
  float fVar1;
  float fVar2;
  float fVar3;
  float fVar4;
  float fVar5;
  float fVar6;
  float fVar7;
  float fVar8;
  float fVar9;
  undefined4 *puVar10;
  code *pcVar11;
  bool bVar12;
  longlong *plVar13;
  int iVar14;
  undefined8 uVar15;
  longlong lVar16;
  float *pfVar17;
  float *pfVar18;
  float *pfVar19;
  float *pfVar20;
  ulonglong uVar21;
  longlong lVar22;
  longlong lVar23;
  float fVar24;
  float fVar25;
  float fVar26;
  float fVar27;
  undefined4 uVar28;
  float fVar29;
  undefined1 auStack_458 [32];
  longlong local_438;
  longlong *local_430;
  longlong *local_428;
  undefined8 **local_420;
  undefined8 local_408;
  float *local_400;
  undefined1 local_3f8 [24];
  undefined8 local_3e0;
  undefined8 local_3d8;
  longlong *local_3d0;
  undefined8 local_3c8;
  undefined1 local_3b8 [80];
  undefined1 local_368 [16];
  undefined1 local_358 [96];
  undefined1 local_2f8 [96];
  undefined1 local_298 [144];
  undefined8 local_208;
  float *local_200;
  undefined8 *puStack_1f8;
  float *local_1f0;
  int local_1e8 [2];
  float *local_1e0;
  ulonglong local_1d8;
  longlong local_1d0;
  longlong lStack_1c8;
  undefined8 local_1c0;
  undefined8 *local_1b8;
  undefined8 *puStack_1b0;
  undefined8 *local_1a8;
  undefined8 local_198;
  undefined8 uStack_190;
  undefined8 local_188;
  undefined8 uStack_180;
  undefined8 local_178;
  undefined8 uStack_170;
  undefined8 local_168;
  undefined8 uStack_160;
  undefined8 local_158;
  undefined8 uStack_150;
  undefined8 local_148;
  undefined8 uStack_140;
  undefined8 local_138;
  undefined8 uStack_130;
  undefined8 local_128;
  undefined8 uStack_120;
  undefined8 local_118;
  undefined8 uStack_110;
  undefined8 local_108;
  undefined8 uStack_100;
  undefined8 local_f8;
  undefined8 uStack_f0;
  undefined8 local_e8;
  undefined8 uStack_e0;
  ulonglong local_d8;
  
  local_d8 = DAT_182625200 ^ (ulonglong)auStack_458;
  local_1e0 = param_5;
  local_3d0 = param_6;
  local_3d8 = param_7;
  pfVar18 = (float *)0x0;
  pfVar19 = (float *)((ulonglong)(*(longlong *)(param_3 + 2) - *(longlong *)param_3 >> 3) >> 1);
  local_200 = (float *)0x0;
  puStack_1f8._0_1_ = 0;
  puStack_1f8._1_7_ = 0;
  pfVar20 = pfVar18;
  local_408 = param_3;
  local_3e0 = param_1;
  local_3c8 = param_4;
  local_1d8 = param_2;
  if (pfVar19 == (float *)0x0) {
    local_208 = 0;
    FUN_1802a5570(&local_208,"nHalfPointsNumber > 0");
    FUN_180067e80(local_208,0);
    uVar15 = FUN_1802a6c20(&local_208);
    FUN_1800682e0(uVar15,0);
    FUN_1802a9110(&local_208,&local_400);
    FUN_1800b3690(&local_208);
    pfVar20 = local_400;
  }
  local_400 = (float *)0x0;
  bVar12 = false;
  puStack_1f8._0_1_ = 0;
  local_200 = pfVar20;
  if (pfVar20 != (float *)0x0) {
    while (!bVar12) {
      uVar15 = FUN_1802a55d0(local_3b8,
                             "C:\\__w\\1\\s\\CoreEngine\\Native\\TextNormalizer\\BaseNormalizer.cpp"
                             ,0xbf,2);
      uVar15 = FUN_1802ad3d0(uVar15);
      uVar15 = FUN_18006bfb0(uVar15,"Check failed: ");
      pfVar17 = pfVar20;
      if (0xf < *(ulonglong *)(pfVar20 + 6)) {
        pfVar17 = *(float **)pfVar20;
      }
      uVar15 = FUN_180070120(uVar15,pfVar17,*(longlong *)(pfVar20 + 4));
      FUN_18006bfb0(uVar15,&DAT_1825674fc);
      FUN_1802a5ca0(local_3b8);
      puStack_1f8._0_1_ = 1;
      bVar12 = true;
    }
  }
  if (pfVar20 == (float *)0x0) {
    fVar25 = 0.0;
    if (pfVar19 < (float *)0x4) {
LAB_1800dc85a:
      if (pfVar18 < pfVar19) {
        lVar23 = *(longlong *)param_3;
        pfVar20 = *(float **)(param_3 + 2);
        goto LAB_1800dc86d;
      }
    }
    else {
      lVar23 = *(longlong *)param_3;
      local_400 = *(float **)(param_3 + 2);
      fVar25 = 0.0;
      lVar22 = (longlong)local_400 - lVar23 >> 3;
      pfVar20 = (float *)(lVar23 + 0xc);
      pfVar17 = (float *)(lVar23 + -4 + lVar22 * 8);
      do {
        fVar26 = pfVar20[-3];
        fVar29 = pfVar17[-1];
        fVar27 = pfVar20[-2];
        fVar1 = *pfVar17;
        fVar2 = pfVar20[-1];
        fVar3 = pfVar17[-3];
        fVar4 = *pfVar20;
        fVar5 = pfVar17[-2];
        fVar6 = pfVar20[1];
        fVar7 = pfVar17[-5];
        fVar8 = pfVar20[2];
        fVar9 = pfVar17[-4];
        lVar16 = lVar22 - (longlong)pfVar18;
        fVar24 = pfVar20[3] - *(float *)(lVar23 + -0x20 + lVar16 * 8);
        fVar24 = (pfVar20[4] - pfVar17[-6]) * (pfVar20[4] - *(float *)(lVar23 + -0x1c + lVar16 * 8))
                 + fVar24 * fVar24;
        if (fVar24 < 0.0) {
          fVar24 = (float)FUN_18067c840(fVar24);
        }
        else {
          fVar24 = SQRT(fVar24);
        }
        fVar26 = (fVar27 - fVar1) * (fVar27 - fVar1) + (fVar26 - fVar29) * (fVar26 - fVar29);
        if (fVar26 < 0.0) {
          fVar26 = (float)FUN_18067c840(fVar26);
        }
        else {
          fVar26 = SQRT(fVar26);
        }
        fVar29 = (fVar4 - fVar5) * (fVar4 - fVar5) + (fVar2 - fVar3) * (fVar2 - fVar3);
        if (fVar29 < 0.0) {
          fVar29 = (float)FUN_18067c840(fVar29);
        }
        else {
          fVar29 = SQRT(fVar29);
        }
        fVar27 = (fVar8 - fVar9) * (fVar8 - fVar9) + (fVar6 - fVar7) * (fVar6 - fVar7);
        if (fVar27 < 0.0) {
          fVar27 = (float)FUN_18067c840(fVar27);
        }
        else {
          fVar27 = SQRT(fVar27);
        }
        fVar25 = fVar26 + fVar25 + fVar29 + fVar27 + fVar24;
        pfVar18 = pfVar18 + 1;
        pfVar20 = pfVar20 + 8;
        pfVar17 = pfVar17 + -8;
      } while (pfVar18 < (float *)((longlong)pfVar19 - 3U));
      pfVar20 = local_400;
      param_3 = local_408;
      if (pfVar19 <= pfVar18) goto LAB_1800dc85a;
LAB_1800dc86d:
      lVar22 = (longlong)pfVar20 - lVar23 >> 3;
      pfVar20 = (float *)((lVar22 - (longlong)pfVar18) * 8 + -4 + lVar23);
      do {
        lVar16 = lVar22 - (longlong)pfVar18;
        fVar26 = *(float *)(lVar23 + 4 + (longlong)pfVar18 * 8);
        fVar29 = *(float *)(lVar23 + (longlong)pfVar18 * 8) - *(float *)(lVar23 + -8 + lVar16 * 8);
        fVar26 = (fVar26 - *(float *)(lVar23 + -4 + lVar16 * 8)) * (fVar26 - *pfVar20) +
                 fVar29 * fVar29;
        if (fVar26 < 0.0) {
          fVar26 = (float)FUN_18067c840(fVar26);
        }
        else {
          fVar26 = SQRT(fVar26);
        }
        fVar25 = fVar25 + fVar26;
        pfVar18 = (float *)((longlong)pfVar18 + 1);
        pfVar20 = pfVar20 + -2;
      } while (pfVar18 < pfVar19);
    }
    local_200 = (float *)0x0;
    puStack_1f8._0_1_ = 0;
    puStack_1f8._1_7_ = 0;
    pfVar18 = (float *)0x0;
    pfVar20 = pfVar18;
    if (fVar25 <= 0.0) {
      local_208 = 0;
      FUN_1802a5570(&local_208,"avgHeight > 0.f");
      FUN_180067c40(local_208,fVar25);
      uVar15 = FUN_1802a6c20(&local_208);
      FUN_180067c40(uVar15);
      FUN_1802a9110(&local_208,&local_408);
      FUN_1800b3690(&local_208);
      pfVar20 = local_408;
    }
    local_408 = (float *)0x0;
    bVar12 = false;
    puStack_1f8._0_1_ = 0;
    local_200 = pfVar20;
    if (pfVar20 != (float *)0x0) {
      while (!bVar12) {
        uVar15 = FUN_1802a55d0(local_3b8,
                               "C:\\__w\\1\\s\\CoreEngine\\Native\\TextNormalizer\\BaseNormalizer.cpp"
                               ,0xd0,2);
        uVar15 = FUN_1802ad3d0(uVar15);
        uVar15 = FUN_18006bfb0(uVar15,"Check failed: ");
        pfVar17 = pfVar20;
        if (0xf < *(ulonglong *)(pfVar20 + 6)) {
          pfVar17 = *(float **)pfVar20;
        }
        uVar15 = FUN_180070120(uVar15,pfVar17,*(longlong *)(pfVar20 + 4));
        FUN_18006bfb0(uVar15,&DAT_1825674fc);
        FUN_1802a5ca0(local_3b8);
        puStack_1f8._0_1_ = 1;
        bVar12 = true;
      }
    }
    pfVar17 = local_1e0;
    if (pfVar20 == (float *)0x0) {
      *local_1e0 = fVar25 / (float)(longlong)pfVar19;
      local_1d0 = 0;
      lStack_1c8 = 0;
      local_1c0 = 0;
      pfVar20 = (float *)((longlong)pfVar19 - 1);
      local_1e0 = pfVar20;
      if (pfVar20 != (float *)0x0) {
        if ((float *)0x3fffffffffffffff < pfVar20) goto LAB_1800dd1cf;
        FUN_18008ecc0(&local_1d0,&local_1e0);
      }
      local_1e8[0] = 0;
      uVar21 = *(longlong *)(param_3 + 2) - *(longlong *)param_3 >> 3;
      local_1b8 = (undefined8 *)0x0;
      puStack_1b0 = (undefined8 *)0x0;
      local_1a8 = (undefined8 *)0x0;
      if (uVar21 != 0) {
        if (0x1fffffffffffffff < uVar21) {
                    /* WARNING: Subroutine does not return */
          FUN_18002b5a0();
        }
        local_1b8 = (undefined8 *)FUN_18002b510(uVar21 * 8);
        local_1a8 = local_1b8 + uVar21;
        puStack_1b0 = local_1b8;
        do {
          *puStack_1b0 = 0;
          puStack_1b0 = puStack_1b0 + 1;
          uVar21 = uVar21 - 1;
        } while (uVar21 != 0);
      }
      local_420 = &local_1b8;
      local_428 = (longlong *)local_1e8;
      local_430 = &local_1d0;
      local_438 = local_3d8;
      (*(code *)PTR__guard_dispatch_icall_1806c86a0)(local_3e0,*pfVar17,pfVar19,param_3);
      local_200 = (float *)0x0;
      puStack_1f8._0_1_ = 0;
      puStack_1f8._1_7_ = 0;
      pfVar19 = pfVar18;
      if (lStack_1c8 - local_1d0 >> 2 == 0) {
        local_208 = 0;
        FUN_1802a5570(&local_208,"vWidth.size() > 0");
        FUN_180067e80(local_208,0);
        uVar15 = FUN_1802a6c20(&local_208);
        FUN_1800682e0(uVar15,0);
        FUN_1802a9110(&local_208,&local_408);
        FUN_1800b3690(&local_208);
        pfVar19 = local_408;
      }
      local_408 = (float *)0x0;
      bVar12 = false;
      puStack_1f8._0_1_ = 0;
      local_200 = pfVar19;
      if (pfVar19 != (float *)0x0) {
        while (!bVar12) {
          uVar15 = FUN_1802a55d0(local_3b8,
                                 "C:\\__w\\1\\s\\CoreEngine\\Native\\TextNormalizer\\BaseNormalizer.cpp"
                                 ,0xda,2);
          uVar15 = FUN_1802ad3d0(uVar15);
          uVar15 = FUN_18006bfb0(uVar15,"Check failed: ");
          pfVar17 = pfVar19;
          if (0xf < *(ulonglong *)(pfVar19 + 6)) {
            pfVar17 = *(float **)pfVar19;
          }
          uVar15 = FUN_180070120(uVar15,pfVar17,*(longlong *)(pfVar19 + 4));
          FUN_18006bfb0(uVar15,&DAT_1825674fc);
          FUN_1802a5ca0(local_3b8);
          puStack_1f8._0_1_ = 1;
          bVar12 = true;
        }
      }
      if (pfVar19 == (float *)0x0) {
        fVar25 = *(float *)(lStack_1c8 + -4);
        local_200 = (float *)0x0;
        puStack_1f8._0_1_ = 0;
        puStack_1f8._1_7_ = 0;
        pfVar19 = pfVar18;
        if (fVar25 <= 0.0) {
          local_208 = 0;
          FUN_1802a5570(&local_208,"fPointsWidth > 0");
          FUN_180067c40(local_208,fVar25);
          uVar15 = FUN_1802a6c20(&local_208);
          FUN_1800682e0(uVar15,0);
          FUN_1802a9110(&local_208,&local_408);
          FUN_1800b3690(&local_208);
          pfVar19 = local_408;
        }
        local_408 = (float *)0x0;
        bVar12 = false;
        puStack_1f8._0_1_ = 0;
        local_200 = pfVar19;
        if (pfVar19 != (float *)0x0) {
          while (!bVar12) {
            uVar15 = FUN_1802a55d0(local_3b8,
                                   "C:\\__w\\1\\s\\CoreEngine\\Native\\TextNormalizer\\BaseNormalizer.cpp"
                                   ,0xdd,2);
            uVar15 = FUN_1802ad3d0(uVar15);
            uVar15 = FUN_18006bfb0(uVar15,"Check failed: ");
            pfVar17 = pfVar19;
            if (0xf < *(ulonglong *)(pfVar19 + 6)) {
              pfVar17 = *(float **)pfVar19;
            }
            uVar15 = FUN_180070120(uVar15,pfVar17,*(longlong *)(pfVar19 + 4));
            FUN_18006bfb0(uVar15,&DAT_1825674fc);
            FUN_1802a5ca0(local_3b8);
            puStack_1f8._0_1_ = 1;
            bVar12 = true;
          }
        }
        iVar14 = local_1e8[0];
        if (pfVar19 == (float *)0x0) {
          local_200 = (float *)0x0;
          puStack_1f8._0_1_ = 0;
          puStack_1f8._1_7_ = 0;
          pfVar19 = pfVar18;
          if (local_1e8[0] < 1) {
            local_208 = 0;
            FUN_1802a5570(&local_208,"nHeight > 0");
            FUN_1800682e0(local_208,iVar14);
            uVar15 = FUN_1802a6c20(&local_208);
            FUN_1800682e0(uVar15,0);
            FUN_1802a9110(&local_208,&local_408);
            FUN_1800b3690(&local_208);
            pfVar19 = local_408;
          }
          local_408 = (float *)0x0;
          bVar12 = false;
          puStack_1f8._0_1_ = 0;
          local_200 = pfVar19;
          if (pfVar19 != (float *)0x0) {
            while (!bVar12) {
              uVar15 = FUN_1802a55d0(local_3b8,
                                     "C:\\__w\\1\\s\\CoreEngine\\Native\\TextNormalizer\\BaseNormalizer.cpp"
                                     ,0xde,2);
              uVar15 = FUN_1802ad3d0(uVar15);
              uVar15 = FUN_18006bfb0(uVar15,"Check failed: ");
              pfVar17 = pfVar19;
              if (0xf < *(ulonglong *)(pfVar19 + 6)) {
                pfVar17 = *(float **)pfVar19;
              }
              uVar15 = FUN_180070120(uVar15,pfVar17,*(longlong *)(pfVar19 + 4));
              FUN_18006bfb0(uVar15,&DAT_1825674fc);
              FUN_1802a5ca0(local_3b8);
              puStack_1f8._0_1_ = 1;
              bVar12 = true;
            }
          }
          if (pfVar19 == (float *)0x0) {
            local_138 = 0;
            uStack_130 = 0;
            local_128 = 0;
            uStack_120 = 0;
            local_118 = 0;
            uStack_110 = 0;
            local_108 = 0;
            uStack_100 = 0;
            local_f8 = 0;
            uStack_f0 = 0;
            local_e8 = 0;
            uStack_e0 = 0;
            FUN_180396ff0(&local_138);
            local_200 = (float *)CONCAT44(local_200._4_4_,0x2010000);
            puStack_1f8 = &local_138;
            local_438 = 0;
            uVar28 = (undefined4)DAT_182576418;
            local_1f0 = pfVar19;
            FUN_18039cd80(local_1d8,&local_200,0x15,uVar28);
            local_408 = (float *)CONCAT44(local_1e8[0],(int)(fVar25 + DAT_1825763b0));
            local_198 = 0;
            uStack_190 = 0;
            local_188 = 0;
            uStack_180 = 0;
            local_178 = 0;
            uStack_170 = 0;
            local_168 = 0;
            uStack_160 = 0;
            local_158 = 0;
            uStack_150 = 0;
            local_148 = 0;
            uStack_140 = 0;
            uVar15 = FUN_1803b59f0(local_368,local_408,0x15);
            FUN_1800db570(uVar15,&local_198);
            FUN_180397090(local_298);
            FUN_180397090(local_2f8);
            FUN_180397090(local_358);
            plVar13 = local_3d0;
            local_1d8 = (lStack_1c8 - local_1d0 >> 2) * 0x20 + 1;
            if ((ulonglong)(local_3d0[2] - *local_3d0 >> 2) < local_1d8) {
              if (0x3fffffffffffffff < local_1d8) {
                    /* WARNING: Subroutine does not return */
                FUN_18002b5a0();
              }
              FUN_18008ecc0(local_3d0,&local_1d8);
            }
            local_208 = CONCAT44(local_208._4_4_,0x3f800000);
            puVar10 = (undefined4 *)plVar13[1];
            if (puVar10 == (undefined4 *)plVar13[2]) {
              FUN_180072fe0(plVar13,puVar10,&local_208);
            }
            else {
              *puVar10 = 0x3f800000;
              plVar13[1] = (longlong)(puVar10 + 1);
            }
            if (pfVar20 != (float *)0x0) {
              do {
                local_428 = plVar13;
                local_430._0_1_ = 1;
                local_438 = (longlong)(int)pfVar18;
                FUN_1800defd0(&local_138,&local_198,param_3,&local_1b8);
                local_428 = plVar13;
                local_430 = (longlong *)((ulonglong)local_430._1_7_ << 8);
                local_438 = (longlong)(int)pfVar18;
                FUN_1800defd0(&local_138,&local_198,param_3,&local_1b8);
                pfVar18 = (float *)((longlong)pfVar18 + 1);
              } while (pfVar18 < pfVar20);
            }
            local_1f0 = (float *)0x0;
            local_200 = (float *)CONCAT44(local_200._4_4_,0x2010000);
            puStack_1f8 = (undefined8 *)local_3c8;
            local_438 = 0;
            FUN_18039cd80(&local_198,&local_200,0x10,uVar28);
            FUN_180397090(&local_198);
            FUN_180397090(&local_138);
            FUN_18002ab40(&local_1b8);
            FUN_180069f30(&local_1d0);
            FUN_180642b80(local_d8 ^ (ulonglong)auStack_458);
            return;
          }
          goto LAB_1800dd231;
        }
      }
      else {
        uVar15 = FUN_180068600(&local_200);
        FUN_1801b7650(local_3f8,uVar15);
        FUN_180661808(local_3f8,&DAT_1826195a8);
      }
      uVar15 = FUN_180068600(&local_200);
      FUN_1801b7650(local_3f8,uVar15);
      FUN_180661808(local_3f8,&DAT_1826195a8);
LAB_1800dd231:
      uVar15 = FUN_180068600(&local_200);
      FUN_1801b7650(local_3f8,uVar15);
      FUN_180661808(local_3f8,&DAT_1826195a8);
      pcVar11 = (code *)swi(3);
      (*pcVar11)();
      return;
    }
  }
  else {
    uVar15 = FUN_180068600(&local_200);
    FUN_1801b7650(local_3f8,uVar15);
    FUN_180661808(local_3f8,&DAT_1826195a8);
  }
  uVar15 = FUN_180068600(&local_200);
  FUN_1801b7650(local_3f8,uVar15);
  FUN_180661808(local_3f8,&DAT_1826195a8);
LAB_1800dd1cf:
                    /* WARNING: Subroutine does not return */
  FUN_18002b5a0();
}

