 #include <iostream>
#include <fstream>
#include <cstring>
#include <cctype>
#include <ctime>

#ifdef _WIN32
    #include <direct.h>
    #define CREAR_CARPETA(dir) _mkdir(dir)
#else
    #include <sys/stat.h>
    #define CREAR_CARPETA(dir) mkdir(dir, 0777)
#endif

using namespace std;


// 1. CAPA DE MODELO: ESTRUCTURAS Y HEADER


struct ArchivoHeader {
    int cantidadRegistros; // Total histórico (incluye eliminados lógicamente)[cite: 4]
    int proximoID;         // Siguiente ID a asignar[cite: 4]
    int registrosActivos;  // Registros con eliminado == false[cite: 4]
    int version;           // Versión del formato (iniciar en 1)[cite: 4]
};

struct Taller {
    char nombre[100];
    char rif[20];
    char direccion[200];
    char telefono[20];
    time_t fechaCreacion;
    time_t fechaUltimaModificacion;
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
    bool eliminado;
    time_t fechaCreacion;
    time_t fechaUltimaModificacion;
};

struct Mecanico {
    int id;
    char nombre[100];
    char cedula[20];
    char especialidad[20];
    char fechaIngreso[11];
    bool eliminado;
    time_t fechaCreacion;
    time_t fechaUltimaModificacion;
};

struct Repuesto {
    int id;
    char codigo[20];
    char nombre[100];
    char marca[50];
    float precio;
    int stock;
    int stockMinimo;
    bool eliminado;
    time_t fechaCreacion;
    time_t fechaUltimaModificacion;
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
    char estado[12]; // "ABIERTA", "CERRADA", "ANULADA"
    char descripcionFalla[200];
    int kilometrajeIngreso;
    float manoObra;
    DetalleOrden detalles[15];
    int numDetalles;
    bool eliminado;
    time_t fechaCreacion;
    time_t fechaUltimaModificacion;
};

enum EstadoBusquedaVehiculo {
    VEHICULO_ENCONTRADO = 0,
    VEHICULO_NO_EXISTE = 1,
    VEHICULO_ELIMINADO = 2,
    VEHICULO_ERROR_ARCHIVO = 3
};

enum ResultadoRegistroVehiculo {
    REGISTRO_OK = 0,
    REGISTRO_DATOS_INVALIDOS = 1,
    REGISTRO_PLACA_DUPLICADA = 2,
    REGISTRO_ERROR_ARCHIVO = 3
};

const char* CARPETA_DATOS = "datos";
const char* RUTA_VEHICULOS = "datos/vehiculos.bin";
const int MAX_RESULTADOS_BUSQUEDA = 20;


// 2. CAPA DE PERSISTENCIA (SIN CIN / COUT NI REGLAS DE NEGOCIO)


bool asegurarCarpetaDatos(const char* carpeta) {
    ifstream prueba(carpeta);
    if (prueba.good()) {
        prueba.close();
        return true;
    }
    prueba.close();
    CREAR_CARPETA(carpeta);
    return true;
}

long long calcularOffsetVehiculo(int id) {
    long long indiceFisico = static_cast<long long>(id) - 1;
    return static_cast<long long>(sizeof(ArchivoHeader)) +
           indiceFisico * static_cast<long long>(sizeof(Vehiculo));
}

bool inicializarArchivoVehiculos(const char* ruta, bool& creadoNuevo) {
    creadoNuevo = false;
    asegurarCarpetaDatos(CARPETA_DATOS);

    ifstream entrada(ruta, ios::binary);
    if (entrada.good()) {
        entrada.close();
        return true;
    }
    entrada.close();

    ofstream salida(ruta, ios::binary | ios::trunc);
    if (!salida.is_open()) {
        return false;
    }

    ArchivoHeader header{0, 1, 0, 1};
    salida.write(reinterpret_cast<const char*>(&header), sizeof(ArchivoHeader));
    bool ok = salida.good();
    salida.close();
    if (ok) creadoNuevo = true;
    return ok;
}

bool leerHeaderVehiculos(const char* ruta, ArchivoHeader& header) {
    ifstream archivo(ruta, ios::binary);
    if (!archivo.is_open()) return false;
    archivo.read(reinterpret_cast<char*>(&header), sizeof(ArchivoHeader));
    bool ok = archivo.gcount() == static_cast<streamsize>(sizeof(ArchivoHeader));
    archivo.close();
    return ok;
}

