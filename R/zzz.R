.onLoad <- function(libname, pkgname) {
    library.dynam("Acinonyx", pkgname, libname)

    ## resolve all known entry points (generated in R/entry_points.R)
    si <- getNativeSymbolInfo(.sym)
    env <- topenv()
    for (i in seq.int(.sym)) assign(.sym[[i]], si[[i]]$address, env)

    cfs <- is.loaded("A_SetCoreFontPath", PACKAGE = pkgname)
    
    # on Mac OS X without GUI start quartz() since is will initialize the app and event loop
    if (!cfs && length(grep("^darwin",R.version$os)) && !nzchar(Sys.getenv("R_GUI_APP_VERSION")))
        try({require(grDevices);grDevices::quartz(); dev.off()}, silent=TRUE)

    # in order to avoid the mess involving linking to FontConfig, we jsut
    # try to use fc-match to get what we want - this may fail, though so we
    # ask for the "ix.core.font.path" option
    if (cfs) {
        fm <- getOption("ix.core.font.path")
        if (is.null(fm)) {
            fm <- try(system("fc-match -f '%{file}' Arial 2> /dev/null", intern = TRUE), silent=TRUE)
            if (inherits(fm, "try-error") && isTRUE(file.exists("/usr/X11/bin/fc-match")))
                fm <- try(system("/usr/X11/bin/fc-match -f '%{file}' Arial 2> /dev/null", intern = TRUE), silent=TRUE)
            if (inherits(fm, "try-error") && isTRUE(file.exists("/usr/X11R6/bin/fc-match")))
                fm <- try(system("/usr/X11R6/bin/fc-match -f '%{file}' Arial 2> /dev/null", intern = TRUE), silent=TRUE)
            if (inherits(fm, "try-error") || !length(fm)) Rf_error("Unable to find core font on your system. Try setting ix.core.font.path option to the full path of the TTF file or make sure Arial is installed and fc-match is accessible in your system.")
        }
        .Call("A_SetCoreFontPath", fm, PACKAGE = pkgname)
    }
    
    .Call(A_Init)
}
