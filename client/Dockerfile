# =====================================================================
# Cliente SOAP — CASO 3: consume un WSDL externo
# =====================================================================
# Aquí el flujo es:
#   WSDL externo  →  wsdl2h  →  .h  →  soapcpp2 -C  →  código cliente
# =====================================================================
FROM debian:bookworm-slim AS builder

RUN apt-get update && apt-get install -y --no-install-recommends \
    gsoap \
    libgsoap-dev \
    g++ \
    ca-certificates \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app

# Copiamos el WSDL "externo" (simula uno entregado por un banco)
COPY Calculadora.wsdl ./
COPY client.cpp ./

# PASO 1 (clave del CASO 3): convertir el WSDL en header C++
RUN wsdl2h -o calc_gen.h Calculadora.wsdl

# PASO 2: generar el código del cliente desde el header (flag -C = solo cliente)
RUN soapcpp2 -j -C calc_gen.h

# PASO 3: compilar el binario
RUN g++ -O2 -o client \
    client.cpp \
    soapCalculadoraProxy.cpp \
    soapC.cpp \
    -lgsoap++

# =====================================================================
# Imagen final
# =====================================================================
FROM debian:bookworm-slim

RUN apt-get update && apt-get install -y --no-install-recommends \
    libgsoap-dev \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app
COPY --from=builder /app/client /app/client

# Por defecto el cliente queda esperando (tail -f) para que los alumnos
# entren al contenedor y ejecuten ./client manualmente.
CMD ["tail", "-f", "/dev/null"]
