param([Parameter(Mandatory=$true)][int]$ControllerProcessId)
$ErrorActionPreference = 'Stop'
$inventory = @(Get-CimInstance Win32_Process)
$tree = [System.Collections.Generic.HashSet[int]]::new()
[void]$tree.Add($ControllerProcessId)
do {
    $changed = $false
    foreach ($entry in $inventory) {
        if ($tree.Contains([int]$entry.ParentProcessId) -and $tree.Add([int]$entry.ProcessId)) { $changed = $true }
    }
} while ($changed)
$measurements = @(
    foreach ($entry in $inventory) {
        if ($tree.Contains([int]$entry.ProcessId) -and $entry.Name -in @('chrome.exe','msedge.exe','chromium.exe')) {
            $browserProcess = Get-Process -Id $entry.ProcessId -ErrorAction SilentlyContinue
            if ($browserProcess) {
                try {
                    [pscustomobject]@{ processId=[int]$entry.ProcessId; name=$entry.Name;
                        peakWorkingSetBytes=$browserProcess.PeakWorkingSet64; workingSetBytes=$browserProcess.WorkingSet64 }
                } catch [System.InvalidOperationException] { continue }
            }
        }
    }
)
if ($measurements.Count -eq 0) { throw 'No live browser process found under the regression controller' }
[pscustomobject]@{
    method='Windows OS PeakWorkingSet64 for live browser processes, queried before browser close'
    scope='One regression worker browser tree; process peaks need not occur simultaneously'
    maxProcessPeakBytes=($measurements | Measure-Object -Property peakWorkingSetBytes -Maximum).Maximum
    sumOfProcessPeaksUpperBoundBytes=($measurements | Measure-Object -Property peakWorkingSetBytes -Sum).Sum
    processes=$measurements
} | ConvertTo-Json -Depth 5
