#include "soapCalculadoraProxy.h"
#include "Calculadora.nsmap"
#include <iostream>
#include <cstdlib>
#include <string>

// Cliente del CASO 3: consume un servicio externo cuyo contrato (WSDL)
// nos fue entregado. NO implementamos lógica, solo llamamos al proxy.
int main(int argc, char* argv[]) {
    // Valores por defecto (se pueden sobrescribir por argumentos)
    double monto = 100.0;
    double tasa  = 3.85;

    if (argc >= 3) {
        monto = std::atof(argv[1]);
        tasa  = std::atof(argv[2]);
    }

    // Endpoint del servicio externo.
    // Cuando corre en Docker Compose, el host "server" lo resuelve la red interna.
    // Variable de entorno SOAP_ENDPOINT permite cambiarlo sin recompilar.
    const char* endpoint_env = std::getenv("SOAP_ENDPOINT");
    std::string endpoint = endpoint_env ? endpoint_env : "http://server:8081";

    CalculadoraProxy proxy;
    proxy.soap_endpoint = endpoint.c_str();

    std::cout << "[CLIENT] Conectando a " << endpoint << std::endl;
    std::cout << "[CLIENT] Llamando convertir(" << monto << ", " << tasa << ")..." << std::endl;

    // 1) Armar el request (clase generada desde el WSDL)
    _ns1__convertir request;
    request.monto = monto;
    request.tasa  = tasa;

    // 2) Preparar el response (se llena con la respuesta del servicio)
    _ns1__convertirResponse response;

    // 3) Llamada remota
    int rc = proxy.convertir(&request, response);

    // 4) Procesar resultado
    if (rc == SOAP_OK) {
        if (response.resultado != nullptr) {
            std::cout << "[CLIENT] ✓ Resultado: "
                      << monto << " x " << tasa << " = "
                      << *response.resultado << std::endl;
            return 0;
        }
        std::cerr << "[CLIENT] ✗ Respuesta vacía del servidor" << std::endl;
        return 2;
    }

    std::cerr << "[CLIENT] ✗ Error en la llamada SOAP:" << std::endl;
    proxy.soap_stream_fault(std::cerr);
    return 1;
}