bool actualizarHeaderVehiculos(const char* ruta, const ArchivoHeader& header) {
    fstream archivo(ruta, ios::in | ios::out | ios::binary);
    if (!archivo.is_open()) return false;
    archivo.seekp(0, ios::beg);
    archivo.write(reinterpret_cast<const char*>(&header), sizeof(ArchivoHeader));
    bool ok = archivo.good();
    archivo.close();
    return ok;
}

bool guardarVehiculoNuevoBin(const char* ruta, Vehiculo& registro, int& idAsignado) {
    ArchivoHeader header;
    if (!leerHeaderVehiculos(ruta, header)) return false;

    fstream archivo(ruta, ios::in | ios::out | ios::binary);
    if (!archivo.is_open()) return false;

    time_t ahora = time(nullptr);
    registro.id = header.proximoID;
    registro.eliminado = false;
    registro.fechaCreacion = ahora;
    registro.fechaUltimaModificacion = ahora;

    long long offset = calcularOffsetVehiculo(registro.id);
    archivo.seekp(offset, ios::beg);
    archivo.write(reinterpret_cast<const char*>(&registro), sizeof(Vehiculo));
    if (!archivo.good()) {
        archivo.close();
        return false;
    }
    archivo.close();

    header.cantidadRegistros++;
    header.registrosActivos++;
    header.proximoID++;
    if (!actualizarHeaderVehiculos(ruta, header)) return false;

    idAsignado = registro.id;
    return true;
}

EstadoBusquedaVehiculo leerVehiculoPorIDBin(const char* ruta, int id, Vehiculo& salida, long long* offsetUsado = nullptr) {
    if (id <= 0) return VEHICULO_NO_EXISTE;

    ArchivoHeader header;
    if (!leerHeaderVehiculos(ruta, header)) return VEHICULO_ERROR_ARCHIVO;
    if (id >= header.proximoID) return VEHICULO_NO_EXISTE;

    ifstream archivo(ruta, ios::binary);
    if (!archivo.is_open()) return VEHICULO_ERROR_ARCHIVO;

    long long offset = calcularOffsetVehiculo(id);
    if (offsetUsado != nullptr) *offsetUsado = offset;

    archivo.seekg(offset, ios::beg);
    archivo.read(reinterpret_cast<char*>(&salida), sizeof(Vehiculo));
    bool ok = archivo.gcount() == static_cast<streamsize>(sizeof(Vehiculo));
    archivo.close();

    if (!ok) return VEHICULO_ERROR_ARCHIVO;
    if (salida.eliminado) return VEHICULO_ELIMINADO;
    return VEHICULO_ENCONTRADO;
}

bool sobrescribirVehiculoBin(const char* ruta, Vehiculo& registro) {
    if (registro.id <= 0) return false;

    fstream archivo(ruta, ios::in | ios::out | ios::binary);
    if (!archivo.is_open()) return false;

    registro.fechaUltimaModificacion = time(nullptr);
    long long offset = calcularOffsetVehiculo(registro.id);
    archivo.seekp(offset, ios::beg);
    archivo.write(reinterpret_cast<const char*>(&registro), sizeof(Vehiculo));
    bool ok = archivo.good();
    archivo.close();
    return ok;
}

bool marcarVehiculoEliminadoBin(const char* ruta, int id) {
    ArchivoHeader header;
    if (!leerHeaderVehiculos(ruta, header)) return false;
    if (id <= 0 || id >= header.proximoID) return false;

    fstream archivo(ruta, ios::in | ios::out | ios::binary);
    if (!archivo.is_open()) return false;

    long long offset = calcularOffsetVehiculo(id);
    Vehiculo actual;
    archivo.seekg(offset, ios::beg);
    archivo.read(reinterpret_cast<char*>(&actual), sizeof(Vehiculo));
    if (archivo.gcount() != static_cast<streamsize>(sizeof(Vehiculo)) || actual.eliminado) {
        archivo.close();
        return false;
    }

    actual.eliminado = true;
    actual.fechaUltimaModificacion = time(nullptr);
    archivo.seekp(offset, ios::beg);
    archivo.write(reinterpret_cast<const char*>(&actual), sizeof(Vehiculo));
    bool ok = archivo.good();
    archivo.close();

    if (!ok) return false;

    header.registrosActivos--;
    return actualizarHeaderVehiculos(ruta, header);
}

