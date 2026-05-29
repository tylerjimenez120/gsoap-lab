# gSOAP — Caso 3: Consumir un servicio externo vía WSDL

Escenario realista en **un sistema transaccional crítico**: un proveedor externo (banco, pasarela de pago, sistema legacy) te entrega un archivo `.wsdl` con el contrato de su servicio. Tu trabajo es construir un cliente C++ que lo consuma.

Este documento cubre **únicamente** ese flujo. Asume que el servidor ya existe (no lo construyes tú, vive en otro equipo o empresa).

---

## Vista general del flujo

```
banco.wsdl (te lo da el banco)
        │
        │  wsdl2h
        ▼
banco.h (generado, no editas)
        │
        │  soapcpp2 -j -C
        ▼
proxy + serializadores
        │
        │  + tu client.cpp
        ▼
./client → conecta al servicio del banco
```

**Resumen en una línea:** WSDL externo → `wsdl2h` → `.h` → `soapcpp2 -C` → compilas tu cliente.

---

## Lado del servidor

**No haces nada.** El servidor lo provee el sistema externo. Asume que:

- Está corriendo y accesible vía HTTP/HTTPS en un endpoint conocido (ej: `https://api.banco.com/servicios`).
- Te entrega el archivo `.wsdl` que describe el contrato.

Lo único que necesitas del lado del servidor es **información**, no código:

| Qué necesitas | Para qué |
|---|---|
| El archivo `.wsdl` | Es el contrato. Define qué funciones existen, qué reciben, qué devuelven. |
| La URL del endpoint | A dónde se conecta tu cliente. |
| Credenciales (si aplica) | Para autenticarte (usuario/contraseña, token, certificado). |

> En este lab usamos como "servicio externo" el `./server` del caso 2 (la calculadora). Eso nos permite probar todo localmente sin depender de un servicio real, pero el flujo es idéntico al que harías con un banco.

---

## Lado del cliente — paso a paso

### Paso 1: Crear el proyecto

```bash
cd ~/roadmaps/c++/new_libs/GsoapLab
mkdir clientGsoap-fromWSDL
cd clientGsoap-fromWSDL
```

### Paso 2: Obtener el WSDL externo

Lo copiamos desde el servidor del lab anterior (simula que un banco te lo entregó):

```bash
cp ../serverGsoap/Calculadora.wsdl .
ls
```

> En un caso real este archivo lo recibirías por correo, lo descargarías de la documentación del proveedor, o lo expondría el propio servicio en una URL pública.

### Paso 3: Convertir WSDL a header (`wsdl2h`)

Esta es **la única herramienta nueva** que aparece en el caso 3.

```bash
wsdl2h -o calc_gen.h Calculadora.wsdl
ls
```

- `-o calc_gen.h`: nombre del header de salida.
- `Calculadora.wsdl`: el contrato externo.

**Qué genera:**
- `calc_gen.h` — archivo grande (300+ líneas) con las clases del contrato traducidas a C++. **No lo editas.**
- `typemap.dat` — referencia para personalizar mapeos. Lo dejas como está.

> Compara mentalmente con el caso 2: allí escribías el `.h` a mano. Aquí el `.h` es **generado**. Esa es la única diferencia conceptual entre los dos escenarios.

### Paso 4: Inspeccionar el header generado (opcional, recomendado)

```bash
grep "^class" calc_gen.h
grep "^int " calc_gen.h
```

Verás algo como:
```cpp
class _ns1__convertir { public: double monto; double tasa; ... };
class _ns1__convertirResponse { public: double* resultado; ... };
int __ns1__convertir(_ns1__convertir* in, _ns1__convertirResponse& out);
```

Observa las diferencias respecto a un `.h` escrito a mano:

| Caso 2 (.h escrito a mano) | Caso 3 (.h generado de WSDL) |
|---|---|
| `int ns__convertir(double, double, double*)` | Clases `_ns1__convertir` + `_ns1__convertirResponse` |
| Prefijo `ns__` | Prefijo `ns1__` (numerado) |
| Parámetros sueltos | Mensajes encapsulados en clases |
| 1 línea | 300+ líneas |

