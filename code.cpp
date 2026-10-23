#include <iostream>
#include <cstring>

using namespace std;

// ========================================================
// 1. CAPA DE MODELO: ESTRUCTURAS DE DATOS OBLIGATORIAS
// ========================================================

struct Taller {
    char nombre[100];
    char rif[20];
    char direccion[200];
    char telefono[20];
};

struct Vehiculo {
    int id;
    char placa[10];
    char marca[50];
    char modelo[50];
    int anio;
    int kilometraje;
    char nombrePropietario[100];
    char cedulaPropietario[20];
    char telefonoPropietario[20];
    char fechaRegistro[11];
};

struct Mecanico {
    int id;
    char nombre[100];
    char cedula[20];
    char especialidad[20];
    char fechaIngreso[11];
};

struct Repuesto {
    int id;
    char codigo[20];
    char nombre[100];
    char marca[50];
    float precio;
    int stock;
    int stockMinimo;
    char fechaRegistro[11];
};

struct DetalleOrden {
    int idRepuesto;
    int cantidad;
    float precioUnitario;
};

struct OrdenServicio {
    int id;
    int idVehiculo;
    int idMecanico;
    char fechaApertura[11];
    char fechaCierre[11];
    char estado[12];
    char descripcionFalla[200];
    int kilometrajeIngreso;
    float manoObra;
    
    DetalleOrden* detalles; 
    int numDetalles;
    int capacidadDetalles;
};

struct SistemaTaller {
    Taller taller;
    
    Vehiculo* vehiculos;
    int numVehiculos;
    int capacidadVehiculos;
    
    Mecanico* mecanicos;
    int numMecanicos;
    int capacidadMecanicos;
    
    Repuesto* repuestos;
    int numRepuestos;
    int capacidadRepuestos;
    
    OrdenServicio* ordenes;
    int numOrdenes;
    int capacidadOrdenes;
    
    int siguienteIdVehiculo;
    int siguienteIdMecanico;
    int siguienteIdRepuesto;
    int siguienteIdOrden;
};

// ========================================================
// 2. CAPA DE LÓGICA (PROHIBIDO USAR CIN / COUT AQUÍ)
// ========================================================


void inicializarSistema(SistemaTaller* s) {
    strcpy(s->taller.nombre, "Autoservicio El Lago");
    
    s->capacidadVehiculos = 4;
    s->numVehiculos = 0;
    s->vehiculos = new Vehiculo[s->capacidadVehiculos];
    s->siguienteIdVehiculo = 1;
    
    s->capacidadMecanicos = 4;
    s->numMecanicos = 0;
    s->mecanicos = new Mecanico[s->capacidadMecanicos];
    s->siguienteIdMecanico = 1;
    
    s->capacidadRepuestos = 4;
    s->numRepuestos = 0;
    s->repuestos = new Repuesto[s->capacidadRepuestos];
    s->siguienteIdRepuesto = 1;
    
    s->capacidadOrdenes = 4;
    s->numOrdenes = 0;
    s->ordenes = new OrdenServicio[s->capacidadOrdenes];
    s->siguienteIdOrden = 1;
}

void liberarSistema(SistemaTaller* s) {
    delete[] s->vehiculos;
    delete[] s->mecanicos;
    delete[] s->repuestos;
    
    for (int i = 0; i < s->numOrdenes; i++) {
        delete[] s->ordenes[i].detalles;
    }
    delete[] s->ordenes;
}


void redimensionarVehiculos(SistemaTaller* s) {
    s->capacidadVehiculos *= 2;
    Vehiculo* nuevoArreglo = new Vehiculo[s->capacidadVehiculos];
    
    for (int i = 0; i < s->numVehiculos; i++) {
        nuevoArreglo[i] = s->vehiculos[i];
    }
    
    delete[] s->vehiculos;
    s->vehiculos = nuevoArreglo;
}


Vehiculo* buscarVehiculoPorID(SistemaTaller* s, int id) {
    for (int i = 0; i < s->numVehiculos; i++) {
        if (s->vehiculos[i].id == id) {
            return &(s->vehiculos[i]);
        }
    }
    return nullptr;
}

