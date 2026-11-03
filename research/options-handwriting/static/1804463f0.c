// 1804463f0 OcrProcessOptionsSetResizeResolution

undefined8 OcrProcessOptionsSetResizeResolution(longlong param_1,int param_2,int param_3)

{
                    /* 0x4463f0  5  OcrProcessOptionsSetResizeResolution */
  if (((param_1 != 0) && (param_2 < 0x481)) && (param_3 < 0x301)) {
    *(int *)(param_1 + 4) = param_2;
    *(int *)(param_1 + 8) = param_3;
    return 0;
  }
  return 4;
}

