.First.lib <- function(libname, pkgname) {
    library.dynam("Acinonyx", pkgname, libname)
    .Call("A_Init", PACKAGE=pkgname)
}