Vehiculo* agregarVehiculo(SistemaTaller* s, Vehiculo datos) {
    // Validación: Año entre 1950 y 2027
    if (datos.anio < 1950 || datos.anio > 2027) return nullptr;
    if (datos.kilometraje <= 0) return nullptr;
    
    if (s->numVehiculos == s->capacidadVehiculos) {
        redimensionarVehiculos(s);
    }
    
    datos.id = s->siguienteIdVehiculo++;
    s->vehiculos[s->numVehiculos] = datos;
    s->numVehiculos++;
    
    return &(s->vehiculos[s->numVehiculos - 1]);
}

// ========================================================
// 3. CAPA DE PRESENTACIÓN (ÚNICA CON CIN / COUT)
// ========================================================

void mostrarVehiculo(Vehiculo* v) {
    if (v == nullptr) {
        cout << "Vehiculo no encontrado.\n";
        return;
    }
    cout << "\n--- Datos del Vehiculo ---\n";
    cout << "ID: " << v->id << " | Placa: " << v->placa << "\n";
    cout << "Marca/Modelo: " << v->marca << " " << v->modelo << " (" << v->anio << ")\n";
    cout << "Propietario: " << v->nombrePropietario << "\n";
    cout << "--------------------------\n";
}

void menuRegistrarVehiculo(SistemaTaller* s) {
    Vehiculo temp;
    cout << "\n--- REGISTRAR NUEVO VEHICULO ---\n";
    
    cout << "Placa: ";
    cin >> temp.placa;
    cout << "Marca: ";
    cin >> temp.marca;
    cout << "Modelo: ";
    cin >> temp.modelo;
    cout << "Anio: ";
    cin >> temp.anio;
    cout << "Kilometraje: ";
    cin >> temp.kilometraje;
    cin.ignore(); 
    
    cout << "Nombre del propietario: ";
    cin.getline(temp.nombrePropietario, 100);
    cout << "Cedula del propietario: ";
    cin >> temp.cedulaPropietario;
    cout << "Telefono del propietario: ";
    cin >> temp.telefonoPropietario;
    strcpy(temp.fechaRegistro, "2026-10-17"); // Fecha simulada para el ejemplo
    
    Vehiculo* nuevo = agregarVehiculo(s, temp);
    
    if (nuevo != nullptr) {
        cout << "\nEXITO: Vehiculo registrado con el ID " << nuevo->id << ".\n";
    } else {
        cout << "\nERROR: Datos invalidos (Revise el anio o kilometraje).\n";
    }
}

void menuPrincipal(SistemaTaller* s) {
    int opcion;
    do {
        cout << "\nSISTEMA DE GESTION DE TALLER\n";
        cout << "Taller: " << s->taller.nombre << "\n";
        cout << "1. Registrar Vehiculo\n";
        cout << "2. Buscar Vehiculo por ID\n";
        cout << "0. Salir\n";
        cout << "Seleccione una opcion: ";
        cin >> opcion;
        
        switch (opcion) {
            case 1:
                menuRegistrarVehiculo(s);
                break;
            case 2: {
                int idBusqueda;
                cout << "Ingrese el ID del vehiculo: ";
                cin >> idBusqueda;
                Vehiculo* encontrado = buscarVehiculoPorID(s, idBusqueda);
                mostrarVehiculo(encontrado);
                break;
            }
            case 0:
                cout << "Saliendo del sistema...\n";
                break;
            default:
                cout << "Opcion invalida. Intente de nuevo.\n";
        }
    } while (opcion != 0);
}

// ========================================================
// 4. FUNCIÓN PRINCIPAL
// ========================================================

int main() {
    SistemaTaller miTaller;
    
    // Inicializar memoria dinámica requerida
    inicializarSistema(&miTaller);
    
    // Iniciar interfaz de usuario
    menuPrincipal(&miTaller);
    
    // Liberar memoria antes de terminar para evitar fugas (Memory Leaks)
    liberarSistema(&miTaller);
    
    return 0;
}