Misma semántica, distinta forma. El WSDL impone una representación más verbose pero más rica.

### Paso 5: Generar el código del cliente (`soapcpp2`)

```bash
soapcpp2 -j -C calc_gen.h
ls
```

- `-j` — clases C++ modernas (proxy).
- `-C` — solo Cliente.
- `calc_gen.h` — el header generado por `wsdl2h`.

**Archivos generados:**

| Archivo | Uso |
|---|---|
| `soapCalculadoraProxy.h` | Lo incluyes en `client.cpp`. Contiene la clase `CalculadoraProxy`. |
| `soapCalculadoraProxy.cpp` | Implementación de red (compilar, no editar). |
| `soapC.cpp` | Serializadores XML ↔ C++ (compilar, no editar). |
| `soapH.h`, `soapStub.h` | Tipos internos (no tocas). |
| `Calculadora.nsmap` | Tabla de namespaces (se incluye una vez). |

### Paso 6: Verificar el nombre de la clase proxy y método

Como los nombres dependen del WSDL, conviene confirmar antes de escribir el cliente:

```bash
grep -A2 "class.*Proxy" soapCalculadoraProxy.h | head -10
grep "convertir" soapCalculadoraProxy.h | head -5
```

Esperado:
```cpp
class SOAP_CMAC CalculadoraProxy { ... };
virtual int convertir(_ns1__convertir *ns1__convertir,
                      _ns1__convertirResponse &ns1__convertirResponse);
```

Confirmamos: clase `CalculadoraProxy`, método `convertir(_ns1__convertir*, _ns1__convertirResponse&)`.

### Paso 7: Escribir `client.cpp`

```cpp
#include "soapCalculadoraProxy.h"
#include "Calculadora.nsmap"
#include <iostream>

int main() {
    CalculadoraProxy proxy;
    proxy.soap_endpoint = "http://localhost:8081";   // URL del servicio externo

    // 1) Armar el mensaje de entrada
    _ns1__convertir request;
    request.monto = 100.0;
    request.tasa  = 3.85;

    // 2) Preparar el mensaje de salida (se llena al recibir)
    _ns1__convertirResponse response;

    // 3) Llamada remota
    int rc = proxy.convertir(&request, response);

    // 4) Procesar respuesta
    if (rc == SOAP_OK) {
        if (response.resultado != nullptr) {
            std::cout << "[CLIENT-fromWSDL] "
                      << request.monto << " x " << request.tasa
                      << " = " << *response.resultado << std::endl;
        } else {
            std::cout << "[CLIENT-fromWSDL] respuesta vacía\n";
        }
    } else {
        proxy.soap_stream_fault(std::cerr);
    }
    return 0;
}
```

**Lo que hace tu código (4 pasos):**

1. Crea el proxy y le dice a qué endpoint apuntar.
2. Llena un objeto de entrada con los datos a enviar.
3. Llama al método remoto.
4. Lee la respuesta y maneja errores.

**El cliente NO implementa lógica de negocio.** Solo arma datos, llama, lee resultado. Todo lo demás (serializar a XML, abrir socket, mandar HTTP, parsear respuesta) lo hace el proxy generado.

### Paso 8: Compilar

```bash
g++ -o client client.cpp soapCalculadoraProxy.cpp soapC.cpp -lgsoap++
```

- `client.cpp` — tu código.
- `soapCalculadoraProxy.cpp` + `soapC.cpp` — los generados por `soapcpp2`.
- `-lgsoap++` — librería runtime de gSOAP.

### Paso 9: Probar

Con el servidor externo corriendo (en nuestro lab, el `./server` del caso 2):

```bash
./client
```

Esperado:
```
[CLIENT-fromWSDL] 100 x 3.85 = 385
```

