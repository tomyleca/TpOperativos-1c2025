#!/bin/bash
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
  ["ENTRADAS_TLB"]="4"
  ["REEMPLAZO_TLB"]="LRU"
  ["ENTRADAS_CACHE"]="0"
  ["REEMPLAZO_CACHE"]="CLOCK"
  ["RETARDO_CACHE"]="250"
)

for var in "${!valores[@]}"; do
  if grep -q "^$var=" "$CONFIG_FILE"; then
    sed -i "s/^$var=.*/$var=${valores[$var]}/" "$CONFIG_FILE"
  else
    echo "$var=${valores[$var]}" >> "$CONFIG_FILE"
  fi
done


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

for var in "${!valores[@]}"; do
  if grep -q "^$var=" "$CONFIG_FILE"; then
    sed -i "s/^$var=.*/$var=${valores[$var]}/" "$CONFIG_FILE"
  else
    echo "$var=${valores[$var]}" >> "$CONFIG_FILE"
  fi
done

cd ../

echo "Configuración actualizada para ejecutar MEMORIA_TLB"

