//
//  AUAppSobrecargaMemoria.cpp
//  GNavNucleo
//
//  Created by Marcos Ortega on 20/09/13.
//
//

#ifndef AUAppSobrecargaOperadoresNew_h
#define AUAppSobrecargaOperadoresNew_h

#include "NBGestorPilaLlamadas.h"
//
#include <memory> //Para "std::bad_alloc", "std::nothrow_t", etc...

//Sobrecargar los operadores NEW/DELETE
void* operator new(std::size_t size) throw(std::bad_alloc) {
	#if defined(CONFIG_NB_GESTOR_MEMORIA_GUARDAR_NOMBRES_PUNTEROS) && defined(CONFIG_NB_IMPLEMETAR_GESTOR_PILA_LLAMADAS)
	void* puntero = NBGestorMemoria::reservarMemoria(size, ENMemoriaTipo_Nucleo);
	int elementosEnPila = 0; char* referenciaEnPila = NULL;
	NBGestorPilaLlamadas::estadoPila(&elementosEnPila, NULL, &referenciaEnPila, 1);
	if(elementosEnPila>0 && referenciaEnPila != NULL){
		NB_DEFINE_NOMBRE_PUNTERO(puntero, referenciaEnPila)
	} else {
		NB_DEFINE_NOMBRE_PUNTERO(puntero, "__NO_AUOBJETO__")
	}
	return puntero;
	#else
	return NBGestorMemoria::reservarMemoria(size, ENMemoriaTipo_Nucleo);
	#endif
}

void* operator new(std::size_t size, const std::nothrow_t&) throw() {
	#if defined(CONFIG_NB_GESTOR_MEMORIA_GUARDAR_NOMBRES_PUNTEROS) && defined(CONFIG_NB_IMPLEMETAR_GESTOR_PILA_LLAMADAS)
	void* puntero = NBGestorMemoria::reservarMemoria(size, ENMemoriaTipo_Nucleo);
	int elementosEnPila = 0; char* referenciaEnPila = NULL;
	NBGestorPilaLlamadas::estadoPila(&elementosEnPila, NULL, &referenciaEnPila, 1);
	if(elementosEnPila>1 && referenciaEnPila != NULL){
		NB_DEFINE_NOMBRE_PUNTERO(puntero, referenciaEnPila)
	} else {
		NB_DEFINE_NOMBRE_PUNTERO(puntero, "__NO_AUOBJETO__")
	}
	return puntero;
	#else
	return NBGestorMemoria::reservarMemoria(size, ENMemoriaTipo_Nucleo);
	#endif
}

void  operator delete(void* ptr) throw(){
	NBGestorMemoria::liberarMemoria(ptr);
}

void  operator delete(void* ptr, const std::nothrow_t&) throw(){
	NBGestorMemoria::liberarMemoria(ptr);
}

//Segun Doc de Apple, los operadores de arreglos redirigen a los operadores de no-arreglos
void* operator new[](std::size_t size) throw(std::bad_alloc){
	#if defined(CONFIG_NB_GESTOR_MEMORIA_GUARDAR_NOMBRES_PUNTEROS) && defined(CONFIG_NB_IMPLEMETAR_GESTOR_PILA_LLAMADAS)
	void* puntero = NBGestorMemoria::reservarMemoria(size, ENMemoriaTipo_Nucleo);
	int elementosEnPila = 0; char* referenciaEnPila = NULL;
	NBGestorPilaLlamadas::estadoPila(&elementosEnPila, NULL, &referenciaEnPila, 1);
	if(elementosEnPila>1 && referenciaEnPila != NULL){
		NB_DEFINE_NOMBRE_PUNTERO(puntero, referenciaEnPila)
	} else {
		NB_DEFINE_NOMBRE_PUNTERO(puntero, "__NO_AUOBJETO__")
	}
	return puntero;
	#else
	return NBGestorMemoria::reservarMemoria(size, ENMemoriaTipo_Nucleo);
	#endif
}

void* operator new[](std::size_t size, const std::nothrow_t&) throw(){
	#if defined(CONFIG_NB_GESTOR_MEMORIA_GUARDAR_NOMBRES_PUNTEROS) && defined(CONFIG_NB_IMPLEMETAR_GESTOR_PILA_LLAMADAS)
	void* puntero = NBGestorMemoria::reservarMemoria(size, ENMemoriaTipo_Nucleo);
	int elementosEnPila = 0; char* referenciaEnPila = NULL;
	NBGestorPilaLlamadas::estadoPila(&elementosEnPila, NULL, &referenciaEnPila, 1);
	if(elementosEnPila>1 && referenciaEnPila != NULL){
		NB_DEFINE_NOMBRE_PUNTERO(puntero, referenciaEnPila)
	} else {
		NB_DEFINE_NOMBRE_PUNTERO(puntero, "__NO_AUOBJETO__")
	}
	return puntero;
	#else
	return NBGestorMemoria::reservarMemoria(size, ENMemoriaTipo_Nucleo);
	#endif
}

void  operator delete[](void* ptr) throw(){
	NBGestorMemoria::liberarMemoria(ptr);
}

void  operator delete[](void* ptr, const std::nothrow_t&) throw() {
	NBGestorMemoria::liberarMemoria(ptr);
}

#endif


