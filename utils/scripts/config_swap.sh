#!/bin/bash
read -p "Ingrese IP Memoria: " ipMemoria
read -p "Ingrese IP Kernel: " ipKernel
read -p "Ingrese Path de Pseudocodigos: " pathPseudocodigos
read -p "Ingrese Path de Swapfile: " pathSwapfile
read -p "Ingrese Path de Dump Memory: " pathDumpMemory

cd ../

#KERNEL
cd kernel
CONFIG_FILE="kernel.config"

declare -A valores=(
  ["ALGORITMO_CORTO_PLAZO"]="FIFO"
  ["ALGORITMO_INGRESO_A_READY"]="FIFO"
  ["ALFA"]="1"
  ["ESTIMACION_INICIAL"]="10000"
  ["TIEMPO_SUSPENSION"]="1000"
)

if [ -n "$ipMemoria" ]; then
  valores["IP_MEMORIA"]="$ipMemoria"
fi

for var in "${!valores[@]}"; do
  if grep -q "^$var=" "$CONFIG_FILE"; then
    # Reemplazar línea existente
    sed -i "s/^$var=.*/$var=${valores[$var]}/" "$CONFIG_FILE"
  else
    # Agregar al final si no existe
    echo "$var=${valores[$var]}" >> "$CONFIG_FILE"
  fi
done

cd ../

#CPU1
cd cpu
CONFIG_FILE="cpu1.config"

declare -A valores=(
  ["ENTRADAS_TLB"]="0"
  ["REEMPLAZO_TLB"]="FIFO"
  ["ENTRADAS_CACHE"]="0"
  ["REEMPLAZO_CACHE"]="CLOCK"
  ["RETARDO_CACHE"]="250"
)

if [ -n "$ipMemoria" ]; then
  valores["IP_MEMORIA"]="$ipMemoria"
fi

if [ -n "$ipKernel" ]; then
  valores["IP_KERNEL"]="$ipKernel"
fi

for var in "${!valores[@]}"; do
  if grep -q "^$var=" "$CONFIG_FILE"; then
    sed -i "s/^$var=.*/$var=${valores[$var]}/" "$CONFIG_FILE"
  else
    echo "$var=${valores[$var]}" >> "$CONFIG_FILE"
  fi
done

cd ../

#MEMORIA
cd memoria
CONFIG_FILE="memoria.config"

declare -A valores=(
  ["TAM_MEMORIA"]="512"
  ["TAM_PAGINA"]="32"
  ["ENTRADAS_POR_TABLA"]="32"
  ["CANTIDAD_NIVELES"]="31"
  ["RETARDO_MEMORIA"]="500"
  ["RETARDO_SWAP"]="2500"
)

if [ -n "$pathPseudocodigos" ]; then
  valores["PATH_PSEUDOCODIGOS"]="$pathPseudocodigos"
fi

if [ -n "$pathDumpMemory" ]; then
  valores["DUMP_PATH"]="$pathDumpMemory"
fi

if [ -n "$pathSwapfile" ]; then
  valores["PATH_SWAPFILE"]="$pathSwapfile"
fi

for var in "${!valores[@]}"; do
  if grep -q "^$var=" "$CONFIG_FILE"; then
    sed -i "s/^$var=.*/$var=${valores[$var]}/" "$CONFIG_FILE"
  else
    echo "$var=${valores[$var]}" >> "$CONFIG_FILE"
  fi
done

cd ../

#IO
cd io
CONFIG_FILE="io.config"

declare -A valores=()

if [ -n "$ipKernel" ]; then
  valores["IP_KERNEL"]="$ipKernel"
fi

for var in "${!valores[@]}"; do
  if grep -q "^$var=" "$CONFIG_FILE"; then
    sed -i "s/^$var=.*/$var=${valores[$var]}/" "$CONFIG_FILE"
  else
    echo "$var=${valores[$var]}" >> "$CONFIG_FILE"
  fi
done

cd ../

echo "Configuración actualizada para ejecutar MEMORIA_SWAP"