En el servidor verás la petición llegar:
```
[SERVER] convertir(100, 3.85) = 385
```

---

## Resumen del flujo del cliente

```
TÚ ESCRIBES:
  client.cpp  →  #include "soapCalculadoraProxy.h"
                 arma request, llama proxy.convertir(), lee response

EJECUTAS:
  wsdl2h  -o calc_gen.h  Calculadora.wsdl
  soapcpp2 -j -C calc_gen.h
  g++ -o client client.cpp soapCalculadoraProxy.cpp soapC.cpp -lgsoap++

CORRES:
  ./client  →  conecta al endpoint, manda XML, recibe XML, imprime resultado
```

---

## Decisiones de diseño que verás en el código generado

Algunas cosas curiosas que aparecen en el `.h` generado por `wsdl2h` y vale la pena entender:

### Prefijo `ns1__` en vez de `ns__`

`wsdl2h` enumera los namespaces (`ns1`, `ns2`, ...) cuando convierte el WSDL. Lo puedes personalizar editando `typemap.dat`, pero por defecto numera. Es solo cosmético, no afecta funcionamiento.

### Mensajes envueltos en clases

En vez de `int convertir(double, double, double*)`, ves `int convertir(_ns1__convertir*, _ns1__convertirResponse&)`. El WSDL fuerza esta forma porque los servicios SOAP serios siempre encapsulan request/response en objetos (escala mejor cuando crecen los campos).

### `response.resultado` es `double*` (puntero), no `double`

En el WSDL, los campos pueden ser opcionales. Por eso `wsdl2h` los genera como punteros: `nullptr` = "el servidor no devolvió este campo". Por eso el `if (response.resultado != nullptr)` en el cliente.

### Métodos `virtual` en el proxy

El proxy tiene `convertir` marcado como `virtual` aunque tú no vayas a heredarlo. Es por extensibilidad: si algún día quieres envolver las llamadas con logging, métricas o reintentos, puedes heredar y sobrescribir. Para uso normal lo ignoras y llamas directo.

### Implementación inline en el `.h`

La versión simple de `convertir` está definida dentro de la clase en el `.h`. Eso la hace `inline`: el compilador puede insertar el código directo donde la llamas, sin penalty de llamada. gSOAP lo hace así porque es solo una línea de delegación a la versión real (que sí vive en el `.cpp`).

---

## Comandos clave (referencia rápida)

```bash
# 1. WSDL externo → header
wsdl2h -o banco.h banco.wsdl

# 2. Header → código de cliente
soapcpp2 -j -C banco.h

# 3. Compilar cliente
g++ -o cliente cliente.cpp soapBancoProxy.cpp soapC.cpp -lgsoap++

# 4. Ejecutar
./cliente
```

---

## Errores comunes

| Error | Causa | Solución |
|---|---|---|
| `wsdl2h: command not found` | Falta el paquete | `sudo apt install -y gsoap` |
| `stdsoap2.h: No such file` | Faltan los headers de desarrollo | `sudo apt install -y libgsoap-dev` |
| `'XxxProxy' was not declared` | Nombre incorrecto de la clase | Confirmar con `grep "class.*Proxy"` en el `.h` generado |
| `Could not connect (SP_error -1)` | El endpoint no está accesible | Verificar URL y que el servicio externo esté corriendo |
| Respuesta `nullptr` en campos | Campos opcionales en el WSDL | Validar siempre `if (response.campo != nullptr)` |

---

## Cuándo aplicar este caso en un sistema transaccional crítico

Cualquier integración con sistemas externos que expongan SOAP:

- Pasarelas de pago tradicionales (bancos, tarjetas, ACH).
- Sistemas core bancarios legacy.
- Servicios gubernamentales (SUNAT, AFP, RUC, etc.).
- Brokers de mensajería corporativos antiguos.

En todos esos casos, **te dan un WSDL** y construyes un cliente C++ con este flujo. No escribes `.h` a mano nunca: lo genera `wsdl2h`.
