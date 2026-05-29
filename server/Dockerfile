# =====================================================================
# Servidor SOAP — simula un servicio externo (ej: un banco)
# =====================================================================
# Imagen base: Debian estable con gSOAP precompilado
FROM debian:bookworm-slim AS builder

# Instalar gSOAP + headers de desarrollo + compilador
RUN apt-get update && apt-get install -y --no-install-recommends \
    gsoap \
    libgsoap-dev \
    g++ \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app

# Copiar el contrato y el código del servidor
COPY calc.h ./
COPY server.cpp ./

# PASO 1: Generar el código de red con soapcpp2 (flag -S = solo servidor)
RUN soapcpp2 -j -S calc.h

# PASO 2: Compilar el binario del servidor
RUN g++ -O2 -o server \
    server.cpp \
    soapCalculadoraService.cpp \
    soapC.cpp \
    -lgsoap++

# =====================================================================
# Imagen final: solo el binario y el runtime de gSOAP (sin compilador)
# =====================================================================
FROM debian:bookworm-slim

# Instalamos solo lo necesario para ejecutar.
# Usamos libgsoap-dev porque su nombre es estable entre versiones de Debian;
# arrastra el runtime que necesitamos sin tener que adivinar el sufijo de versión.
RUN apt-get update && apt-get install -y --no-install-recommends \
    libgsoap-dev \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app
COPY --from=builder /app/server /app/server
# IMPORTANTE: copiamos el WSDL generado para que el cliente lo pueda usar
COPY --from=builder /app/Calculadora.wsdl /app/Calculadora.wsdl

EXPOSE 8081

CMD ["/app/server"]
