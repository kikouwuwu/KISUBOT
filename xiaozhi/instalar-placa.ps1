# Copia la placa kisubot-cabeza al clon de XiaoZhi y la registra en su
# Kconfig y CMakeLists. Se puede correr varias veces sin duplicar nada.
#
# Uso (desde la raíz del repo KISUBOT):
#   powershell -ExecutionPolicy Bypass -File xiaozhi\instalar-placa.ps1
#   powershell -ExecutionPolicy Bypass -File xiaozhi\instalar-placa.ps1 -XiaoZhi C:\ruta\al\clon

param(
    [string]$XiaoZhi = "$env:USERPROFILE\esp\xiaozhi-esp32"
)

$ErrorActionPreference = "Stop"
$origen = Join-Path $PSScriptRoot "kisubot-cabeza"
$main = Join-Path $XiaoZhi "main"
if (-not (Test-Path (Join-Path $main "Kconfig.projbuild"))) {
    throw "No encuentro el clon de XiaoZhi en $XiaoZhi"
}

# 1. Copiar los archivos de la placa
$destino = Join-Path $main "boards\kisubot-cabeza"
New-Item -ItemType Directory -Force $destino | Out-Null
Copy-Item "$origen\*" $destino -Force
Write-Host "Archivos copiados a $destino"

$utf8 = New-Object System.Text.UTF8Encoding($false)

# Respeta los saltos de línea del archivo (CRLF en Windows, LF en Linux/Mac)
function Get-SaltoDeLinea([string]$texto) {
    if ($texto.Contains("`r`n")) { return "`r`n" } else { return "`n" }
}

# 2. Registrar la placa en Kconfig.projbuild (lista "Board Type")
$kconfig = Join-Path $main "Kconfig.projbuild"
$texto = [IO.File]::ReadAllText($kconfig)
if ($texto -notmatch "BOARD_TYPE_KISUBOT_CABEZA") {
    $nl = Get-SaltoDeLinea $texto
    $ancla = "    config BOARD_TYPE_BREAD_COMPACT_WIFI_CAM$nl"
    if (-not $texto.Contains($ancla)) { throw "No encontré dónde agregar la placa en Kconfig.projbuild" }
    $nuevo = "    config BOARD_TYPE_KISUBOT_CABEZA$nl" +
             "        bool `"KISUBOT cabeza (ESP32-S3 + OV5640)`"$nl" +
             "        depends on IDF_TARGET_ESP32S3$nl"
    $texto = $texto.Replace($ancla, $nuevo + $ancla)
    [IO.File]::WriteAllText($kconfig, $texto, $utf8)
    Write-Host "Placa agregada a Kconfig.projbuild"
} else {
    Write-Host "Kconfig.projbuild ya tenía la placa"
}

# 3. Registrar la carpeta en main/CMakeLists.txt
$cmake = Join-Path $main "CMakeLists.txt"
$texto = [IO.File]::ReadAllText($cmake)
if ($texto -notmatch "CONFIG_BOARD_TYPE_KISUBOT_CABEZA") {
    $nl = Get-SaltoDeLinea $texto
    $ancla = "elseif(CONFIG_BOARD_TYPE_BREAD_COMPACT_WIFI_CAM)$nl"
    if (-not $texto.Contains($ancla)) { throw "No encontré dónde agregar la placa en main/CMakeLists.txt" }
    $nuevo = "elseif(CONFIG_BOARD_TYPE_KISUBOT_CABEZA)$nl" +
             "    set(BOARD_DIR `"kisubot-cabeza`")$nl" +
             "    set(BUILTIN_TEXT_FONT font_noto_sans_basic_16_4)$nl" +
             "    set(BUILTIN_ICON_FONT font_material_symbols_16_4)$nl" +
             "    set(DEFAULT_EMOJI_COLLECTION noto-color-emoji_64)$nl"
    $texto = $texto.Replace($ancla, $nuevo + $ancla)
    [IO.File]::WriteAllText($cmake, $texto, $utf8)
    Write-Host "Placa agregada a main/CMakeLists.txt"
} else {
    Write-Host "main/CMakeLists.txt ya tenía la placa"
}

Write-Host "Listo. Para compilar, en una terminal de ESP-IDF:"
Write-Host "  cd $XiaoZhi"
Write-Host "  python scripts/build.py kisubot-cabeza --language es-ES"
