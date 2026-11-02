param([string]$ProjectRoot = (Split-Path $PSScriptRoot -Parent))
$ErrorActionPreference = 'Stop'
$runtime = Join-Path $ProjectRoot '.research-python'
$python = Join-Path $runtime 'python.exe'
if (-not (Test-Path -LiteralPath $python)) {
    $archive = Join-Path $ProjectRoot 'work/python-3.12.8-embed-amd64.zip'
    New-Item -ItemType Directory -Force (Split-Path $archive) | Out-Null
    Invoke-WebRequest -UseBasicParsing 'https://www.python.org/ftp/python/3.12.8/python-3.12.8-embed-amd64.zip' -OutFile $archive
    Expand-Archive -LiteralPath $archive -DestinationPath $runtime
    @('python312.zip', '.', 'Lib/site-packages', 'import site') | Set-Content (Join-Path $runtime 'python312._pth') -Encoding ascii
}
if (-not (Test-Path -LiteralPath (Join-Path $runtime 'Lib/site-packages/pip'))) {
    $pipWheel = Join-Path $ProjectRoot 'work/pip-25.2-py3-none-any.whl'
    $metadata = Invoke-RestMethod 'https://pypi.org/pypi/pip/25.2/json'
    $asset = $metadata.urls | Where-Object { $_.filename -eq 'pip-25.2-py3-none-any.whl' }
    Invoke-WebRequest -UseBasicParsing $asset.url -OutFile $pipWheel
    if ((Get-FileHash -LiteralPath $pipWheel -Algorithm SHA256).Hash.ToLowerInvariant() -ne $asset.digests.sha256) { throw 'pip wheel SHA-256 mismatch' }
    New-Item -ItemType Directory -Force (Join-Path $runtime 'Lib/site-packages') | Out-Null
    & $python -c 'import sys,zipfile; zipfile.ZipFile(sys.argv[1]).extractall(sys.argv[2])' $pipWheel (Join-Path $runtime 'Lib/site-packages')
    if ($LASTEXITCODE -ne 0) { throw 'pip extraction failed' }
}
& $python -m pip install --disable-pip-version-check -r (Join-Path $PSScriptRoot 'requirements-lock.txt')
if ($LASTEXITCODE -ne 0) { throw 'Research dependency installation failed' }
Write-Output "Research Python: $python"
