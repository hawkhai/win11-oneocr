# Third-party components and model ownership

The native C++ implementation and its interfaces are original project code,
distributed under the project's existing MIT designation. The components below
retain their own notices. No license here grants rights to Microsoft's model
weights or makes the extracted models MIT-licensed.

| Component | Version/source | Terms and integration |
| --- | --- | --- |
| ONNX Runtime | v1.23.2, `a83fc4d58cb48eb68890dd689f94f28288cf2278` | MIT plus dependency notices; see `licenses/onnxruntime-LICENSE.txt` and `licenses/onnxruntime-ThirdPartyNotices.txt`. Can be built from source. |
| OpenCV | 4.10.0, `71d3237a093b60a27601c20e9ee6c3e52154e8b1` | Apache-2.0; see `licenses/opencv-Apache-2.0.txt`, `licenses/opencv-zlib.txt` and `licenses/opencv-softfloat-fdlibm.txt`. Only core/imgproc are built from pinned source and linked statically for native normalization, rasterization and adaptive thresholding. Source tree retains upstream third-party notices. |
| ICU | 74.2, `2d029329c82c7792b985024b2bdab5fc7278fbc8` | Unicode-3.0; see `licenses/icu-Unicode-3.0.txt`. Common library and embedded Unicode/Thai dictionary data are built from pinned source. Host data-generation tools are build dependencies, not runtime dependencies. |
| nlohmann/json | Existing `json.hpp`, v3.9.1 | MIT; Niels Lohmann's complete notice is retained in the header and `licenses/nlohmann-json-MIT.txt`. |
| stb_image | Existing `stb_image.h`, v2.30 | MIT or public-domain alternative; original dual-license text remains in the header. |
| Python reference | bropines/oneocr-onnx-python, `75cc12666503425ffd6ea0cb052c0bcaaff21058` | MIT. Used as an inspected reference and optional benchmark; its source is not linked into the native runtime. Its license explicitly excludes Microsoft assets. |
| Container format reference | shiyori/oneocr-native, `6020d4339ce2feb835393e2512e433baec29f8d3`, `internal/engine/onemodel_container.go` | AGPL-3.0-only. The derived standalone `scripts/unpack-onemodel.py` is separately marked AGPL-3.0-only; full license in `licenses/oneocr-native-AGPL-3.0.txt`. The preparation program is not linked into or executed by the native C++ runtime. The current sample's CFB128 encoding was additionally verified locally. |
| PyCryptodome | 3.23.0 | Public domain and BSD-2-Clause; development/model preparation only. |
| reverse-skill | `cab634bd855fc287f6e420c1f36fd1a6b9245960` | MIT routing/research tools, downloaded to ignored `work/vendor/`. Not a runtime dependency. |
| Ghidra / Temurin | 11.3.2 / 21.0.8+9 | Research tools only, not included in runtime packages; retain their upstream licenses in their installations. |
| Handwritten regression sample | Azure-Samples/cognitive-services-sample-data-files, `65af3b93cab5f45c76594f3265210646d4b3809e`, ComputerVision/Images/handwritten_text.jpg | MIT; copyright Microsoft Corporation. Original and deterministic rotated fixtures in `samples/oneocr-parity/`; complete notice in `samples/oneocr-parity/LICENSE.azure.txt`. Test data only; unrelated to model ownership. |
| OneOCR assets | Local `oneocr.onemodel`, original DLLs, extracted graphs/dictionaries/calibration tables | Microsoft assets; retain ownership and source records. Existing tracked samples are preserved. Newly extracted resources stay in ignored directories and native npm packages exclude these assets. No redistribution permission is inferred from this repository's code license. |

The legacy implementation's upstream credits remain in README.md. New native
packages exclude the original `oneocr.dll` and encrypted `oneocr.onemodel`.
Their `onnxruntime.dll`/`libonnxruntime` is the open-source inference dependency,
not the closed OneOCR engine. A source-only build recipe is provided.

Offline operation requires no OCR account, certificate, activation endpoint,
or online authorization service. This is an execution property, not a change
to the ownership of any source or model assets. No model has been uploaded
or published by this work.
