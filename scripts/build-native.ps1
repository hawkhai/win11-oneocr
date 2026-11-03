param(
    [string]$SdkRoot = '',
    [switch]$SourceRuntime,
    [string]$IcuSource = 'work/vendor/icu-source',
    [string]$OpenCvSource = '',
    [string]$IcuData = '',
    [string]$BuildDirectory = 'build/source-runtime',
    [int]$Jobs = 6
)
$ErrorActionPreference = 'Stop'
$ProjectRoot = Split-Path $PSScriptRoot -Parent
Push-Location $ProjectRoot
try {
    $cmakeCommand = Get-Command cmake -ErrorAction SilentlyContinue
    if ($cmakeCommand) { $cmake = $cmakeCommand.Source } else {
        $vswhere = Join-Path ${env:ProgramFiles(x86)} 'Microsoft Visual Studio\Installer\vswhere.exe'
        if (-not (Test-Path -LiteralPath $vswhere)) { throw 'Install CMake or Visual Studio 2022 C++ tools' }
        $installation = & $vswhere -latest -products '*' -property installationPath
        $cmake = Join-Path $installation 'Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe'
        if (-not (Test-Path -LiteralPath $cmake)) { throw 'Visual Studio CMake component is missing' }
    }
    $env:PATH = (Split-Path $cmake) + ';' + $env:PATH
    if (-not $SdkRoot -and $env:ONNXRUNTIME_ROOT) { $SdkRoot = $env:ONNXRUNTIME_ROOT }
    if (-not $OpenCvSource) { $OpenCvSource = if ($env:ONEOCR_OPENCV_SOURCE) { $env:ONEOCR_OPENCV_SOURCE } else { 'work/vendor/opencv' } }
    if (-not $PSBoundParameters.ContainsKey('IcuSource') -and $env:ONEOCR_ICU_SOURCE) { $IcuSource = $env:ONEOCR_ICU_SOURCE }
    $explicitIcuData = [bool]$IcuData -or [bool]$env:ONEOCR_ICU_DATA_C
    if (-not $IcuData) { $IcuData = if ($env:ONEOCR_ICU_DATA_C) { $env:ONEOCR_ICU_DATA_C } else { 'work/icu-generated/icudt74l_dat.c' } }
    if (-not $SourceRuntime -and -not $SdkRoot) { throw 'Provide -SdkRoot (or ONNXRUNTIME_ROOT) and prepared OpenCV/ICU paths; see docs/colleague-build.md. Use -SourceRuntime only to explicitly rebuild ORT.' }
    $python = Join-Path $ProjectRoot '.research-python/python.exe'
    if ($SourceRuntime) {
        if (-not (Test-Path -LiteralPath $python)) { throw 'Run scripts/bootstrap-research.ps1 first for the optional -SourceRuntime workflow' }
        if (-not (Test-Path -LiteralPath $OpenCvSource)) {
            if ($OpenCvSource -ne 'work/vendor/opencv') { throw 'The supplied OpenCV source directory does not exist' }
            & $python scripts/fetch-opencv.py
            if ($LASTEXITCODE -ne 0) { throw 'OpenCV source preparation failed' }
        }
        & $python scripts/prepare-icu.py --source $IcuSource --fetch-only
        if ($LASTEXITCODE -ne 0) { throw 'ICU source preparation failed' }
    }
    if ($SourceRuntime -and -not $explicitIcuData -and -not (Test-Path -LiteralPath $IcuData)) {
        if (-not (Get-Command wsl.exe -ErrorAction SilentlyContinue)) { throw 'ICU data needs a Linux/macOS build host. Run scripts/prepare-icu.py there and copy work/icu-generated back.' }
        $IcuSourcePath = (Resolve-Path -LiteralPath $IcuSource).Path
        $IcuWslSource = (& wsl.exe --exec wslpath -u $IcuSourcePath).Trim()
        if ($LASTEXITCODE -ne 0) { throw 'Cannot resolve ICU source path in WSL' }
        & wsl.exe --cd $ProjectRoot --exec python3 scripts/prepare-icu.py --source $IcuWslSource --jobs $Jobs
        if ($LASTEXITCODE -ne 0) { throw 'ICU source data generation failed; WSL needs a C/C++ compiler and make' }
    }
    if ($SourceRuntime) {
        if (-not (Test-Path -LiteralPath $python)) { throw 'Run scripts/bootstrap-research.ps1 first' }
        & $python scripts/fetch-runtime.py --source
        if ($LASTEXITCODE -ne 0) { throw 'ORT source preparation failed' }
        & $python scripts/build-ort.py --build_dir (Join-Path $ProjectRoot 'work/ort-source') --config Release --update --build --parallel $Jobs --build_shared_lib --skip_tests --compile_no_warning_as_error --cmake_generator 'Visual Studio 17 2022' --cmake_extra_defines onnxruntime_BUILD_UNIT_TESTS=OFF
        if ($LASTEXITCODE -ne 0) { throw 'ORT source build failed' }
        $SdkRoot = Join-Path $ProjectRoot 'work/ort-install'
        & $cmake --install work/ort-source/Release --config Release --prefix $SdkRoot
        if ($LASTEXITCODE -ne 0) { throw 'ORT source install failed' }
    }
    $SdkRoot = (Resolve-Path -LiteralPath $SdkRoot).Path
    $OpenCvSourcePath = (Resolve-Path -LiteralPath $OpenCvSource).Path
    $IcuSourcePath = (Resolve-Path -LiteralPath $IcuSource).Path
    $IcuDataPath = (Resolve-Path -LiteralPath $IcuData).Path
    & $cmake -S . -B $BuildDirectory -G 'Visual Studio 17 2022' -A x64 -U ONNXRUNTIME_INCLUDE_DIR -U ONNXRUNTIME_LIBRARY -U ORT_RUNTIME_DLL "-DONNXRUNTIME_ROOT=$SdkRoot" "-DONEOCR_OPENCV_SOURCE=$OpenCvSourcePath" "-DONEOCR_ICU_SOURCE=$IcuSourcePath" "-DONEOCR_ICU_DATA_C=$IcuDataPath"
    if ($LASTEXITCODE -ne 0) { throw 'Native configure failed' }
    & $cmake --build $BuildDirectory --config Release --parallel $Jobs
    if ($LASTEXITCODE -ne 0) { throw 'Native build failed' }
    & (Join-Path (Split-Path $cmake) 'ctest.exe') --test-dir $BuildDirectory -C Release --output-on-failure
    if ($LASTEXITCODE -ne 0) { throw 'Native tests failed' }
} finally { Pop-Location }
