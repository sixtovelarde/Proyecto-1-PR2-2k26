# Proyecto 1 PR2 2k26
Sistema de Gestión de Taller Mecánico

Autores: Francisco López. C.I: 30567703 
         Sixto Velarde. C.I: 30901575
Lenguaje: C++
Propósito académico: Proyecto de Programación II enfocado en el uso de punteros y memoria dinámica.

Descripción

Este proyecto implementa una versión inicial de un sistema de gestión para un taller mecánico. Su estructura contempla información del taller, vehículos, mecánicos, repuestos y órdenes de servicio.

En la versión actual, las funciones operativas permiten:

 ● Registrar vehículos.
 ● Asignar automáticamente un identificador a cada vehículo.
 ● Validar el año y el kilometraje antes de registrar.
 ● Buscar vehículos por su identificador.
 ● Mostrar los datos de un vehículo.
 ● Ampliar dinámicamente el arreglo de vehículos cuando se llena.
 ● Liberar la memoria dinámica al salir del programa.


Conceptos de C++ aplicados

 ● Estructuras (struct): agrupan los datos del taller, vehículos, mecánicos, repuestos y órdenes.
 ● Punteros: permiten acceder y modificar la estructura principal y trabajar con los arreglos dinámicos.
 ● Memoria dinámica (new[] y delete[]): reserva y libera espacio para los arreglos.
 ● Redimensionamiento: duplica la capacidad del arreglo de vehículos cuando se llena.
 ● Búsqueda lineal: recorre los vehículos registrados para localizar un ID.
 ● Validación básica: comprueba que el año esté entre 1950 y 2027 y que el kilometraje sea mayor que cero.
 ● Separación por capas lógicas: distingue el modelo de datos, la lógica del programa y la presentación por consola.

Organización del código

El código se divide en cuatro secciones principales:

 1. Capa de modelo: define las estructuras Taller, Vehiculo, Mecanico, Repuesto, DetalleOrden, OrdenServicio y SistemaTaller.

 2. Capa de lógica: inicializa y libera la memoria, redimensiona el arreglo de vehículos, busca registros y agrega vehículos.

 3. Capa de presentación: muestra el menú, solicita los datos al usuario y presenta los resultados.

 4. Función principal (main): inicializa el sistema, ejecuta el menú y libera la memoria antes de terminar.

Requisitos

 ● Un compilador compatible con C++, por ejemplo g++.

 ● Una terminal o entorno de desarrollo que permita compilar y ejecutar programas de C++.

Uso

Al ejecutar el programa, se muestra el menú principal:

SISTEMA DE GESTION DE TALLER

Taller: Autoservicio El Lago

1. Registrar Vehiculo

2. Buscar Vehiculo por ID

0. Salir

Seleccione una opcion:

 ● Selecciona 1 para introducir los datos de un vehículo.
 ● Selecciona 2 para buscar un vehículo por su ID.
 ● Selecciona 0 para salir y liberar la memoria dinámica.

Al registrar un vehículo, el programa valida el año y el kilometraje. Si los datos pasan esas comprobaciones, asigna un ID automáticamente y almacena el vehículo.

Administración de memoria

El sistema comienza reservando espacio para cuatro elementos en cada arreglo principal. Cuando el arreglo de vehículos se llena, redimensionarVehiculos():

 1. Duplica la capacidad.
 2. Reserva un arreglo nuevo.
 3. Copia los vehículos existentes.
 4. Libera el arreglo anterior.
 5. Actualiza el puntero para apuntar al nuevo arreglo.

Al finalizar, liberarSistema() libera los arreglos dinámicos. También contempla la liberación de los arreglos de detalles pertenecientes a las órdenes de servicio, para cuando esa funcionalidad se implemente.


