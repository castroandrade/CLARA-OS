# build-run.ps1
# Script de automação para compilar com Docker e rodar com QEMU no Windows

# 1. Detectar o executável do QEMU
$qemuCmd = "qemu-system-i386"
$qemuPathFound = $false

# Verifica se está no PATH do sistema
if (Get-Command $qemuCmd -ErrorAction SilentlyContinue) {
    $qemuPathFound = $true
} else {
    # Caminhos comuns de instalação no Windows (incluindo MSYS2 UCRT64)
    $commonPaths = @(
        "C:\msys64\ucrt64\bin\qemu-system-i386.exe",
        "C:\Program Files\qemu\qemu-system-i386.exe",
        "C:\msys64\mingw64\bin\qemu-system-i386.exe"
    )
    foreach ($path in $commonPaths) {
        if (Test-Path $path) {
            $qemuCmd = $path
            $qemuPathFound = $true
            break
        }
    }
}

Write-Host "=== 1. Garantindo a Imagem Docker (clara-os-builder) ===" -ForegroundColor Cyan
docker build -t clara-os-builder -f Dockerfile .

if ($LASTEXITCODE -eq 0) {
    Write-Host "`n=== 2. Compilando o CLARA OS via Docker ===" -ForegroundColor Cyan
    docker run --rm -v "${PWD}:/usr/src/clara-os" -w /usr/src/clara-os clara-os-builder make clean all
    
    if ($LASTEXITCODE -eq 0) {
        if (-not $qemuPathFound) {
            Write-Host "`n[ERRO] O QEMU não foi localizado no PATH ou nas pastas comuns de instalação." -ForegroundColor Red
            Write-Host "Por favor, adicione o caminho do QEMU (ex: C:\msys64\ucrt64\bin) às Variáveis de Ambiente (PATH) ou instale-o." -ForegroundColor Yellow
        } else {
            Write-Host "`n=== 3. Iniciando Emulação no QEMU ===" -ForegroundColor Green
            Write-Host "Executando: $qemuCmd" -ForegroundColor Gray
            & $qemuCmd -kernel clara_os.bin
        }
    } else {
        Write-Host "`n[ERRO] A compilação do Make falhou. Verifique as mensagens acima." -ForegroundColor Red
    }
} else {
    Write-Host "`n[ERRO] A criação da imagem Docker falhou." -ForegroundColor Red
}
