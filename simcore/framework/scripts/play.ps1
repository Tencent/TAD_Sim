param ($scene_path)

$DEFAULT_SCENE_PATH = "C:\\Users\\nemoforwang\\AppData\\Roaming\\tadsim\\scenario\\scene\\CityWay-E-ChangeLane-STRR-XX-MIIT.sim"
if ( [string]::IsNullOrEmpty($scene_path) ) {
    $scene_path = $DEFAULT_SCENE_PATH
}
if ( -not ( Test-Path -Path $scene_path -PathType Leaf ) ) {
    Write-Host "*** Error: invalid TAD Sim scenario file path: $scene_path"
    return
}
Write-Host "scenario file: $scene_path"

$service_path = "$PSScriptRoot\txsim-local-service.exe"
$app_root = (Get-ChildItem -Path Env:\TADSIM_DATA).Value
Write-Host "service path: $service_path"
Write-Host "app data dir: $app_root"

Write-Host "===> Start TAD Sim service ..."
$txsim_ps = Start-Process -FilePath $service_path -ArgumentList "--root `"$app_root`" --logdir `"$app_root\debug_log`"" -NoNewWindow -PassThru

Write-Host "===> Setup TAD Sim ..."
node $PSScriptRoot\play-client.js setup "$scene_path" | Out-Null
Write-Host "===> Run TAD Sim ..."
node $PSScriptRoot\play-client.js run | Out-Null
Write-Host "===> Un-setup TAD Sim ..."
node $PSScriptRoot\play-client.js un-setup | Out-Null

Write-Host "===> Run TAD Sim finished."
Stop-Process $txsim_ps
