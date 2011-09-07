.onLoad <- function(libname, pkgname) {
    library.dynam("Acinonyx", pkgname, libname)

    ## resolve all known entry points (generated in R/entry_points.R)
    si <- getNativeSymbolInfo(.sym)
    env <- topenv()
    for (i in seq.int(.sym)) assign(.sym[[i]], si[[i]]$address, env)
    
    # on Mac OS X without GUI start quartz() since is will initialize the app and event loop
    if (length(grep("^darwin",R.version$os)) && !nzchar(Sys.getenv("R_GUI_APP_VERSION")))
        try({require(grDevices);grDevices::quartz(); dev.off()}, silent=TRUE)

    .Call(A_Init)
}
