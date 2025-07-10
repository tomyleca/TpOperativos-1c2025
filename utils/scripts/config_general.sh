#!/bin/bash


read -p "Ingrese IP Memoria: " ipMemoria
read -p "Ingrese IP Kernel: " ipKernel
read -p "Ingrese Path de Pseudocodigos: " pathPseudocodigos
read -p "Ingrese Path de Swapfile: " pathSwapfile
read -p "Ingrese Path de Dump Memory: " pathDumpMemory

cd ../

# KERNEL
cd kernel
CONFIG_FILE="kernel.config"

declare -A valores=(
  ["ALGORITMO_CORTO_PLAZO"]="SRT"
  ["ALGORITMO_INGRESO_A_READY"]="PMCP"
  ["ALFA"]="0.75"
  ["ESTIMACION_INICIAL"]="100"
  ["TIEMPO_SUSPENSION"]="3000"
)


if [ -n "$ipMemoria" ]; then
  valores["IP_MEMORIA"]="$ipMemoria"
fi

for var in "${!valores[@]}"; do
  if grep -q "^$var=" "$CONFIG_FILE"; then
    sed -i "s|^$var=.*|$var=${valores[$var]}|" "$CONFIG_FILE"
  else
    echo "$var=${valores[$var]}" >> "$CONFIG_FILE"
  fi
done

cd ../

#CPU
cd cpu

for config in cpu1.config cpu2.config cpu3.config cpu4.config; do
  declare -A valores=()

  case $config in
    "cpu1.config")
      valores=(
        ["ENTRADAS_TLB"]="4"
        ["REEMPLAZO_TLB"]="FIFO"
        ["ENTRADAS_CACHE"]="2"
        ["REEMPLAZO_CACHE"]="CLOCK-M"
        ["RETARDO_CACHE"]="50"
      )
      ;;
    "cpu2.config")
      valores=(
        ["ENTRADAS_TLB"]="4"
        ["REEMPLAZO_TLB"]="LRU"
        ["ENTRADAS_CACHE"]="2"
        ["REEMPLAZO_CACHE"]="CLOCK-M"
        ["RETARDO_CACHE"]="50"
      )
      ;;
    "cpu3.config")
      valores=(
        ["ENTRADAS_TLB"]="256"
        ["REEMPLAZO_TLB"]="FIFO"
        ["ENTRADAS_CACHE"]="256"
        ["REEMPLAZO_CACHE"]="CLOCK"
        ["RETARDO_CACHE"]="1"
      )
      ;;
    "cpu4.config")
      valores=(
        ["ENTRADAS_TLB"]="0"
        ["REEMPLAZO_TLB"]="FIFO"
        ["ENTRADAS_CACHE"]="0"
        ["REEMPLAZO_CACHE"]="CLOCK"
        ["RETARDO_CACHE"]="0"
      )
      ;;
  esac

 
  [ -n "$ipMemoria" ] && valores["IP_MEMORIA"]="$ipMemoria"
  [ -n "$ipKernel" ] && valores["IP_KERNEL"]="$ipKernel"

  for var in "${!valores[@]}"; do
    if grep -q "^$var=" "$config"; then
      sed -i "s|^$var=.*|$var=${valores[$var]}|" "$config"
    else
      echo "$var=${valores[$var]}" >> "$config"
    fi
  done
done

cd ../

#MEMORIA
cd memoria
CONFIG_FILE="memoria.config"

declare -A valores=(
  ["TAM_MEMORIA"]="4096"
  ["TAM_PAGINA"]="32"
  ["ENTRADAS_POR_TABLA"]="8"
  ["CANTIDAD_NIVELES"]="3"
  ["RETARDO_MEMORIA"]="100"
  ["RETARDO_SWAP"]="2500"
)

[ -n "$pathPseudocodigos" ] && valores["PATH_PSEUDOCODIGOS"]="$pathPseudocodigos"
[ -n "$pathDumpMemory" ] && valores["DUMP_PATH"]="$pathDumpMemory"
[ -n "$pathSwapfile" ] && valores["PATH_SWAPFILE"]="$pathSwapfile"

for var in "${!valores[@]}"; do
  if grep -q "^$var=" "$CONFIG_FILE"; then
    sed -i "s|^$var=.*|$var=${valores[$var]}|" "$CONFIG_FILE"
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
    sed -i "s|^$var=.*|$var=${valores[$var]}|" "$CONFIG_FILE"
  else
    echo "$var=${valores[$var]}" >> "$CONFIG_FILE"
  fi
done

cd ../

echo "Configuración actualizada para ejecutar ESTABILIDAD_GENERAL."
