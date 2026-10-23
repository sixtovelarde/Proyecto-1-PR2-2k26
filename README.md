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

Preguntas de Análisis   

1. agregarVehiculo retorna un puntero a un elemento de s->vehiculos. Si guardan ese puntero en una variable, registran varios vehículos más y luego lo usan, ¿qué puede ocurrir y por qué?

Si guardo ese puntero y luego sigo agregando vehículos, en algún momento el arreglo alcanzará su capacidad máxima y se va a llamar a la función de redimensionamiento, esa función crea un arreglo más grande, copia los datos y le hace un delete[] al arreglo viejo, el puntero que yo había guardado se convierte en un puntero colgante porque ahora apunta a un espacio de memoria que ya fue liberado. Si intento usarlo, el programa va a fallar.

2. ¿Por qué DetalleOrden guarda precioUnitario en lugar de consultar el precio del repuesto cada vez que se calcula el total?

Porque necesitamos guardar el precio histórico. Si hoy le pongo a un cliente unas pastillas de freno a $25 y cierro la orden, el total queda fijo, si la semana que viene el proveedor sube las pastillas a $35 y yo actualizo el precio en el inventario, las órdenes viejas no pueden recalcularse con el precio nuevo, guardar el precioUnitario en la línea de detalle congela el precio al momento exacto en que se agregó a la orden.

3. Cuando se redimensiona el arreglo de órdenes, ¿qué ocurre con el arreglo de detalles de cada orden? ¿Se copia, se comparte o se pierde? Justifiquen con su implementación.

Se comparte temporalmente al redimensionar el arreglo de s->ordenes, hacemos una copia miembro a miembro (nuevoArreglo[i] = s->ordenes[i]), como la estructura OrdenServicio contiene un puntero detalles, lo que se está copiando hacia el nuevo arreglo es la dirección de memoria de los detalles, no los detalles en sí. Por lo tanto, tanto la orden vieja como la nueva apuntan al mismo arreglo de detalles, esto es seguro siempre y cuando al liberar el arreglo viejo de órdenes (delete[] s->ordenes) no se hace delete de los detalles internos, porque si no, destruiríamos la información de las órdenes del nuevo arreglo.