bool contarVehiculosActivosBin(const char* ruta, int& activos) {
    ArchivoHeader header;
    if (!leerHeaderVehiculos(ruta, header)) return false;
    activos = header.registrosActivos;
    return true;
}

// 3. CAPA DE LÓGICA (SIN CIN / COUT)


static string aMinusculas(const char* texto) {
    string res(texto);
    for (char& c : res) {
        c = static_cast<char>(tolower(static_cast<unsigned char>(c)));
    }
    return res;
}

bool existePlacaActiva(const char* ruta, const char* placa) {
    ArchivoHeader header;
    if (!leerHeaderVehiculos(ruta, header)) return false;

    ifstream archivo(ruta, ios::binary);
    if (!archivo.is_open()) return false;

    archivo.seekg(sizeof(ArchivoHeader), ios::beg);
    Vehiculo actual;
    for (int i = 0; i < header.cantidadRegistros; i++) {
        archivo.read(reinterpret_cast<char*>(&actual), sizeof(Vehiculo));
        if (archivo.gcount() != static_cast<streamsize>(sizeof(Vehiculo))) break;
        if (!actual.eliminado && strcmp(actual.placa, placa) == 0) {
            archivo.close();
            return true;
        }
    }
    archivo.close();
    return false;
}

ResultadoRegistroVehiculo registrarVehiculoLogica(const char* ruta, Vehiculo datos, int& idGenerado) {
    if (datos.anio < 1950 || datos.anio > 2027 || datos.kilometraje < 0 || strlen(datos.placa) == 0) {
        return REGISTRO_DATOS_INVALIDOS;
    }
    if (existePlacaActiva(ruta, datos.placa)) {
        return REGISTRO_PLACA_DUPLICADA;
    }
    if (!guardarVehiculoNuevoBin(ruta, datos, idGenerado)) {
        return REGISTRO_ERROR_ARCHIVO;
    }
    return REGISTRO_OK;
}

EstadoBusquedaVehiculo obtenerVehiculoPorIDLogica(const char* ruta, int id, Vehiculo& salida, long long* offsetUsado = nullptr) {
    return leerVehiculoPorIDBin(ruta, id, salida, offsetUsado);
}

EstadoBusquedaVehiculo obtenerVehiculoPorPlacaLogica(const char* ruta, const char* placa, Vehiculo& salida) {
    ArchivoHeader header;
    if (!leerHeaderVehiculos(ruta, header)) return VEHICULO_ERROR_ARCHIVO;

    ifstream archivo(ruta, ios::binary);
    if (!archivo.is_open()) return VEHICULO_ERROR_ARCHIVO;

    archivo.seekg(sizeof(ArchivoHeader), ios::beg);
    Vehiculo actual;
    bool encontroEliminado = false;
    Vehiculo ultimoEliminado{};

    for (int i = 0; i < header.cantidadRegistros; i++) {
        archivo.read(reinterpret_cast<char*>(&actual), sizeof(Vehiculo));
        if (archivo.gcount() != static_cast<streamsize>(sizeof(Vehiculo))) break;
        if (strcmp(actual.placa, placa) == 0) {
            if (!actual.eliminado) {
                salida = actual;
                archivo.close();
                return VEHICULO_ENCONTRADO;
            }
            encontroEliminado = true;
            ultimoEliminado = actual;
        }
    }
    archivo.close();

    if (encontroEliminado) {
        salida = ultimoEliminado;
        return VEHICULO_ELIMINADO;
    }
    return VEHICULO_NO_EXISTE;
}

