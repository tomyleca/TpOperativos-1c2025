#!/bin/bash

read -p "Ingrese IP Memoria: " ipMemoria
read -p "Ingrese IP Kernel: " ipKernel
read -p "Ingrese Path de Pseudocodigos" pathPseudocodigos

cd ../

#KERNEL
cd kernel
CONFIG_FILE="kernel.config"

declare -A valores=(
  ["ALGORITMO_CORTO_PLAZO"]="FIFO"
  ["ALGORITMO_INGRESO_A_READY"]="FIFO"
  ["ALFA"]="1"
  ["ESTIMACION_INICIAL"]="10000"
  ["TIEMPO_SUSPENSION"]="3000"
)

[ -n "$ipMemoria" ] && valores["IP_MEMORIA"]="$ipMemoria"

for var in "${!valores[@]}"; do
  if grep -q "^$var=" "$CONFIG_FILE"; then
    sed -i "s/^$var=.*/$var=${valores[$var]}/" "$CONFIG_FILE"
  else
    echo "$var=${valores[$var]}" >> "$CONFIG_FILE"
  fi
done

cd ../

#CPU1
cd cpu
CONFIG_FILE="cpu1.config"

declare -A valores=(
  ["ENTRADAS_TLB"]="4"
  ["REEMPLAZO_TLB"]="FIFO"
  ["ENTRADAS_CACHE"]="2"
  ["REEMPLAZO_CACHE"]="CLOCK-M"
  ["RETARDO_CACHE"]="250"
)

[ -n "$ipMemoria" ] && valores["IP_MEMORIA"]="$ipMemoria"
[ -n "$ipKernel" ] && valores["IP_KERNEL"]="$ipKernel"

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
  ["TAM_MEMORIA"]="2048"
  ["TAM_PAGINA"]="32"
  ["ENTRADAS_POR_TABLA"]="4"
  ["CANTIDAD_NIVELES"]="3"
  ["RETARDO_MEMORIA"]="500"
  ["RETARDO_SWAP"]="5000"
)

[ -n "$pathPseudocodigos" ] && valores["PATH_PSEUDOCODIGOS"]="$pathPseudocodigos"

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

[ -n "$ipKernel" ] && valores["IP_KERNEL"]="$ipKernel"

for var in "${!valores[@]}"; do
  if grep -q "^$var=" "$CONFIG_FILE"; then
    sed -i "s/^$var=.*/$var=${valores[$var]}/" "$CONFIG_FILE"
  else
    echo "$var=${valores[$var]}" >> "$CONFIG_FILE"
  fi
done

cd ../

echo "Configuración actualizada para ejecutar MEMORIA_CACHE"
