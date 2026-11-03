// 1804463d0 OcrProcessOptionsSetMaxRecognitionLineCount

undefined8 OcrProcessOptionsSetMaxRecognitionLineCount(longlong param_1,uint param_2)

{
                    /* 0x4463d0  7  OcrProcessOptionsSetMaxRecognitionLineCount */
  if ((param_1 != 0) && (param_2 < 0x3e9)) {
    *(uint *)(param_1 + 0x70) = param_2;
    return 0;
  }
  return 4;
}