int buscarVehiculosPorPropietarioLogica(const char* ruta, const char* subcadena, Vehiculo resultados[], int maxResultados, bool& hayMas) {
    hayMas = false;
    ArchivoHeader header;
    if (!leerHeaderVehiculos(ruta, header)) return -1;

    ifstream archivo(ruta, ios::binary);
    if (!archivo.is_open()) return -1;

    archivo.seekg(sizeof(ArchivoHeader), ios::beg);
    Vehiculo actual;
    string sub = aMinusculas(subcadena);
    int encontrados = 0;

    for (int i = 0; i < header.cantidadRegistros; i++) {
        archivo.read(reinterpret_cast<char*>(&actual), sizeof(Vehiculo));
        if (archivo.gcount() != static_cast<streamsize>(sizeof(Vehiculo))) break;
        if (!actual.eliminado) {
            string prop = aMinusculas(actual.nombrePropietario);
            if (prop.find(sub) != string::npos) {
                if (encontrados < maxResultados) {
                    resultados[encontrados++] = actual;
                } else {
                    hayMas = true;
                }
            }
        }
    }
    archivo.close();
    return encontrados;
}

bool actualizarVehiculoLogica(const char* ruta, int id, const Vehiculo& nuevosDatos) {
    if (nuevosDatos.anio < 1950 || nuevosDatos.anio > 2027) return false;

    Vehiculo actual;
    long long offset = 0;
    if (leerVehiculoPorIDBin(ruta, id, actual, &offset) != VEHICULO_ENCONTRADO) {
        return false;
    }

    strncpy(actual.marca, nuevosDatos.marca, sizeof(actual.marca) - 1);
    actual.marca[sizeof(actual.marca) - 1] = '\0';

    strncpy(actual.modelo, nuevosDatos.modelo, sizeof(actual.modelo) - 1);
    actual.modelo[sizeof(actual.modelo) - 1] = '\0';

    actual.anio = nuevosDatos.anio;

    strncpy(actual.nombrePropietario, nuevosDatos.nombrePropietario, sizeof(actual.nombrePropietario) - 1);
    actual.nombrePropietario[sizeof(actual.nombrePropietario) - 1] = '\0';

    strncpy(actual.cedulaPropietario, nuevosDatos.cedulaPropietario, sizeof(actual.cedulaPropietario) - 1);
    actual.cedulaPropietario[sizeof(actual.cedulaPropietario) - 1] = '\0';

    strncpy(actual.telefonoPropietario, nuevosDatos.telefonoPropietario, sizeof(actual.telefonoPropietario) - 1);
    actual.telefonoPropietario[sizeof(actual.telefonoPropietario) - 1] = '\0';

    return sobrescribirVehiculoBin(ruta, actual);
}


// 4. Capa de presentacion unica con cin / cout


void mostrarVehiculo(const Vehiculo& v) {
    cout << "\n--- Datos del Vehiculo ---\n";
    cout << "ID: " << v.id << " | Placa: " << v.placa << "\n";
    cout << "Marca/Modelo: " << v.marca << " " << v.modelo << " (" << v.anio << ")\n";
    cout << "Kilometraje: " << v.kilometraje << " km\n";
    cout << "Propietario: " << v.nombrePropietario << " | CI: " << v.cedulaPropietario << " | Tel: " << v.telefonoPropietario << "\n";
    cout << "--------------------------\n";
}

void menuRegistrarVehiculo() {
    Vehiculo temp{};
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
    cin.getline(temp.nombrePropietario, sizeof(temp.nombrePropietario));
    cout << "Cedula del propietario: ";
    cin >> temp.cedulaPropietario;
    cout << "Telefono del propietario: ";
    cin >> temp.telefonoPropietario;

    int idCreado = 0;
    ResultadoRegistroVehiculo res = registrarVehiculoLogica(RUTA_VEHICULOS, temp, idCreado);
    if (res == REGISTRO_OK) {
        cout << "\nEXITO: Vehiculo registrado con el ID " << idCreado << ".\n";
    } else if (res == REGISTRO_PLACA_DUPLICADA) {
        cout << "\nERROR: Ya existe un vehiculo activo con esa placa.\n";
    } else if (res == REGISTRO_DATOS_INVALIDOS) {
        cout << "\nERROR: Datos invalidos (anio entre 1950 y 2027, kilometraje >= 0 y placa no vacia).\n";
    } else {
        cout << "\nERROR: No se pudo abrir o escribir en " << RUTA_VEHICULOS << ".\n";
    }
}

