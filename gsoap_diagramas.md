# gSOAP — Caso 3: Diagramas

Cuatro vistas distintas del mismo proceso, de lo más general a lo más operativo. Cada diagrama responde a una pregunta específica que típicamente surge al aprender gSOAP.

---

## 1. ¿Qué es cada cosa? — Mapa mental

Antes de tocar comandos, hay que saber qué papel cumple cada pieza. Estas son las 3 que más se confunden:

```mermaid
flowchart LR
    WSDL["📄 WSDL<br/><i>contrato XML</i><br/>define el servicio"]
    WSDL2H["🔧 wsdl2h<br/><i>herramienta</i><br/>WSDL → header C++"]
    SOAPCPP2["⚙️ soapcpp2<br/><i>herramienta</i><br/>header → código de red"]

    WSDL --- WSDL2H
    WSDL2H --- SOAPCPP2

    style WSDL fill:#FEF3C7,stroke:#D97706,color:#000
    style WSDL2H fill:#DBEAFE,stroke:#2563EB,color:#000
    style SOAPCPP2 fill:#D1FAE5,stroke:#059669,color:#000
```

**Idea clave:** WSDL es un **archivo** (el contrato). `wsdl2h` y `soapcpp2` son **herramientas** distintas con propósitos distintos. Confundirlas es el error más común.

---

## 2. ¿Qué escribo yo y qué genera gSOAP? — División de trabajo

Lo que más frena al principio es la cantidad de archivos. Este diagrama deja claro que tú solo escribes **uno**:

```mermaid
flowchart TB
    subgraph TU [" 👤 LO QUE TÚ HACES "]
        direction TB
        T1[client.cpp<br/>• arma request<br/>• llama al proxy<br/>• lee response<br/>• imprime resultado]
    end

    subgraph GSOAP [" 🤖 LO QUE gSOAP GENERA "]
        direction TB
        G1[banco.h<br/><i>de wsdl2h</i>]
        G2[soapBancoProxy.h/.cpp]
        G3[soapC.cpp<br/><i>~63 KB de serializadores</i>]
        G4[soapH.h, soapStub.h<br/>Banco.nsmap]
    end

    TU -. "usa" .-> GSOAP

    style TU fill:#D1FAE5,stroke:#059669,color:#000
    style GSOAP fill:#E0E7FF,stroke:#6366F1,color:#000
    style T1 fill:#FFFFFF,stroke:#059669,color:#000
    style G1 fill:#FFFFFF,stroke:#6366F1,color:#000
    style G2 fill:#FFFFFF,stroke:#6366F1,color:#000
    style G3 fill:#FFFFFF,stroke:#6366F1,color:#000
    style G4 fill:#FFFFFF,stroke:#6366F1,color:#000
```

**Idea clave:** todo lo del bloque morado es ruido autogenerado. No lo lees, no lo editas. Solo compilas y olvidas. Tú vives en `client.cpp`.

---

## 3. Pipeline operativo — Qué genera cada paso

La guía paso a paso para ir del WSDL al binario. Cada flecha es un comando:

```mermaid
flowchart TB
    A["📄 banco.wsdl<br/><i>(entrada externa)</i>"]
    B["📄 banco.h<br/><i>(header generado)</i>"]
    C["📁 soapBancoProxy.h<br/>soapBancoProxy.cpp<br/>soapC.cpp<br/>Banco.nsmap"]
    D["✏️ client.cpp<br/><i>(tu código)</i>"]
    E["🚀 ./client<br/><i>(binario)</i>"]

    A -->|"wsdl2h -o banco.h banco.wsdl"| B
    B -->|"soapcpp2 -j -C banco.h"| C
    C -->|"+"| F[" "]
    D -->|"+"| F
    F -->|"g++ -o client client.cpp<br/>soapBancoProxy.cpp soapC.cpp<br/>-lgsoap++"| E

    style A fill:#FEF3C7,stroke:#D97706,color:#000
    style B fill:#DBEAFE,stroke:#2563EB,color:#000
    style C fill:#E0E7FF,stroke:#6366F1,color:#000
    style D fill:#D1FAE5,stroke:#059669,color:#000
    style E fill:#FCE7F3,stroke:#DB2777,color:#000
    style F fill:#FFFFFF,stroke:#FFFFFF,color:#000
```

**Idea clave:** son **3 comandos en total**: `wsdl2h`, `soapcpp2`, `g++`. Nada más. Lo demás es código y archivos que pasan entre ellos.

---

## 4. Anatomía de una llamada en runtime — Qué viaja por la red

Después de compilar, ¿qué pasa cuando ejecutas `./client`? Esta es la vista dinámica:

```mermaid
sequenceDiagram
    autonumber
    participant App as client.cpp<br/>(tu código)
    participant Proxy as CalculadoraProxy<br/>(generado)
    participant Server as Servicio externo<br/>(otro proceso/máquina)

    App->>Proxy: proxy.convertir(req, resp)
    Note over Proxy: serializa req a XML
    Proxy->>Server: HTTP POST con XML del request<br/>(monto=100, tasa=3.85)
    Note over Server: deserializa<br/>ejecuta lógica<br/>serializa respuesta
    Server-->>Proxy: HTTP 200 con XML del response<br/>(resultado=385)
    Note over Proxy: deserializa XML a resp
    Proxy-->>App: SOAP_OK, resp.resultado=385
    Note over App: cout muestra resultado
```

**Idea clave:** lo que **parece** una llamada local (`proxy.convertir(...)`) es en realidad un viaje completo de XML por HTTP. El proxy es quien hace toda esa magia sin que tú escribas una línea de red.

---

## Cómo usar este documento

Recomendación para tus alumnos:

1. Lee los **diagramas 1 y 2 primero** (5 min). Te sitúa antes de tocar comandos.
2. Sigue el **diagrama 3** como guía para hacer el lab paso a paso.
3. Una vez compilado, mira el **diagrama 4** para entender qué pasa en runtime.
