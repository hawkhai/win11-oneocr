@echo off
REM ============================================
REM  win11-oneocr 构建脚本 (VS2019 + CMake)
REM ============================================
REM  前提: 已安装 VS2019 BuildTools 和 CMake
REM  依赖: stb_image.h (已包含在项目中, 无需 OpenCV)
REM  运行时: bin/ 下的 DLL 和模型文件会自动拷贝到输出目录
REM ============================================

REM 1. CMake 配置 (生成 VS2019 x64 工程)
echo [1/3] CMake configure ...
cmake -G "Visual Studio 16 2019" -A x64 -B build -S .
if %errorlevel% neq 0 (
    echo CMake configure failed!
    pause
    exit /b 1
)

REM 2. 编译 Release
echo [2/3] Building Release ...
cmake --build build --config Release
if %errorlevel% neq 0 (
    echo Build failed!
    pause
    exit /b 1
)

REM 3. 运行测试 (需要从 exe 目录运行, 因为 oneocr.dll 用相对路径加载模型)
echo [3/3] Running OCR test ...
pushd build\Release
ocr.exe ocr-book.jpg
popd

echo.
echo Done. Output files are in: build\Release\
pause