void menuBuscarVehiculoPorID() {
    int idBusqueda;
    cout << "Ingrese el ID del vehiculo: ";
    cin >> idBusqueda;

    Vehiculo encontrado{};
    long long offset = 0;
    EstadoBusquedaVehiculo estado = obtenerVehiculoPorIDLogica(RUTA_VEHICULOS, idBusqueda, encontrado, &offset);

    if (estado == VEHICULO_ENCONTRADO) {
        cout << "[DEBUG] Leido en byte offset: " << offset << "\n";
        mostrarVehiculo(encontrado);
    } else if (estado == VEHICULO_ELIMINADO) {
        cout << "AVISO: El vehiculo con ID " << idBusqueda << " fue eliminado logicamente.\n";
    } else if (estado == VEHICULO_NO_EXISTE) {
        cout << "Vehiculo con ID " << idBusqueda << " no existe.\n";
    } else {
        cout << "ERROR: Fallo al acceder a " << RUTA_VEHICULOS << ".\n";
    }
}

void menuBuscarVehiculoPorPlaca() {
    char placa[10];
    cout << "Ingrese la placa: ";
    cin >> placa;

    Vehiculo encontrado{};
    EstadoBusquedaVehiculo estado = obtenerVehiculoPorPlacaLogica(RUTA_VEHICULOS, placa, encontrado);

    if (estado == VEHICULO_ENCONTRADO) {
        mostrarVehiculo(encontrado);
    } else if (estado == VEHICULO_ELIMINADO) {
        cout << "AVISO: El vehiculo con placa '" << placa << "' corresponde a un registro eliminado.\n";
    } else if (estado == VEHICULO_NO_EXISTE) {
        cout << "Vehiculo con placa '" << placa << "' no encontrado.\n";
    } else {
        cout << "ERROR: Fallo al acceder a " << RUTA_VEHICULOS << ".\n";
    }
}

void menuBuscarVehiculoPorPropietario() {
    char sub[100];
    cout << "Ingrese subcadena del nombre del propietario: ";
    cin.ignore();
    cin.getline(sub, sizeof(sub));

    Vehiculo resultados[MAX_RESULTADOS_BUSQUEDA];
    bool hayMas = false;
    int cant = buscarVehiculosPorPropietarioLogica(RUTA_VEHICULOS, sub, resultados, MAX_RESULTADOS_BUSQUEDA, hayMas);

    if (cant < 0) {
        cout << "ERROR: Fallo al leer " << RUTA_VEHICULOS << ".\n";
        return;
    }
    if (cant == 0) {
        cout << "No se encontraron vehiculos activos con ese propietario.\n";
        return;
    }

    for (int i = 0; i < cant; i++) {
        mostrarVehiculo(resultados[i]);
    }
    if (hayMas) {
        cout << "AVISO: Hay mas resultados disponibles que superan el limite de " << MAX_RESULTADOS_BUSQUEDA << ".\n";
    }
}

void menuPrincipal() {
    int opcion;
    do {
        cout << "\nSISTEMA DE GESTION DE TALLER\n";
        cout << "1. Registrar Vehiculo\n";
        cout << "2. Buscar Vehiculo por ID\n";
        cout << "3. Buscar Vehiculo por Placa\n";
        cout << "4. Buscar Vehiculo por Propietario\n";
        cout << "0. Salir\n";
        cout << "Seleccione una opcion: ";
        if (!(cin >> opcion)) {
            cin.clear();
            cin.ignore(10000, '\n');
            continue;
        }

        switch (opcion) {
            case 1:
                menuRegistrarVehiculo();
                break;
            case 2:
                menuBuscarVehiculoPorID();
                break;
            case 3:
                menuBuscarVehiculoPorPlaca();
                break;
            case 4:
                menuBuscarVehiculoPorPropietario();
                break;
            case 0:
                cout << "Saliendo del sistema...\n";
                break;
            default:
                cout << "Opcion invalida. Intente de nuevo.\n";
        }
    } while (opcion != 0);
}


// 5. Funcion principal


int main() {
    bool creadoNuevo = false;
    if (!inicializarArchivoVehiculos(RUTA_VEHICULOS, creadoNuevo)) {
        cerr << "ERROR: No se pudo inicializar " << RUTA_VEHICULOS << ".\n";
        return 1;
    }
    if (creadoNuevo) {
        cout << "Archivo " << RUTA_VEHICULOS << " inicializado con header por defecto.\n";
    }

    menuPrincipal();
    return 0;
}