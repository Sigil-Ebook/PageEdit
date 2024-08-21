Param
(
    [parameter(Mandatory = $true)]
    [string] $Version
)

function Write-MetaData {
    param (
        [parameter(Mandatory = $true)]
        [string]
        $FileName,
        [parameter(Mandatory = $true)]
        [string]
        $Version,
        [parameter(Mandatory = $true)]
        [string]
        $HashAmd64
    )
    $content = Get-Content $FileName -Raw
    $content = $content.Replace('<VERSION>', $Version)
    $content = $content.Replace('<HASH-AMD64>', $HashAmd64)
    $date = Get-Date -Format "yyyy-MM-dd"
    $content = $content.Replace('<DATE>', $date)
    $content | Out-File -Encoding 'UTF8' "./$Version/$FileName"
}

New-Item -Path $PWD -Name $Version -ItemType "directory"

# Get hash for installers
Invoke-WebRequest "https://github.com/Sigil-Ebook/PageEdit/releases/download/$Version/PageEdit-$Version-Windows-x64-Setup.exe" -OutFile 'INSTALLER-AMD64.exe'
$HashAmd64 = (Get-FileHash -Path 'INSTALLER-AMD64.exe' -Algorithm SHA256).Hash

# Update the metadata for the files
Get-ChildItem '*.yaml' | ForEach-Object -Process {
    Write-MetaData -FileName $_.Name -Version $Version -HashAmd64 $HashAmd64
}
if (-not $env:WINGET_TOKEN) {
    Write-Error "You must set the WINGET_TOKEN environment variable to create a pull request."
    Exit 1
}
# Get the latest wingetcreate exe
Invoke-WebRequest 'https://aka.ms/wingetcreate/preview' -OutFile wingetcreate.exe

$prtitle = "New Version: Sigil-Ebook.PageEdit version $Version"

# Create the PR
./wingetcreate.exe submit --prtitle $prtitle --token $env:WINGET_TOKEN $Version
