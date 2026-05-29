#include "soapCalculadoraService.h"
#include "Calculadora.nsmap"
#include <iostream>

// gSOAP genera toda la clase CalculadoraService EXCEPTO este método.
// Aquí va la lógica de negocio: en este lab, simulamos un "convertidor de moneda".
int CalculadoraService::convertir(double monto, double tasa, double* resultado) {
    *resultado = monto * tasa;
    std::cout << "[SERVER] convertir(" << monto << ", " << tasa
              << ") = " << *resultado << std::endl;
    return SOAP_OK;
}

int main() {
    CalculadoraService servicio;
    std::cout << "[SERVER] Servicio SOAP escuchando en puerto 8081..." << std::endl;
    std::cout << "[SERVER] Esperando peticiones..." << std::endl;

    if (!soap_valid_socket(servicio.run(8081))) {
        servicio.soap_stream_fault(std::cerr);
        return 1;
    }
    return 0;
}
