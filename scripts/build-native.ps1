param(
    [string]$SdkRoot = '',
    [switch]$SourceRuntime,
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
    $python = Join-Path $ProjectRoot '.research-python/python.exe'
    if ($SourceRuntime) {
        if (-not (Test-Path -LiteralPath $python)) { throw 'Run scripts/bootstrap-research.ps1 first' }
        & $python scripts/fetch-runtime.py --source
        if ($LASTEXITCODE -ne 0) { throw 'ORT source preparation failed' }
        & $python scripts/build-ort.py --build_dir (Join-Path $ProjectRoot 'work/ort-source') --config Release --update --build --target onnxruntime --parallel $Jobs --build_shared_lib --skip_tests --compile_no_warning_as_error --cmake_generator 'Visual Studio 17 2022' --cmake_extra_defines onnxruntime_BUILD_UNIT_TESTS=OFF
        if ($LASTEXITCODE -ne 0) { throw 'ORT source build failed' }
        $SdkRoot = Join-Path $ProjectRoot 'work/ort-install'
        & $cmake --install work/ort-source/Release --config Release --prefix $SdkRoot
        if ($LASTEXITCODE -ne 0) { throw 'ORT source install failed' }
    }
    if (-not $SdkRoot) { throw 'Use -SourceRuntime, or explicitly supply -SdkRoot for an optional prebuilt SDK' }
    $SdkRoot = (Resolve-Path -LiteralPath $SdkRoot).Path
    & $cmake -S . -B build/source-runtime -G 'Visual Studio 17 2022' -A x64 -U ONNXRUNTIME_INCLUDE_DIR -U ONNXRUNTIME_LIBRARY -U ORT_RUNTIME_DLL "-DONNXRUNTIME_ROOT=$SdkRoot"
    if ($LASTEXITCODE -ne 0) { throw 'Native configure failed' }
    & $cmake --build build/source-runtime --config Release --parallel $Jobs
    if ($LASTEXITCODE -ne 0) { throw 'Native build failed' }
    & (Join-Path (Split-Path $cmake) 'ctest.exe') --test-dir build/source-runtime -C Release --output-on-failure
    if ($LASTEXITCODE -ne 0) { throw 'Native tests failed' }
} finally { Pop-Location }
