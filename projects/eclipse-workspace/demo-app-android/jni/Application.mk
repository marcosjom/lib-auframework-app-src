#Requerido para compilar Box2D
#LOCAL_CFLAGS := -fsigned-char	#en ARM char es unsigned
#LOCAL_CPPFLAGS := -fsigned-char
APP_OPTIM := release #release | debug
APP_STL := gnustl_static #stlport_static #gnustl_static --> 'gnustl_static' es requerido para la sobrecarga de operadores 'new' y 'delete'
APP_ABI := armeabi #all armeabi armeabi-v7a mips x86