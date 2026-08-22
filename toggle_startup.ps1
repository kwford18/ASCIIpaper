<#
.SYNOPSIS
Toggles ASCIIpaper on or off in the Windows Startup folder.
#>

$startupFolder = [Environment]::GetFolderPath('Startup')
$shortcutPath = Join-Path -Path $startupFolder -ChildPath "ASCIIpaper.lnk"

# Locate the executable (checks current folder, then the build folder)
$exeName = "ASCIIpaper.exe"
$exePath = Join-Path -Path $PSScriptRoot -ChildPath $exeName

if (-Not (Test-Path $exePath)) {
    $exePath = Join-Path -Path $PSScriptRoot -ChildPath "build\$exeName"
}

if (-Not (Test-Path $exePath)) {
    Write-Host "Error: Could not find $exeName." -ForegroundColor Red
    Write-Host "Please place this script next to the executable or in the root of your project folder."
    Write-Host ""
    Read-Host "Press Enter to exit"
    exit
}

if (Test-Path $shortcutPath) {
    # Shortcut exists -> Disable startup
    Remove-Item -Path $shortcutPath -Force
    Write-Host "[-] Disabled: ASCIIpaper removed from startup." -ForegroundColor Yellow
} else {
    # Shortcut does not exist -> Enable startup
    $WshShell = New-Object -comObject WScript.Shell
    $Shortcut = $WshShell.CreateShortcut($shortcutPath)
    $Shortcut.TargetPath = $exePath
    
    # Explicitly set the Working Directory so config.ini is found
    $Shortcut.WorkingDirectory = Split-Path -Path $exePath -Parent
    $Shortcut.Description = "ASCIIpaper Background Engine"
    $Shortcut.Save()
    
    Write-Host "[+] Enabled: ASCIIpaper added to startup!" -ForegroundColor Green
}

Write-Host ""
Read-Host "Press Enter to close"