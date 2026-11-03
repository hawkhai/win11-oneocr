// 1800f9680 FUN_1800f9680

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_1800f9680(longlong param_1,int *param_2,undefined8 param_3,longlong param_4,
                  longlong param_5)

{
  int *piVar1;
  code *pcVar2;
  bool bVar3;
  undefined4 uVar4;
  float fVar5;
  longlong *plVar6;
  undefined8 uVar7;
  longlong lVar8;
  longlong lVar9;
  undefined8 *puVar10;
  int iVar11;
  undefined8 *puVar12;
  int *piVar13;
  undefined1 uVar14;
  longlong *plVar15;
  longlong lVar16;
  ulonglong uVar17;
  uint uVar18;
  uint uVar19;
  int iVar20;
  int iVar21;
  ulonglong uVar22;
  undefined8 uVar23;
  undefined1 auStack_308 [32];
  int *local_2e8;
  int *local_2e0;
  undefined1 local_2d8;
  int local_2d0;
  undefined8 local_2b8;
  undefined4 local_2b0;
  undefined1 local_2a8 [4];
  int local_2a4;
  int local_2a0;
  uint local_29c;
  uint local_298;
  undefined8 local_290;
  longlong lStack_288;
  longlong local_280;
  int *local_278;
  int *local_270;
  longlong local_268;
  undefined8 *local_260;
  undefined4 local_258;
  undefined4 uStack_254;
  longlong local_250;
  longlong local_248;
  longlong local_238;
  longlong local_230;
  longlong local_228;
  undefined1 local_220;
  longlong local_218;
  longlong local_210;
  undefined8 local_208;
  int *local_1e8;
  longlong local_1e0;
  longlong lStack_1d8;
  longlong local_1d0;
  longlong *local_1c8;
  longlong *plStack_1c0;
  longlong *local_1b8;
  longlong *plStack_1b0;
  undefined8 local_1a8;
  longlong local_1a0;
  longlong lStack_198;
  longlong local_190;
  undefined1 local_188 [16];
  undefined4 *local_178;
  longlong *local_170;
  undefined8 *local_168;
  ulonglong uStack_160;
  longlong *local_158;
  undefined8 local_150;
  undefined8 uStack_148;
  undefined8 uStack_140;
  undefined8 uStack_138;
  undefined8 *local_130;
  longlong lStack_128;
  undefined4 local_118;
  undefined4 uStack_114;
  undefined4 uStack_110;
  undefined4 uStack_10c;
  undefined4 local_108;
  undefined4 uStack_104;
  int iStack_100;
  int aiStack_fc [33];
  ulonglong local_78;
  
  local_78 = DAT_182625200 ^ (ulonglong)auStack_308;
  local_250 = param_5;
  local_29c = 0;
  local_278 = param_2;
  local_268 = param_1;
  local_248 = param_4;
  local_1e8 = param_2;
  FUN_180058400(&local_170);
  plVar6 = (longlong *)FUN_1800f7370(param_3);
  lVar9 = (plVar6[1] - *plVar6) / 6 + (plVar6[1] - *plVar6 >> 0x3f);
  lVar9 = (lVar9 >> 4) - (lVar9 >> 0x3f);
  local_290 = lVar9;
  uVar23 = FUN_1800eb380(param_2);
  uVar19 = 1;
  local_298 = 1;
  local_29c = 1;
  local_188 = ZEXT816(0);
  local_178 = (undefined4 *)0x0;
  iVar20 = (int)lVar9;
  plVar15 = (longlong *)(longlong)iVar20;
  local_158 = plVar15;
  if (iVar20 != 0) {
    if ((longlong *)0x1e1e1e1e1e1e1e1 < plVar15) goto LAB_1800fa22f;
    uVar23 = FUN_1800eba00(local_188,&local_158);
  }
  FUN_1801130f0(uVar23,&local_258,*(undefined4 *)(param_4 + 4),*(undefined4 *)(param_4 + 8));
  uVar23 = DAT_1825763b8;
  uVar4 = DAT_1825763b0;
  iVar21 = -1;
  iVar20 = -1;
  local_2a4 = -1;
  local_2a0 = -1;
  piVar13 = (int *)*plVar6;
  local_270 = (int *)plVar6[1];
  if (piVar13 != local_270) {
    do {
      local_168 = (undefined8 *)0x0;
      uStack_160 = 0;
      iVar11 = (*piVar13 >> 3 & 0x1ffU) + 1;
      if (iVar11 == 3) {
        uVar19 = uVar19 | 4;
        puVar12 = (undefined8 *)0x0;
      }
      else {
        local_1a8 = 0;
        FUN_1802a5570(&local_1a8,"rawImage.channels() == s_fImageMeans.size()");
        FUN_1800682e0(local_1a8,iVar11);
        uVar7 = FUN_1802a6c20(&local_1a8);
        FUN_180067e80(uVar7,3);
        FUN_1802a9110(&local_1a8,&local_260);
        FUN_1800b3690(&local_1a8);
        uVar19 = uVar19 | 0xc;
        puVar12 = local_260;
      }
      local_260 = (undefined8 *)0x0;
      bVar3 = false;
      uStack_160 = uStack_160 & 0xffffffffffffff00;
      local_29c = uVar19;
      local_298 = uVar19;
      local_168 = puVar12;
      if (puVar12 != (undefined8 *)0x0) {
        while (!bVar3) {
          uVar7 = FUN_1802a55d0(&local_238,
                                "C:\\__w\\1\\s\\CoreEngine\\Native\\TextDetector\\BaseRCNN.cpp",
                                0x133,2);
          uVar7 = FUN_1802ad3d0(uVar7);
          uVar7 = FUN_18006bfb0(uVar7,"Check failed: ");
          puVar10 = puVar12;
          if (0xf < (ulonglong)puVar12[3]) {
            puVar10 = (undefined8 *)*puVar12;
          }
          uVar7 = FUN_180070120(uVar7,puVar10,puVar12[2]);
          FUN_18006bfb0(uVar7,&DAT_1825674fc);
          FUN_1802a5ca0(&local_238);
          uStack_160 = CONCAT71(uStack_160._1_7_,1);
          bVar3 = true;
        }
      }
      if (puVar12 != (undefined8 *)0x0) {
        uVar23 = FUN_180068600(&local_168);
        FUN_1801b7650(&local_1c8,uVar23);
        FUN_180661808(&local_1c8,&DAT_1826195a8);
        pcVar2 = (code *)swi(3);
        (*pcVar2)();
        return;
      }
      local_118 = 0;
      uStack_114 = 0;
      uStack_110 = 0;
      uStack_10c = 0;
      local_108 = 0;
      uStack_104 = 0;
      iStack_100 = 0;
      aiStack_fc[0] = 0;
      aiStack_fc[1] = 0;
      aiStack_fc[2] = 0;
      aiStack_fc[3] = 0;
      aiStack_fc[4] = 0;
      aiStack_fc[5] = 0;
      aiStack_fc[6] = 0;
      aiStack_fc[7] = 0;
      aiStack_fc[8] = 0;
      aiStack_fc[9] = 0;
      aiStack_fc[10] = 0;
      aiStack_fc[0xb] = 0;
      aiStack_fc[0xc] = 0;
      aiStack_fc[0xd] = 0;
      aiStack_fc[0xe] = 0;
      aiStack_fc[0xf] = 0;
      aiStack_fc[0x10] = 0;
      aiStack_fc[0x11] = 0;
      aiStack_fc[0x12] = 0;
      aiStack_fc[0x13] = 0;
      aiStack_fc[0x14] = 0;
      aiStack_fc[0x15] = 0;
      aiStack_fc[0x16] = 0;
      aiStack_fc[0x17] = 0;
      aiStack_fc[0x18] = 0;
      aiStack_fc[0x19] = 0;
      aiStack_fc[0x1a] = 0;
      local_2b0 = 0;
      if (*(char *)(local_248 + 0xc) == '\0') {
        local_2b0 = uVar4;
      }
      local_2b8 = uVar23;
      local_2d0 = CONCAT31(local_2d0._1_3_,*(undefined1 *)(param_1 + 0x58));
      local_2d8 = *(undefined1 *)(param_1 + 0x4c);
      local_2e0 = (int *)CONCAT44(local_2e0._4_4_,uStack_254);
      local_2e8 = (int *)CONCAT44(local_2e8._4_4_,local_258);
      FUN_180113400(param_1,&local_118,piVar13,*(undefined4 *)(param_1 + 0x50));
      if (iVar21 < aiStack_fc[0]) {
        iVar21 = aiStack_fc[0];
      }
      if (iVar20 < iStack_100) {
        iVar20 = iStack_100;
      }
      piVar1 = *(int **)(local_278 + 0x2a);
      local_2a4 = iVar21;
      local_2a0 = iVar20;
      if (piVar1 == *(int **)(local_278 + 0x2c)) {
        local_2e0 = &iStack_100;
        local_2e8 = aiStack_fc;
        FUN_1800ff480(local_278 + 0x28,piVar1,piVar13 + 3,piVar13 + 2);
      }
      else {
        iVar11 = piVar13[2];
        *piVar1 = piVar13[3];
        piVar1[1] = iVar11;
        piVar1[2] = aiStack_fc[0];
        piVar1[3] = iStack_100;
        piVar1[4] = aiStack_fc[0];
        piVar1[5] = iStack_100;
        *(undefined1 *)(piVar1 + 6) = 0;
        *(longlong *)(local_278 + 0x2a) = *(longlong *)(local_278 + 0x2a) + 0x1c;
      }
      uVar7 = local_188._8_8_;
      if ((undefined4 *)local_188._8_8_ == local_178) {
        FUN_1800ebf20(local_188,local_188._8_8_,&local_118);
      }
      else {
        *(undefined4 *)local_188._8_8_ = local_118;
        *(undefined4 *)(local_188._8_8_ + 4) = uStack_114;
        *(undefined4 *)(local_188._8_8_ + 8) = uStack_110;
        *(undefined4 *)(local_188._8_8_ + 0xc) = uStack_10c;
        FUN_180395c10((undefined4 *)(local_188._8_8_ + 0x10),&local_108);
        *(ulonglong *)(uVar7 + 0x70) = CONCAT44(aiStack_fc[0x16],aiStack_fc[0x15]);
        *(ulonglong *)(uVar7 + 0x78) = CONCAT44(aiStack_fc[0x18],aiStack_fc[0x17]);
        *(int *)(uVar7 + 0x80) = aiStack_fc[0x19];
        local_188._8_8_ = local_188._8_8_ + 0x88;
      }
      FUN_180397090(&local_108);
      piVar13 = piVar13 + 0x18;
      plVar15 = local_158;
    } while (piVar13 != local_270);
  }
  uVar18 = (*(int *)(local_188._0_8_ + 0x10) >> 3 & 0x1ffU) + 1;
  local_1a8 = CONCAT44(local_1a8._4_4_,uVar18);
  plVar6 = (longlong *)FUN_180058400(&local_270);
  fVar5 = DAT_18257653c;
  *(float *)(local_250 + 0x34) = (float)(*plVar6 - (longlong)local_170) / DAT_18257653c;
  FUN_180058400(&local_270);
  iVar11 = local_2a0;
  iVar20 = local_2a4;
  local_1a0 = 0;
  lStack_198 = 0;
  lVar9 = 0;
  local_190 = 0;
  if (*(char *)(local_268 + 0x4d) == '\0') {
    local_1e0 = 0;
    lStack_1d8 = 0;
    uVar22 = 0;
    local_1d0 = 0;
    local_170 = plVar15;
    if (plVar15 != (longlong *)0x0) {
      if ((longlong *)0x2aaaaaaaaaaaaaa < plVar15) {
                    /* WARNING: Subroutine does not return */
        FUN_18002b5a0();
      }
      FUN_1800ebaa0(&local_1e0,&local_170);
    }
    lVar16 = (longlong)(local_188._8_8_ - local_188._0_8_) >> 0x3f;
    uVar17 = uVar22;
    if ((longlong)(local_188._8_8_ - local_188._0_8_) / 0x88 + lVar16 != lVar16) {
      do {
        if (lStack_1d8 == local_1d0) {
          FUN_1800ec500(&local_1e0,lStack_1d8);
        }
        else {
          FUN_180395d50(lStack_1d8,local_188._0_8_ + 0x10 + uVar22);
          lStack_1d8 = lStack_1d8 + 0x60;
        }
        uVar17 = uVar17 + 1;
        uVar22 = uVar22 + 0x88;
      } while (uVar17 < (ulonglong)((longlong)(local_188._8_8_ - local_188._0_8_) / 0x88));
    }
    local_2e8 = (int *)CONCAT44(local_2e8._4_4_,local_2a0);
    plVar15 = (longlong *)
              FUN_1801b0bd0(&local_1c8,&local_1e0,
                            CONCAT71((int7)((ulonglong)local_188._0_8_ >> 8),1),iVar21);
    if (&local_1a0 != plVar15) {
      local_1a0 = *plVar15;
      lStack_198 = plVar15[1];
      lVar9 = plVar15[2];
      *plVar15 = 0;
      plVar15[1] = 0;
      plVar15[2] = 0;
      local_190 = lVar9;
    }
    lVar16 = local_1a0;
    FUN_180069f30(&local_1c8);
    FUN_18002aba0(&local_1e0);
  }
  else {
    uVar22 = 0;
    lVar16 = (longlong)(local_188._8_8_ - local_188._0_8_) >> 0x3f;
    if ((longlong)(local_188._8_8_ - local_188._0_8_) / 0x88 + lVar16 == lVar16) {
      lVar16 = 0;
      iVar21 = local_2a4;
    }
    else {
      piVar13 = (int *)(local_268 + 0x40);
      lVar9 = 0;
      lVar16 = 0;
      do {
        local_2d0 = iVar11 - *(int *)(*(longlong *)(local_278 + 0x28) + 0xc + lVar9);
        local_1c8 = (longlong *)0x0;
        plStack_1c0 = (longlong *)0x0;
        local_1b8 = (longlong *)0x0;
        local_2e8 = piVar13;
        FUN_1800fe6a0(&local_1c8,local_188._0_8_ + 0x10 + lVar16,
                      iVar20 - *(int *)(*(longlong *)(local_278 + 0x28) + 8 + lVar9),uVar4);
        FUN_1800df250(&local_1a0,lStack_198,local_1c8,
                      (longlong)plStack_1c0 - (longlong)local_1c8 >> 2);
        FUN_180069f30(&local_1c8);
        uVar22 = uVar22 + 1;
        lVar16 = lVar16 + 0x88;
        lVar9 = lVar9 + 0x1c;
      } while (uVar22 < (ulonglong)((longlong)(local_188._8_8_ - local_188._0_8_) / 0x88));
      lVar16 = local_1a0;
      lVar9 = local_190;
      uVar19 = local_298;
      iVar21 = local_2a4;
      uVar18 = (uint)local_1a8;
    }
  }
  iVar20 = local_2a0;
  local_1e0 = 0;
  lStack_1d8 = 0;
  local_1d0 = 0;
  plStack_1c0 = (longlong *)(longlong)local_2a0;
  local_1b8 = (longlong *)(longlong)iVar21;
  if (*(char *)(local_268 + 0x4e) == '\0') {
    local_1c8 = (longlong *)(ulonglong)uVar18;
    FUN_1800887a0(&local_1e0,3);
    lVar8 = local_1e0;
    FUN_180698e50(local_1e0,&local_1c8,0x18);
    lStack_1d8 = lVar8 + 0x18;
  }
  else {
    local_1c8 = local_158;
    plVar15 = plStack_1c0;
    plStack_1c0 = (longlong *)(ulonglong)uVar18;
    plStack_1b0 = local_1b8;
    local_1b8 = plVar15;
    FUN_1800887a0(&local_1e0,4);
    lVar8 = local_1e0;
    FUN_180698e50(local_1e0,&local_1c8,0x20);
    lStack_1d8 = lVar8 + 0x20;
  }
  plVar15 = (longlong *)FUN_180058400(&local_170);
  piVar13 = local_278;
  *(float *)(local_250 + 0x38) = (float)(*plVar15 - (longlong)local_270) / fVar5;
  *local_278 = (int)local_290;
  local_278[1] = iVar21;
  local_278[2] = iVar20;
  local_1c8 = (longlong *)0x0;
  plStack_1c0 = (longlong *)0x0;
  local_1b8 = (longlong *)0x0;
  plStack_1b0 = (longlong *)0x0;
  FUN_18002b200(&local_1c8,&DAT_182568cf0,4);
  local_150 = (ulonglong)local_150._4_4_ << 0x20;
  uStack_148 = 0;
  uStack_140 = 0;
  uStack_138 = 0;
  local_130 = (undefined8 *)0x0;
  lStack_128 = 0;
  local_168 = (undefined8 *)(lStack_198 - lVar16 >> 2);
  if ((local_168 != (undefined8 *)0xffffffffffffffff) &&
     ((lVar16 != 0 || (uStack_160 = 0, local_168 == (undefined8 *)0x0)))) {
    uStack_160 = lVar16;
    local_130 = local_168;
    lStack_128 = lVar16;
    FUN_1800880a0(&uStack_148,&local_1e0);
    local_230 = lStack_198;
    local_208 = local_1d0;
    local_210 = lStack_1d8;
    local_218 = local_1e0;
    local_190 = 0;
    lStack_198 = 0;
    local_1a0 = 0;
    local_220 = 0;
    local_1d0 = 0;
    lStack_1d8 = 0;
    local_1e0 = 0;
    piVar1 = piVar13 + 4;
    local_238 = lVar16;
    local_228 = lVar9;
    FUN_180096ff0(piVar1,&local_1c8,&local_150,&local_238);
    FUN_18002ab40(&local_218);
    FUN_180066740(&local_238);
    FUN_18002ab40(&uStack_148);
    FUN_18002acd0(&local_1c8);
    local_290 = CONCAT44((float)piVar13[1],(float)piVar13[2]);
    lStack_288 = CONCAT44(lStack_288._4_4_,(float)*piVar13);
    local_1c8 = (longlong *)0x0;
    plStack_1c0 = (longlong *)0x0;
    plVar15 = (longlong *)FUN_18002b510(0xc);
    local_1b8 = (longlong *)((longlong)plVar15 + 0xc);
    local_1c8 = plVar15;
    FUN_180698e50(plVar15,&local_290,0xc);
    plStack_1c0 = (longlong *)((longlong)plVar15 + 0xc);
    local_29c = uVar19 | 2;
    local_168 = _DAT_1825766a0;
    uStack_160 = _UNK_1825766a8;
    local_290 = 0;
    lStack_288 = 0;
    lVar9 = FUN_18002b510(0x10);
    local_280 = lVar9 + 0x10;
    local_290 = lVar9;
    FUN_180698e50(lVar9,&local_168,0x10);
    lStack_288 = lVar9 + 0x10;
    local_150 = 0;
    uStack_148 = 0;
    uStack_140 = 0;
    uStack_138 = 0;
    FUN_18002b200(&local_150,"im_info",7);
    FUN_180096b70(piVar1,&local_150,&local_290,&local_1c8);
    FUN_18002acd0(&local_150);
    FUN_18002ab40(&local_290);
    FUN_180069f30(&local_1c8);
    lVar16 = local_248;
    lVar9 = local_268;
    if (*(char *)(local_268 + 0x21a) != '\0') {
      lVar8 = FUN_180028940(&local_118,local_248);
      if ((*(int *)(lVar8 + 0x60) - 0x3e9U < 3) || (*(int *)(lVar8 + 0x60) == 0x3ed)) {
        bVar3 = true;
      }
      else {
        bVar3 = false;
      }
      FUN_18002acd0(lVar8 + 0x38);
      FUN_18002acd0(lVar8 + 0x10);
      if (bVar3) {
        lVar16 = FUN_180028940(&local_118,lVar16);
        if ((*(int *)(lVar16 + 0x60) - 0x3e9U < 3) || (*(int *)(lVar16 + 0x60) == 0x3ed)) {
          uVar14 = 1;
        }
        else {
          uVar14 = 0;
        }
        FUN_18002acd0(lVar16 + 0x38);
        FUN_18002acd0(lVar16 + 0x10);
        local_290 = 1;
        local_1c8 = (longlong *)0x0;
        plStack_1c0 = (longlong *)0x0;
        local_2a8[0] = uVar14;
        local_1c8 = (longlong *)FUN_18002b510(8);
        local_1b8 = local_1c8 + 1;
        *local_1c8 = local_290;
        plStack_1c0 = local_1c8 + 1;
        FUN_1800f7190(piVar1,lVar9 + 0xf8,&local_1c8,local_2a8);
        FUN_18002ab40(&local_1c8);
      }
    }
    FUN_18002ab40(&local_1e0);
    FUN_180069f30(&local_1a0);
    FUN_1800eb7c0(local_188);
    FUN_180642b80(local_78 ^ (ulonglong)auStack_308);
    return;
  }
  Unwind_1800852f0();
LAB_1800fa22f:
                    /* WARNING: Subroutine does not return */
  FUN_18002b5a0();
}

